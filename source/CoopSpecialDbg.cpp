// =============================================================================
// CoopSpecialDbg.cpp — Salmon Run special-pool shuffle hook.
// =============================================================================
//
// THE PROBLEM
// -----------
// Salmon Run picks 4 specials per shift from `Coop_System.bprm`'s
// `mCommon.mSpecialWeaponNameBuffer` pool. The picker
// (Game::Coop::CommonRuleModerator::setupChangeWeapon_, 5.5.2:
// sub_710073110C at file offset 0x73110C) clamps the iteration to
// `min(output_cap=4, *(RuleConfig+0x178))`, which means it ALWAYS reads
// only `pool[0..3]` regardless of how many entries the bprm has.
//
// THE FIX
// -------
// Pre-shuffle the WHOLE pool buffer (RuleConfig + 0x160) so the first 4
// entries are 4 random distinct picks from the full N. The picker then
// runs unchanged and step 5's tail shuffle randomizes which player gets
// which special.
//
// POOL SIZE DETECTION
// -------------------
// First attempt assumed `*(u32*)(RuleConfig+0x178)` was the pool size,
// but on-hardware testing (2026-05-18) confirmed it returns 4 even with
// 14 bprm entries — so it's the hardcoded "specials per shift" cap, not
// the pool size. We now SCAN the pool buffer up to 32 entries, stopping
// at NULL / an obviously-invalid pointer. Each pool entry is a
// `Lp::Sys::Param<FixedSafeString<32>>*` whose first qword is a vtable
// in the binary's data range (~`0x7102_xxxx_xxxx`).
// =============================================================================

#include "flexlion/CoopSpecialDbg.hpp"
#include "flexlion/FsLogger.hpp"
#include "flexlion/ProcessMemory.hpp"
#include "types.h"

extern "C" void _ZN4Game4Coop19CommonRuleModerator18setupChangeWeapon_ERN4sead6RandomE(void* this_, void* rng);
extern "C" u32  _ZN4sead6Random6getU32Ev(void* rng);

// RuleConfig::sInstance storage lives at 5.5.2 file offset 0x2D5B3C8.
// Resolved via ProcessMemory::MainAddr at runtime — robust against
// any linker-symbol relocation quirks.
static constexpr u64 RULECONFIG_SINSTANCE_FILE_OFFSET = 0x2D5B3C8;

static bool sShufLogged = false;  // log once per session so we don't spam

// Original bprm-loaded pool order, captured once on first hook fire.
// Each subsequent shuffle restores this BEFORE applying Fisher-Yates so the
// output is purely a function of the rng state (= the SR shift seed), not
// of whatever permutation the last shuffle left the pool in. Without this,
// our in-place shuffle compounds across shifts and the picks cycle through
// a fixed σ-orbit instead of staying deterministic per shift.
static void* sOriginalPool[64] = {0};
static u32   sOriginalN        = 0;
static bool  sPoolSaved        = false;

static inline bool isLikelyVtablePtr(u64 vt) {
    // Vtables for bprm-loaded Lp::Sys::Param objects live in the MAIN
    // binary's .data.rel.ro. We don't know the host's load base
    // (different on hardware vs Ryujinx), so derive it dynamically from
    // MainAddr(0). Then accept any pointer inside the main module
    // (generous 256MB window).
    u64 mainBase = ProcessMemory::MainAddr(0);
    return mainBase != 0 && vt >= mainBase && vt < (mainBase + 0x10000000ULL);
}

static u32 scanPoolSize(void** pool) {
    if (!pool) return 0;
    const u32 MAX_SCAN = 32;
    u32 n = 0;
    for (u32 i = 0; i < MAX_SCAN; ++i) {
        void* entry = pool[i];
        if (!entry) break;
        if ((u64)entry < 0x100000ULL) break;        // not a real pointer
        u64 vt = *(u64*)entry;                      // entry's vtable ptr
        if (!isLikelyVtablePtr(vt)) break;
        ++n;
    }
    return n;
}

void coopSpecialShufflerHook(void* moderatorState, void* rng) {
    // Read RuleConfig::sInstance via runtime address (robust, doesn't
    // depend on linker symbol relocation).
    void** sInstanceSlot = (void**)ProcessMemory::MainAddr(RULECONFIG_SINSTANCE_FILE_OFFSET);
    void*  cfg           = sInstanceSlot ? *sInstanceSlot : nullptr;
    void** pool          = nullptr;
    u32    stored        = 0xDEADBEEF;
    u32    scanned       = 0xDEADBEEF;
    if (cfg) {
        pool    = *(void***)((u8*)cfg + 0x160);
        stored  = *(u32*)   ((u8*)cfg + 0x178);
        scanned = scanPoolSize(pool);
    }

#if 0
    // Diagnostic: log shift seed + pool state. Flip to `#if 1` to re-enable
    // when iterating on the picker logic.
    u32* st = (u32*)rng;
    FsLogger::LogFormatDefaultDirect(
        "[SR-SHUF] cfg=%p pool=%p stored=%u scanned=%u rng=[%08x %08x %08x %08x]\n",
        cfg, pool, stored, scanned,
        st ? st[0] : 0, st ? st[1] : 0, st ? st[2] : 0, st ? st[3] : 0);
    sShufLogged = true;
#endif

    if (cfg && rng && pool) {
        u32 n = (scanned > stored) ? scanned : stored;
        if (n > 64) n = 64;

        // First call: snapshot the original bprm-loaded order.
        if (!sPoolSaved && n > 0) {
            for (u32 i = 0; i < n; ++i) sOriginalPool[i] = pool[i];
            sOriginalN = n;
            sPoolSaved = true;
        }
        // Restore original order before this shuffle so each call starts
        // from the same baseline and the output is purely rng-determined.
        if (sPoolSaved) {
            u32 restoreN = (n < sOriginalN) ? n : sOriginalN;
            for (u32 i = 0; i < restoreN; ++i) pool[i] = sOriginalPool[i];
        }

        if (n > 4) {
            for (u32 i = n - 1; i > 0; --i) {
                u32 r = _ZN4sead6Random6getU32Ev(rng);
                u32 j = (u32)(((u64)r * (u64)(i + 1)) >> 32);
                void* tmp = pool[i];
                pool[i]   = pool[j];
                pool[j]   = tmp;
            }
        }
    }
    _ZN4Game4Coop19CommonRuleModerator18setupChangeWeapon_ERN4sead6RandomE(moderatorState, rng);

#if 0
    // Diagnostic: log the 4 specials vanilla picker just assigned.
    // Output list lives at moderatorState+0x78:
    //   +0x78 packed (mSize, mCap)  — 4 entries after vanilla picker
    //   +0x80 T** buffer            — array of node ptrs
    //   +0x88 Node* freelistHead
    // Each node's first u32 is the assigned special weapon ID.
    if (moderatorState) {
        u32   count        = *(u32  *)((u8*)moderatorState + 0x78);
        void** specialBuf  = *(void***)((u8*)moderatorState + 0x80);
        u32 ids[4] = {0, 0, 0, 0};
        if (specialBuf) {
            for (u32 i = 0; i < 4 && i < count; ++i) {
                void* node = specialBuf[i];
                if (node) ids[i] = *(u32*)node;
            }
        }
        FsLogger::LogFormatDefaultDirect(
            "[SR-SHUF] picked: count=%u SpP1=%u SpP2=%u SpP3=%u SpP4=%u\n",
            count, ids[0], ids[1], ids[2], ids[3]);
    }
#endif
}
