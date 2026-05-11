// =============================================================================
// StageSelectPageExpand.cpp — extend the Shoal stage-select page's UI list
// from 64 to 128 entries, to match the savefile-cap extension (see
// SaveDataLocalMapList.cpp).
// =============================================================================
//
// PROBLEM
// -------
// `Cui::StageSelectPage` allocates a 64-slot inline buffer for its UI list
// nodes (page+0x930..0xB30) and stores capacity = 64 in [page+0x914]. The
// makeStageList helper `sub_7100410AF0` reads from this buffer in the
// offline savefile loop AND a follow-up "all MushMapInfo" iteration (the
// latter is enabled by an existing slpatch NOPing `B.NE` at 0x410CC4 —
// "Always show Shifty Stations on PBs").
//
// Each iteration that passes the filters does:
//   1. Pop a node from free list (page+0x920) — UNCONDITIONAL
//   2. Zero the popped node — UNCONDITIONAL (`STR WZR, [X8]`)
//   3. Check capacity; if full, skip the actual append
//   4. Else write node ptr to UI array, increment count
//
// With our savefile extension (allowing >64 entries), the offline loop pops
// up to 128 free-list nodes. With only 64 nodes available, the 65th pop
// returns NULL → step 2 null-derefs.
//
// (The crash log misleadingly reports PC at the BL target's pure-arithmetic
// helper `sub_710032B3C4` due to JIT trace-collapsing on Ryujinx; the
// actual fault is at `STR WZR, [X8]` inside `sub_7100410AF0`.)
//
// SOLUTION
// --------
// We wrap every BL site that calls `sub_7100410AF0` with a hook that:
//
//   1. Initializes our static 128-node free list + 128-slot UI array
//      (fresh chain on every call — the helper's reset loop already
//      assumes a stateless invocation, so each call starting from a clean
//      pool matches the engine's expected behavior).
//   2. Overrides the page's fields to use our buffers:
//        page[+0x910]: count     = 0
//        page[+0x914]: capacity  = 128
//        page[+0x918]: UI ptr    = our 128-slot UI array
//        page[+0x920]: free head = our first free-list node
//      The inline 64-node buffer at page+0x930..0xB30 is left orphaned;
//      the engine never references it again because both pointers have
//      been redirected.
//   3. Forwards the call to the vanilla `sub_7100410AF0` via a linker
//      symbol, so the helper's full logic runs against our buffers.
//
// WHY WRAP THE CALLERS (instead of the helper's entry)
// ----------------------------------------------------
// `sub_7100410AF0`'s first instruction is `SUB SP, SP, #0x80` — its
// stack-frame prologue. Replacing it with a B redirect requires a
// trampoline (re-emit the SUB SP in our hook, then jump to instruction 2).
// Wrapping the 3 BL callsites is structurally simpler and the same effect.
//
// CALLSITES (5.5.2)
// -----------------
// All 3 are `MOV X0, X19; BL sub_7100410AF0` patterns:
//   - 0x004108F4 inside sub_7100410494 (page setup method)
//   - 0x004111CC inside sub_7100411184
//   - 0x00411B88 inside sub_7100411B78
//
// SINGLE-PAGE ASSUMPTION
// ----------------------
// We use a single shared static buffer. The Splatoon 2 UI flow only ever
// has one Cui::StageSelectPage active at a time (the Shoal stage picker),
// so there's no concurrent-instance conflict.
// =============================================================================

#include "Cmn/StaticMem.h"
#include "flexlion/StageSelectPageExpand.hpp"
#include "types.h"

namespace {

constexpr int kCapacity = 128;

// Free-list nodes. Each "node" is 8 bytes:
//   - On the free list: bytes 0..7 hold the next-pointer.
//   - After pop+append: the engine's STR WZR zeros the link, then later
//     STR W21 writes the mapId (u32) at the same offset, overwriting the
//     stale next-pointer (which is fine since the node is no longer linked).
static u64 sFreeListNodes[kCapacity];

// UI array — 128 pointers to free-list nodes. Offset i corresponds to
// the i-th map currently shown in the UI. Set by the append code.
static u64 sUiArray[kCapacity];

// Re-initialize the free list as a fresh chain: node[0] → node[1] → ... →
// node[127] → NULL.
inline void initBuffers() {
    for (int i = 0; i < kCapacity - 1; ++i) {
        sFreeListNodes[i] = (u64)&sFreeListNodes[i + 1];
    }
    sFreeListNodes[kCapacity - 1] = 0;
    // sUiArray contents don't need explicit clearing — count starts at 0,
    // so the engine never reads UI[i] before writing it.
}

inline void overridePageFields(void* page) {
    u8* pb = (u8*)page;

    initBuffers();

    // count = 0
    *(u32*)(pb + 0x910) = 0;
    // capacity = 128 (replaces vanilla's hardcoded 64)
    *(u32*)(pb + 0x914) = kCapacity;
    // UI array pointer → our 128-slot array
    *(u64*)(pb + 0x918) = (u64)sUiArray;
    // Free list head → first node in our 128-node chain
    *(u64*)(pb + 0x920) = (u64)&sFreeListNodes[0];
}

}  // namespace

