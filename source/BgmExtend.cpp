#include "types.h"
#include "Cmn/StaticMem.h"
#include "sead/random.h"

#define BGM_SLOT_COUNT 29

// ---------------------------------------------------------------------------
// External game function that fills/shuffles a PtrArray with BGM indices.
// Located at 0x3778FC (552 offset: 0x3778FC - 0x2EBE000, added to syms552.ld).
//
// Args struct layout (passed as a single pointer, all fields at 8-byte stride):
//   [+0x00] void*          pThis   – VersusMatchmakeExe* (vtable[0xF8/8] = getBgmIdNum)
//   [+0x08] BgmPtrArray*   pArray  – extended array struct (see below)
//   [+0x10] s32*           pSeed   – pointer to the random seed word
// ---------------------------------------------------------------------------
struct BgmPtrArray; // forward-declared; full definition below

struct BgmArgs {
    void*        pThis;
    BgmPtrArray* pArray;
    s32*         pSeed;
};

extern "C" void bgmfuncthign(BgmArgs*);

// ---------------------------------------------------------------------------
// Extended PtrArray struct — mirrors the stack layout that choiceBgm builds,
// but allocated statically with BGM_SLOT_COUNT entries instead of 16.
//
// Memory layout (matches what bgmfuncthign accesses):
//   +0x00  mLength     (s32)  — current fill count
//   +0x04  mBufferSize (s32)  — max capacity
//   +0x08  mPtr        (u64*) — pointer to nodes[] array (= &nodes[0])
//            ^ bgmfuncthign writes node pointers here (mPtr[i] = &nodes[k])
//   +0x10  mFreeHead   (u64*) — head of the singly-linked free list
//   +0x18  mFreeHead2  (u64*) — secondary head set by choiceBgm (not read by bgmfuncthign
//                               itself, but we replicate the original behaviour)
//   +0x20  nodes[N]    (u64)  — N free-node cells.  While free: stores a
//                               u64 pointer to the next free node.
//                               While in-use: stores BGM index in lower 32 bits.
//   +0x20+N*8  ptrBuf[N] (u64*) — the actual PtrArray pointer buffer.
//                               mPtr points here; bgmfuncthign fills it with
//                               pointers to in-use nodes.
//
// Total size: 0x20 + 2*N*8  (for N=25: 0x20 + 400 = 432 bytes)
// ---------------------------------------------------------------------------
struct alignas(8) BgmPtrArray {
    s32   mLength;                    // +0x00
    s32   mBufferSize;                // +0x04
    u64*  mPtr;                       // +0x08  → ptrBuf below
    u64*  mFreeHead;                  // +0x10
    u64*  mFreeHead2;                 // +0x18
    u64   nodes[BGM_SLOT_COUNT];      // +0x20  node storage
    u64*  ptrBuf[BGM_SLOT_COUNT];     // +0x20+BGM_SLOT_COUNT*8  pointer buffer
};

// One static instance — safe because choiceBgm is never called concurrently.
static BgmPtrArray sBgmArray;

// ---------------------------------------------------------------------------
// Initialise the free list for a fresh call.
// Chains: nodes[0] → nodes[1] → … → nodes[N-1] → NULL
// ---------------------------------------------------------------------------
static void initBgmArray() {
    sBgmArray.mLength     = 0;
    sBgmArray.mBufferSize = BGM_SLOT_COUNT;
    sBgmArray.mPtr        = reinterpret_cast<u64*>(sBgmArray.ptrBuf); // point to ptrBuf

    for (int i = 0; i < BGM_SLOT_COUNT - 1; i++)
        sBgmArray.nodes[i] = reinterpret_cast<u64>(&sBgmArray.nodes[i + 1]);
    sBgmArray.nodes[BGM_SLOT_COUNT - 1] = 0ULL; // NULL terminator

    sBgmArray.mFreeHead  = &sBgmArray.nodes[0];
    sBgmArray.mFreeHead2 = &sBgmArray.nodes[0]; // mirrors choiceBgm's var_130 initialisation
}

// ---------------------------------------------------------------------------
// Helper: read the u32 BGM ID from a node pointer stored in ptrBuf.
// bgmfuncthign stores the BGM index in the lower 32 bits of the node cell.
// ---------------------------------------------------------------------------
static inline u32 getBgmIdAt(int idx) {
    return static_cast<u32>(*sBgmArray.ptrBuf[idx]);
}

