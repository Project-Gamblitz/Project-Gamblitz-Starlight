// =============================================================================
// InkColorExtension.cpp — appends 14 Splatoon 1 ink-color combos to the
// engine's natural team-color cycle (Path B / engine-loader integration).
// =============================================================================
//
// HOW THE ENGINE PICKS COLORS
// ---------------------------
// Each match-scene start, `Cmn::TeamColorMgr::getNextVersus` reads from
// `mgr->entries[kindIdx].mParamLookup[]` (an array of `TeamColorParam`
// objects), advances a per-kindIdx counter, and writes the chosen combo's
// four `Color4f` values to its output buffer.
//
//   kindIdx 1 = Vss_Regular  (Turf War — vanilla 8 entries)
//   kindIdx 2 = Vss_Gachi    (Ranked   — vanilla 7 entries)
//
// `Cmn::TeamColorMgr::load` (5.5.2 `sub_710014221C`) at startup
// pre-allocates **32 slots per kindIdx** but only fills `mLengthOfLookup`
// of them from the static name table. Slots `[mLengthOfLookup..32)` are
// constructed but empty — perfect targets for new colors with no
// destructive overwrite of vanilla data.
//
// PATH B (this file)
// ------------------
// 1. Hook the BL that calls `load` (5.5.2 `0x10D238`).
// 2. After original runs, for each new entry:
//      a. Compute the next free slot (offset = 0x1C0 bytes per
//         TeamColorParam in 5.5.2).
//      b. Write the asset name (e.g. `"Vss_Regular_BlueLime"`) into the
//         slot's `mPath` buffer at slot+0x170 (verified from
//         `LDR X21, [X20,#0x170]` in load's inner loop).
//      c. Build a `sead::FixedSafeString<256>` scratch buffer on the
//         stack (matches the type the engine constructs for the same
//         calls — vtable + char ptr + size + inline 256-char buffer).
//      d. Call `slot.vtable[27]` (tryLoad/resolve) — returns true if
//         the `TeamColor/TeamColor_<name>.bprm` asset exists.
//      e. If yes, call `slot.vtable[30]` (load) — populates the slot's
//         four `Param<Color4f>` objects from the .bprm file.
//      f. Increment `entries[kindIdx].mLengthOfLookup`.
//
// Result: cycle naturally extends from 8/7 to 16/13 entries with
// authentic Splatoon 1 RGB values — same code path the engine uses for
// vanilla entries, no networking impact, no overwrite of user-pickable
// Test_Vss_Fest_Private themes.
//
// FALLBACK BEHAVIOR
// -----------------
// If `tryLoad` returns false for any name (asset missing in this build),
// the slot stays empty and `mLengthOfLookup` is NOT incremented — the
// engine simply doesn't include that entry in the cycle. Safe failure.
// =============================================================================

#include "Cmn/Actor.h"
#include "Cmn/Def/Mode.h"
#include "Cmn/TeamColorMgr.h"
#include "sead/string.h"
#include "flexlion/ProcessMemory.hpp"
#include "flexlion/InkColorExtension.hpp"