// The vanilla makeStageList helper at 5.5.2 `0x00410AF0`. We forward to
// it after overriding the page's pointers so it operates on our 128-slot
// buffers instead of the inline 64-slot ones.
extern "C" void* _makeStageListHelperOrig(void* page);

void* makeStageListHelperHook(void* page) {
    if (page != nullptr) {
        overridePageFields(page);
    }
    return _makeStageListHelperOrig(page);
}

// =============================================================================
// updateStageListForRandomFromOnOffList_ wrapper — caps page count to the
// vanilla rotation-list capacity (64) before the call.
// =============================================================================
// `sub_71004130D0` at 5.5.2 0x004130D0 processes the page's UI list, pushing
// each toggle-enabled entry into a STATIC global "random rotation" list at
// `*Cmn::StaticMem::sInstance + 0x7A0` (or `+0xBC0` if [page+0xE18] == 2).
// That global list's free pool has vanilla capacity 64 — when our extended
// page UI has 65+ enabled entries, this function pops past the global free
// list and null-derefs at the next `STR WZR, [X13]`.
//
// EXPLORATION HISTORY (and why we cap instead of extend)
// ------------------------------------------------------
// First attempt: cap page count to 32 before each call. Worked — no crash —
// but limited the random-rotation pool to 32 maps even though vanilla can
// hold 64.
//
// Second attempt: override the StaticMem-global rotation descriptor at both
// `singleton + 0x7A0` and `singleton + 0xBC0` to point at 128-slot external
// buffers, hoping vanilla would naturally fill 128 entries. Result: NEW
// crash deep in the page-setup chain (PC=0x19C7658 inside an L_Option_NN
// button-setup loop). The downstream consumers of the rotation list have
// their own hardcoded ≤64 expectations; expanding the list to 128 entries
// surfaces those latent caps. Chasing all of them would require fixing a
// long tail of unrelated consumers throughout the UI layer.
//
// Pragmatic compromise (this version): cap the page count visible to
// `sub_71004130D0` at exactly 64 — vanilla's actual rotation-list capacity.
// First 64 toggle-enabled maps land in the rotation pool; the rest still
// display in the page UI but aren't eligible for random rotation. After
// the function returns, we restore the real page count so any code after
// the call sees the full 128.
//
// CAP CHOICE: 64
// --------------
// User confirmed empirically that vanilla 5.5.2 supports 64 maps in the
// rotation list. Lower caps (we tried 32 initially) work but waste headroom.
//
// CALLSITES (5.5.2) — patched via 4 BL replacements in patches/552.slpatch:
//   - 0x00410AD0 inside sub_7100410494 (page setup, last call)
//   - 0x004112A4 inside sub_7100411184
//   - 0x00411C60 inside sub_7100411B78
//   - 0x0041256C inside sub_7100411EE0
// =============================================================================

namespace {

// Global rotation-list buffers — one set per state variant.
//
// "A" variant: state != 2 (regular private match). At singleton + 0x7A0.
// "B" variant: state == 2. At singleton + 0xBC0.
static u64 sGlobalRotFreeListA[kCapacity];
static u64 sGlobalRotUiArrayA[kCapacity];
static u64 sGlobalRotFreeListB[kCapacity];
static u64 sGlobalRotUiArrayB[kCapacity];

inline void initGlobalRotFreeList(u64* freeList) {
    for (int i = 0; i < kCapacity - 1; ++i) {
        freeList[i] = (u64)&freeList[i + 1];
    }
    freeList[kCapacity - 1] = 0;
}

inline void overrideGlobalRotDescriptor(u8* descriptor, u64* freeList, u64* uiArray) {
    initGlobalRotFreeList(freeList);

    // count = 0 (low 32) | capacity = 128 (high 32)
    *(u32*)(descriptor + 0x00) = 0;
    *(u32*)(descriptor + 0x04) = kCapacity;
    // UI array pointer
    *(u64*)(descriptor + 0x08) = (u64)uiArray;
    // Free-list head
    *(u64*)(descriptor + 0x10) = (u64)freeList;
}

inline void overrideGlobalRotationLists() {
    Cmn::StaticMem* singleton = Cmn::StaticMem::sInstance;
    if (singleton == nullptr) return;

    u8* singletonBytes = (u8*)singleton;

    // Override both state variants on every call — either path is then safe.
    overrideGlobalRotDescriptor(singletonBytes + 0x7A0,
                                sGlobalRotFreeListA, sGlobalRotUiArrayA);
    overrideGlobalRotDescriptor(singletonBytes + 0xBC0,
                                sGlobalRotFreeListB, sGlobalRotUiArrayB);
}

}  // namespace

extern "C" void* _updateStageListForRandomOrig(void* page);

void* updateStageListForRandomHook(void* page) {
    // Redirect the StaticMem-global random-rotation list's pointers to our
    // 128-slot external buffers BEFORE the vanilla function runs. Vanilla
    // then naturally operates on our extended buffer.
    //
    // Earlier we feared this caused the OptionsPage L_Option_04 crash, but
    // that turned out to be a missing layout file in the friend's setup.
    // With that file in place, going to 128 here should work cleanly.
    overrideGlobalRotationLists();

    return _updateStageListForRandomOrig(page);
}