// ---------------------------------------------------------------------------
// choiceBgmHook — full replacement for choiceBgm (0x376950 in 5.5.2)
//
// Called as:   void Cnet::VersusMatchmakeExe::choiceBgm(void)
// i.e. pThis is passed in X0 (first argument), no other arguments.
//
// Behaviour is identical to the original, except the PtrArray lives in static
// storage and is sized for BGM_SLOT_COUNT instead of 16.
// ---------------------------------------------------------------------------
void choiceBgmHook(void* pThis) {

    // -----------------------------------------------------------------------
    // 1. Retrieve or generate the random seed (mirrors original exactly).
    //    StaticMem layout (verified in IDA):
    //      +0x3D0 (u32) = current BGM ID    (mVersusMode/Rule area)
    //      +0x3D4 (u32) = persisted seed
    // -----------------------------------------------------------------------
    Cmn::StaticMem* sm = Cmn::StaticMem::sInstance;
    u8* smBytes = reinterpret_cast<u8*>(sm);

    s32 seed = *reinterpret_cast<s32*>(smBytes + 0x3D4);

    if (!seed) {
        u32 r = sead::GlobalRandom::sInstance->getU32();
        seed  = static_cast<s32>(r ? r : 1u);
    }

    // -----------------------------------------------------------------------
    // 2. Initialise our static PtrArray with a fresh BGM_SLOT_COUNT free list.
    // -----------------------------------------------------------------------
    initBgmArray();

    // -----------------------------------------------------------------------
    // 3. Call bgmfuncthign to fill + shuffle the array.
    //    It uses vtable[0xF8/8](pThis) = getBgmIdNum() to determine how many
    //    BGMs to insert.  getBgmIdNum() calls getVersusBgmNum() which (after
    //    the byte-patch) now returns BGM_SLOT_COUNT for normal versus mode.
    // -----------------------------------------------------------------------
    BgmArgs args { pThis, &sBgmArray, &seed };
    bgmfuncthign(&args);

    // -----------------------------------------------------------------------
    // 4. Find the current BGM in the shuffled list and compute nextIdx.
    //    Original logic (verbatim):
    //      - Search for curBgm in ptrBuf[0..count-1]
    //      - If found at position i, nextIdx = i+1
    //      - If not found (loop exhausted), nextIdx = 0+1 = 1 (LABEL_10 path)
    //    count = mLength after bgmfuncthign (may be < BGM_SLOT_COUNT if BgmSelectInfo
    //    filtered some BGMs out).
    // -----------------------------------------------------------------------
    sm      = Cmn::StaticMem::sInstance;
    smBytes = reinterpret_cast<u8*>(sm);

    s32 count  = sBgmArray.mLength;
    s64 nextIdx = 0; // LABEL_10 fallback (idx=0, so nextIdx = 0+1 = 1)

    if (count >= 1) {
        u32 curBgm = *reinterpret_cast<u32*>(smBytes + 0x3D0);
        for (s64 i = 0; i < count; i++) {
            if (getBgmIdAt(i) == curBgm) {
                nextIdx = i; // found at i → nextIdx = i+1 below
                break;
            }
        }
        nextIdx++; // advance to the next BGM after the current one
    }

    // -----------------------------------------------------------------------
    // 5. Wrap-around: if nextIdx >= count, reshuffle until ptrBuf[0] != curBgm.
    // -----------------------------------------------------------------------
    if (count < 1 || nextIdx >= static_cast<s64>(count)) {
        u32 curBgm = *reinterpret_cast<u32*>(smBytes + 0x3D0);

        for (int retry = 1; retry <= 999; retry++) {
            u32 r = sead::GlobalRandom::sInstance->getU32();
            seed  = static_cast<s32>(r ? r : 1u);

            // bgmfuncthign's Phase-1 reset re-chains the nodes from ptrBuf[]
            // back onto the free list automatically (mLength > 0 after prev fill),
            // so we do NOT need to call initBgmArray() again here.
            bgmfuncthign(&args);

            if (getBgmIdAt(0) != curBgm)
                break;
        }
        nextIdx = 0;

    } else {
        // Normal path: re-read sm like the original does.
        sm      = Cmn::StaticMem::sInstance;
        smBytes = reinterpret_cast<u8*>(sm);
    }

    // -----------------------------------------------------------------------
    // 6. Write the selected BGM ID and seed back to StaticMem.
    // -----------------------------------------------------------------------
    sm      = Cmn::StaticMem::sInstance;
    smBytes = reinterpret_cast<u8*>(sm);

    *reinterpret_cast<u32*>(smBytes + 0x3D0) = getBgmIdAt(nextIdx);
    *reinterpret_cast<u32*>(smBytes + 0x3D4) = static_cast<u32>(seed);
}
