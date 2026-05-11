// =============================================================================
// SaveDataLocalMapList.cpp — extend Shoal/Private-match map list 64 → 128.
// =============================================================================
//
// PROBLEM
// -------
// Splatoon 2 5.5.2's local play (Shoal) private-match map roster is stored in
// the savefile as a 256-byte block at SaveDataLocal.body[+0x00..+0xFF],
// holding 64 entries × 4 bytes. Each entry is a `u32` mapId; empty slots are
// `0xFFFFFFFF` (sentinel). Real mapIds always fit in 16 bits.
//
// Once the array is full, `setHavePrivateStage` silently drops further adds
// (return without writing). Saves with >64 entries (manually edited or via
// a future BCAT delivering many maps) corrupt adjacent fields and crash on
// downstream reads.
//
// SOLUTION (Option A — u16-pack)
// ------------------------------
// Repack the 256 bytes as 128 entries × 2 bytes. Sentinel becomes `0xFFFF`.
// The savefile body LAYOUT IS UNCHANGED — only the interpretation of the
// bytes within the existing 256-byte slice changes. Adjacent fields
// (rule list at body[+0x100], matched-device list at body[+0x1B0+], etc.)
// are unaffected, so existing saves remain decodable WITHOUT a
// schema-version migration of the surrounding struct.
//
// MIGRATION OF EXISTING SAVES (one-shot, in-place)
// ------------------------------------------------
// Per-u32 entry rule:
//   - `0xFFFFFFFF` (old empty)   → skip; final array fills with `0xFFFF`.
//   - `0x00000000` (real mapId 0)→ write a single `0x0000` to next u16 slot.
//   - `0x000000NN` (small mapId) → write `0x00NN` (the low halfword IS the
//                                  real mapId; the high halfword is u32
//                                  representation padding — discarded).
//   - high halfword non-zero non-0xFFFF → array is already in u16 format;
//     skip migration.
//
// Detection: scan all 64 old-u32 slots. If ANY high halfword is in
// (0x0001..0xFFFE), we're already u16 (because in u32 format, high halves
// are always exactly 0x0000 or 0xFFFF). Otherwise we're old u32 → migrate.
//
// READER PATCHES
// --------------
// The savefile array's only direct reader in 5.5.2 is `sub_7100410AF0`
// (the merged `Cui::StageSelectPage::makeStageList_` /
// `Match::UIVersusPrivateMainPage::makeStageList`). Its inner loop walks
// body[i] for i in 0..256 step 4, reads as u32, sentinel-tests bit 31. We
// patch three single-byte instruction edits (in patches/552.slpatch):
//   - 0x00410C44: LDR W21,[X8,X22]   → LDRH W21,[X8,X22]   (B8 → 78)
//   - 0x00410C48: TBNZ W21,#0x1F,…   → TBNZ W21,#0xF,…     (F8 → 78)
//   - 0x00410CAC: ADD X22,X22,#4     → ADD X22,X22,#2      (12 → 0A)
// After patching, the loop reads u16 entries with a u16-aware sentinel test
// and 2-byte stride — covering all 128 slots in the same 256 bytes.
//
// SCOPE NOT-TOUCHED (audited but unrelated)
// -----------------------------------------
//   - `Cmn::SaveDataLocal::reset` (5.5.2 sub_710030C410): unchanged. Its
//     `memset(body, 0xFF, 0x100)` produces 128 sentinels of 0xFFFF — exactly
//     what the new format wants.
//   - `Cmn::SaveDataLocal::isHavePrivateStage`: dead-code-eliminated in 5.5.2
//     (had zero callers in 3.1).
//   - BCAT/schedule readers: read from `ScheduleVersus.entries[]` with a
//     dynamic count, no 64-cap. Online doesn't touch this savefile array.
//   - `Match::UICoopLocalMainPage::makeStageList`: filters MushMapInfo by
//     `_Cop` suffix, no SaveDataLocal access.
//   - `updateLocalPrivateMatchPlayableSetting`: only writes via
//     setHavePrivateStage — covered by our hook.
// =============================================================================

// WeaponKind.h must be included before MushDataHolder.h (which transitively
// pulls MushWeaponInfo → WeaponData with a Cmn::Def::WeaponKind member;
// MushWeaponInfo.h's local include order is `WeaponData.h` then `WeaponKind.h`,
// so dependents must add WeaponKind.h up front).
#include "Cmn/Def/WeaponKind.h"
#include "Cmn/Mush/MushDataHolder.h"
#include "Cmn/Mush/MushMapInfo.h"
#include "flexlion/SaveDataLocalMapList.hpp"
#include "types.h"