namespace {

struct ExtraColorEntry {
    const char* name;
    int         kindIdx;
};

// 14 Splatoon 1 leftover combos still shipped as `.bprm` assets in 5.5.2
// romfs — Nintendo just stopped registering them in the engine's static
// name table (sub_71001464B4). We append them via Path B to put them back
// into the cycle.
//
// Order matters for cycle-position predictability:
//   Turf War cycle: vanilla 0..7 → ext 8..15 → loop
//   Ranked  cycle: vanilla 0..6 → ext 7..12 → loop
// To add another combo:
//   1) Make sure `TeamColor/TeamColor_<Name>.bprm` exists in romfs.
//      Either it ships with the game (search the romfs filelist for
//      "TeamColor_*.bprm") or you provide it via romfs replacement.
//   2) Append a `{"<Name>", <kindIdx>}` line to this array.
//        kindIdx 1 = Vss_Regular (Turf War)
//        kindIdx 2 = Vss_Gachi   (Ranked)
//   3) Rebuild. The engine pre-allocates 32 TeamColorParam slots per
//      kindIdx (we use 8 for Vss_Regular and 7 for Vss_Gachi out of the
//      box), so we have ~24 / ~25 slack slots available before hitting
//      the engine's cap. No other code changes needed.
static const ExtraColorEntry kExtras[] = {
    // 7 new Vss_Regular (Turf War)
    {"Vss_Regular_BlueLime",            1},
    {"Vss_Regular_GreenPurple",         1},
    {"Vss_Regular_LightBlueDarkBlue",   1},
    {"Vss_Regular_LightBlueYellow",     1},
    {"Vss_Regular_OrangeBlue_Default",  1},
    {"Vss_Regular_PinkOrange",          1},
    {"Vss_Regular_TurquoiseOrange",     1},
    // 6 new Vss_Gachi (Ranked)
    {"Vss_Gachi_DarkblueYellow",        2},
    {"Vss_Gachi_GreenOrange",           2},
    {"Vss_Gachi_LightgreenBlue",        2},
    {"Vss_Gachi_LumigreenPurple",       2},
    {"Vss_Gachi_SodaPink",              2},
    {"Vss_Gachi_YellowLilac_Default",   2},
};

// 5.5.2 layout constants (verified against `sub_710014221C` disasm at
// 0x710014285C and 0x7100142868):
//   MOV W10, #0x1C0     →  TeamColorParam stride
//   LDR X21, [X20,#0x170] → slot.mPath.mCharPtr offset
constexpr size_t kTCParamStride         = 0x1C0;  // 448 bytes
constexpr size_t kSlotMPathCharPtrOff   = 0x170;  // slot.mPath.mCharPtr
constexpr size_t kSlotMPathBufferOff    = 0x180;  // mPathData inline buffer
constexpr size_t kEntriesBaseOffset     = 0x10;   // mgr->entries[0]
constexpr size_t kEntryStride           = 0x18;   // 24 bytes per entry
constexpr int    kMaxPreallocatedSlots  = 32;

// Vtable indices read from the engine's load loop:
//   ((vtable_at[v19[0] + 216])(v19, &v70, 1) & 1) != 0
//   (vtable_at[v19[0] + 240])(v19, &v70)
// 216/8 = 27, 240/8 = 30
constexpr size_t kVtableTryLoadOff  = 216;  // bytes
constexpr size_t kVtableLoadOff     = 240;

typedef bool (*TryLoadFn)(void* slot, sead::BufferedSafeStringBase<char>* scratch, int flag);
typedef void (*LoadFn)   (void* slot, sead::BufferedSafeStringBase<char>* scratch);

void appendOneColor(Cmn::TeamColorMgr* mgr, const ExtraColorEntry& e) {
    // entries[kindIdx] = mgr + 0x10 + 24*kindIdx
    char* entry = (char*)mgr + kEntriesBaseOffset + kEntryStride * e.kindIdx;

    // entries[N] layout:
    //   +0x00: void*  mParamLookup
    //   +0x08: int    mStart
    //   +0x0C: int    mBeginningOfLookup
    //   +0x10: int    mLengthOfLookup
    char* lookup = *(char**)(entry + 0x00);
    int   start  = *(int*) (entry + 0x08);
    int   begin  = *(int*) (entry + 0x0C);
    int   length = *(int*) (entry + 0x10);

    if (lookup == nullptr) return;
    if (length >= kMaxPreallocatedSlots) return;  // out of slack slots

    // Compute slot pointer using the same arithmetic the engine does
    // (see `7100142854 MOV W10, #0x1C0; SMADDL X20, W9, W10, X8`):
    //   adjusted = mBeginningOfLookup + length
    //   if (adjusted < mStart) mStart = 0
    //   slot = mParamLookup + 0x1C0 * (adjusted - mStart)
    int   adjusted = begin + length;
    int   subStart = (adjusted < start) ? 0 : start;
    char* slot     = lookup + kTCParamStride * (adjusted - subStart);

    // Write the asset name into the slot's mPath buffer. mPath is a
    // pre-constructed `sead::BufferedSafeStringBase<char>` whose
    // mCharPtr (at slot+0x170) already points to the inline mPathData
    // buffer (slot+0x180, 64 bytes capacity).
    char* mPathBuf = *(char**)(slot + kSlotMPathCharPtrOff);
    if (mPathBuf == nullptr) {
        // Fallback: write directly into the inline buffer slot.
        mPathBuf = slot + kSlotMPathBufferOff;
    }
    {
        size_t i = 0;
        while (i < 63 && e.name[i] != '\0') { mPathBuf[i] = e.name[i]; ++i; }
        mPathBuf[i] = '\0';
    }

    // Set up scratch buffer (sead::FixedSafeString<256>'s ctor wires up
    // vtable + char buffer ptr + size automatically — matches what the
    // engine constructs on its stack for the same purpose).
    sead::FixedSafeString<256> scratch;

    void** slotVtable = *(void***)slot;
    if (slotVtable == nullptr) return;

    // slotVtable is the vtable base; method N is at slotVtable[N], i.e.
    // byte offset N*8 — equivalently kVtableTryLoadOff/8 = 27.
    TryLoadFn tryLoad = (TryLoadFn)slotVtable[kVtableTryLoadOff / sizeof(void*)];
    LoadFn    load    = (LoadFn)   slotVtable[kVtableLoadOff    / sizeof(void*)];

    if (tryLoad == nullptr || load == nullptr) return;

    bool ok = tryLoad(slot, &scratch, 1);
    if (!ok) return;

    load(slot, &scratch);

    // Bump mLengthOfLookup so getNextVersus's mod arithmetic includes
    // this slot in the cycle.
    *(int*)(entry + 0x10) = length + 1;
}

}  // namespace