namespace {

constexpr int kMapArrayBytes  = 256;       // savefile slot total bytes
constexpr int kOldSlotCount   = 64;        // u32 slots (legacy)
constexpr int kNewSlotCount   = 128;       // u16 slots (new)
constexpr u16 kU16Sentinel    = 0xFFFFu;
constexpr u32 kU32Sentinel    = 0xFFFFFFFFu;

// SaveDataLocal[+0x10] holds the body pointer (verified against 3.1
// ctor: `*((u64*)this + 2) = body + 227696` and the reset/setHave
// accessors at `*((char*)this + 16)`). Same offset in 5.5.2.
constexpr size_t kSaveDataLocalBodyOff = 0x10;

// -------------------------------------------------------------------------
// Detect whether the 256-byte block is still in the legacy u32 layout.
//
// In u32 layout: every slot's high halfword (bytes +2/+3 of the u32) is
// either 0x0000 (real mapId, u32 padding) or 0xFFFF (empty sentinel).
// A non-0/non-0xFFFF high halfword means the array has already been
// repacked into u16 (where adjacent slots can hold any small mapId).
// -------------------------------------------------------------------------
bool isOldU32Format(const u8* body)
{
    for (int i = 0; i < kOldSlotCount; ++i) {
        u32 val  = *(const u32*)(body + i * 4);
        u16 high = (u16)((val >> 16) & 0xFFFFu);
        if (high != 0x0000u && high != 0xFFFFu) {
            return false;  // already u16
        }
    }
    return true;
}

// -------------------------------------------------------------------------
// One-shot in-place migration. Uses a 256-byte stack buffer to avoid
// destructive partial overwrite of the same memory we're reading from.
// -------------------------------------------------------------------------
void migrateU32ToU16(u8* body)
{
    u16 newArr[kNewSlotCount];
    int cursor = 0;

    for (int i = 0; i < kOldSlotCount; ++i) {
        u32 val = *(const u32*)(body + i * 4);

        // Old empty slot → contributes nothing to the compacted u16 array.
        if (val == kU32Sentinel) {
            continue;
        }

        // Real mapId 0 case: u32 view is 0x00000000. The user-visible
        // mapId 0 IS valid (the engine has a map with that id), so we
        // explicitly preserve it as a single u16 0x0000. Without this
        // special case the per-half rule below would drop both halves.
        if (val == 0u) {
            if (cursor < kNewSlotCount) {
                newArr[cursor++] = 0x0000u;
            }
            continue;
        }

        // Regular path: old u32 stored a non-zero small mapId in the low
        // halfword; the high halfword is u32-representation padding
        // (always 0x0000 for real mapIds < 0x10000). Discard the padding,
        // keep the real value as a single u16.
        if (cursor < kNewSlotCount) {
            newArr[cursor++] = (u16)(val & 0xFFFFu);
        }
    }

    // Pad remaining slots with sentinel.
    while (cursor < kNewSlotCount) {
        newArr[cursor++] = kU16Sentinel;
    }

    // Commit.
    __builtin_memcpy(body, newArr, kMapArrayBytes);
}

inline void migrateIfNeeded(u8* body)
{
    if (isOldU32Format(body)) {
        migrateU32ToU16(body);
    }
}

}  // namespace

// -----------------------------------------------------------------------------
// saveDataLocalSetHavePrivateStageHook
//
// Replaces the engine's `Cmn::SaveDataLocal::setHavePrivateStage` at its
// entry point (patches/552.slpatch: `0030C4D0 B …`). Fully reimplements the
// add/remove logic in u16 format and runs the one-shot u32→u16 migration on
// first call after the mod is installed.
//
// Return value mirrors the original: returns the `MushMapInfo::Data*` for
// the looked-up mapId (non-null = valid mapId, success), or 0 otherwise.
// Callers (mainly `updateLocalPrivateMatchPlayableSetting`) treat it as a
// boolean.
// -----------------------------------------------------------------------------
__int64 saveDataLocalSetHavePrivateStageHook(
    void* this_, int mapId, char haveValue)
{
    if (this_ == nullptr) return 0;

    // Validate mapId — original returns 0/null for unknown ids before
    // touching the array. Preserves caller-visible behavior.
    Cmn::MushDataHolder* holder = Cmn::MushDataHolder::sInstance;
    if (holder == nullptr) return 0;
    Cmn::MushMapInfo* mushMapInfo = holder->mMushMapInfo;
    if (mushMapInfo == nullptr) return 0;

    Cmn::MushMapInfo::Data* found = mushMapInfo->search(mapId);
    if (found == nullptr) return 0;

    // body = SaveDataLocal[+0x10] (qword) → start of map array.
    u8* body = *(u8**)((u8*)this_ + kSaveDataLocalBodyOff);
    if (body == nullptr) return (__int64)found;

    // Idempotent migration. Only does work the first time after install.
    migrateIfNeeded(body);

    u16* arr    = (u16*)body;
    u16  target = (u16)mapId;

    if (haveValue & 1) {
        // ADD path. Idempotent: if mapId is already in the array, no-op.
        for (int i = 0; i < kNewSlotCount; ++i) {
            if (arr[i] == target) {
                return (__int64)found;
            }
        }
        // Find first sentinel slot and write target.
        for (int i = 0; i < kNewSlotCount; ++i) {
            if (arr[i] == kU16Sentinel) {
                arr[i] = target;
                return (__int64)found;
            }
        }
        // Array is full. Silent failure (matches original behavior at the
        // 64-slot cap). With 128 slots this should be effectively
        // unreachable in normal play.
    }
    else {
        // REMOVE path. Find first matching slot, mark sentinel.
        for (int i = 0; i < kNewSlotCount; ++i) {
            if (arr[i] == target) {
                arr[i] = kU16Sentinel;
                return (__int64)found;
            }
        }
        // mapId not in array — original silently no-ops, so do we.
    }

    return (__int64)found;
}