// The engine's merged ctor+load in 5.5.2 (sub_710014221C) — exposed via the
// C++ mangled symbol `_ZN3Cmn12TeamColorMgrC2Ev`. We declare it here as
// extern "C" with a single `this`-pointer arg so we can call it directly
// from the hook WITHOUT relying on runtime function-pointer init: this BL
// site fires during early static init (before init_starlion runs), so any
// runtime-initialized pointer would be NULL at first invocation.
extern "C" void _ZN3Cmn12TeamColorMgrC2Ev(void* this_);

// Hook installed at 5.5.2 0x10D238 (BL site that calls the TeamColorMgr
// ctor+load from GfxSetting-equivalent setup). After original runs, every
// kindIdx's mParamLookup has its vanilla TeamColorParam objects loaded;
// we then fill the slack slots with our 14 extras via the engine's own
// loader.
__int64 teamColorMgrLoadHook(__int64 mgr) {
    // Always run the original ctor+load — it builds entries[], allocates
    // the 32 pre-allocated TeamColorParam slots per kindIdx, and loads
    // the vanilla 8/7 entries from the static name table. Skipping this
    // would leave the manager uninitialized and crash any later access.
    _ZN3Cmn12TeamColorMgrC2Ev((void*)mgr);

    // Vtable indices verified against xr45 (3.1) `_ZTVN3Cmn14TeamColorParamE`:
    //   vtable[27] (byte offset 216) = Lp::Sys::Params::makePathName(buf, withExt)
    //   vtable[30] (byte offset 240) = Lp::Sys::Params::load(path) → loadCore_(this, path, 0)
    // makePathName fills `buf` with the asset path (e.g.
    // "TeamColor/TeamColor_Vss_Regular_BlueLime.b<ext>") using this slot's
    // `pathName` (vtable+200 idx 25) and `extName` (vtable+208 idx 26).
    // Returns 1 on success, 0 if pathName produced an empty string. Then
    // load(path) reads the .bprm and populates the slot's four
    // Param<Color4f> values.
    Cmn::TeamColorMgr* tcm = (Cmn::TeamColorMgr*)mgr;
    for (size_t i = 0; i < sizeof(kExtras) / sizeof(kExtras[0]); ++i) {
        appendOneColor(tcm, kExtras[i]);
    }

    return 0;
}

extern "C" void inkColorExtensionInit() {
    // No-op now; we call orig via the linker symbol directly.
}
