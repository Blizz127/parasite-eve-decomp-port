/* libds text_170 (guest 0x8007E704), the command-queue flush, written
 * clean-room as a native from black-box vectors of the original code
 * (pc_port/tools/pe_cd_leaf_vectors.py cdgen39, checked by
 * pc_port/tools/pe_spu_caller_check.c with -DCHK_HAVE_DS_GEN39).  Independent
 * re-implementation: the behaviour below is exactly the calls, loads and
 * stores seen in the vectors (hashes in
 * docs/evidence/native-cd-rows-20261125/REPORT.md); no SDK source and nothing
 * from any other implementation was used, and no claim is made about the
 * original's source.  The original code was only run as a black box.
 *
 * Behaviour (void; `status` is the low byte of the first argument, which is
 * what both the push calls and the callbacks receive, e.g. a0 = 0x8D353779 ->
 * 0x79):
 *  1. count = word 0x800A3608, then idx = word 0x800A3600 (in this order).
 *  2. while i < count (signed; the first test uses the value loaded in step 1,
 *     every later one reloads 0x800A3608 after the pass): the entry is at
 *     0x800A3540 + 0x18 * idx (32-bit wrap); load its word 0 (sequence) and its
 *     word +0x10 (callback); when the sequence differs from the previous
 *     pass's (initially 0; full word) call 0x8007EB88(sequence, status byte,
 *     response) -- so a run of equal sequences is pushed once, and a zero
 *     entry after a nonzero one is pushed as 0.  Independently, remember the
 *     entry's callback when it is nonzero and its sequence differs from the
 *     sequence of the last entry whose callback was remembered (none before the
 *     first: a first entry with sequence 0 or 0xFFFFFFFF still counts).  Entries
 *     without a callback do not move that reference.  (Vectors: equal-sequence
 *     entries share the first one's callback, even when entries without a
 *     callback sit between them.)
 *     Then idx = idx + 1 and, when that is >= 8 (signed), idx = 0.  A head of 8
 *     or more reads its first entry unwrapped, one past the ring (entry 8
 *     overlaps the head word).
 *  3. clear the queue block: 75 zero stores (0x800A3608, 0x800A3604,
 *     0x800A3600, then for each of the 8 entries from 0x800A3540: word +0 (4
 *     bytes), then single bytes at +4, +8, +7, +6, +5, then words at +0xC,
 *     +0x10, +0x14).
 *  4. for each remembered callback in pass order, when nonzero (full word)
 *     dispatch it with (status byte, response).  Callbacks go through the
 *     address-keyed dispatcher PE_Cd_DispatchDataCallback.
 * Undetermined: the order of the two entry loads against the push call, and
 * the original's storage for the remembered callbacks (vectors have at most
 * 16 passes).
 *
 * Host contract (not behaviour of the original): PE_Port_ShouldStop() is
 * checked before each callback dispatch; once the dispatcher has requested a
 * stop (an unresolved guest callback target is a recorded boundary) no further
 * callback is dispatched (the retail-recovery test expects exactly one
 * boundary).  The remembered callbacks
 * live in a host array of 256 words; passes beyond that still run their
 * push calls but their callbacks are not dispatched.  A count above 256 (the
 * ring has eight entries) is therefore bounded only by the guest's own word,
 * as in the original. */

/* (second function of this file) libds text_3D0 (guest 0x8007E964), the command-complete / error handler,
 * written clean-room as a native from black-box vectors of the original code
 * (pc_port/tools/pe_cd_leaf_vectors.py cdgen39, checked by
 * pc_port/tools/pe_spu_caller_check.c with -DCHK_HAVE_DS_GEN39).  Independent
 * re-implementation: the behaviour below is exactly the calls, loads and stores
 * seen in the vectors (hashes in docs/evidence/native-cd-rows-20261125/
 * REPORT.md); no SDK source and nothing from any other implementation was used,
 * and no claim is made about the original's source.  The original code was only
 * run as a black box.
 *
 * Notation: E(i) = 0x800A3540 + 0x18 * i (32-bit wrap) is a queue entry; its
 * words are +0 sequence, +4 (byte) and +0xC (word) command arguments, +0x10 a
 * callback word, +0x14 a retry word.  head = word 0x800A3604, tail = word
 * 0x800A3600, count = word 0x800A3608.  "next(h)" is h + 1, or 0 when that is
 * >= 8 (signed).  status is the low byte of the first argument (vectors: 0x102
 * behaves as 2).  void.
 *
 *  1. head = 0x800A3604; seq = E(head)+0.
 *  2. if seq != 0: store seq (word) to 0x800A3510 and status (byte) to
 *     0x800A3514; 0x80080998(0x800A3515, response); then
 *     - status == 2: reload head; load E(next(head))+0 and E(head)+0 again; when
 *       they are equal the entry is chained: store head + 1 to 0x800A3604 and,
 *       when that is >= 8 (signed), store 0 there too (two stores).  Otherwise
 *       load cb = E(head)+0x10; 0x8007EB88(seq, status, response); when cb is
 *       nonzero dispatch cb(status, response); 0x8007E5C4().
 *     - status == 5: retry = E(head)+0x14; when retry > 0 (signed) or retry ==
 *       0xFFFFFFFF: load tail, reload retry, store tail to 0x800A3604 and, when
 *       the reloaded retry > 0 (signed), store retry - 1 to E(head)+0x14 (so
 *       0xFFFFFFFF retries forever); otherwise the same push / callback / pop as
 *       the chain-less status 2 arm (seq and cb loaded again first).
 *     any other status does nothing more.
 *  3. gcb = word 0x800B8AB0; when nonzero dispatch gcb(status, response).
 *  4. when 0x8007FBF0(0) == 1: count = word 0x800A3608; when count > 0 (signed)
 *     and a second 0x8007FBF0(0) == 1: reload head, load E(head)+0 and, when
 *     nonzero, 0x8007FB44(byte E(head)+4, word E(head)+0xC) (result unused).
 * Callbacks go through the address-keyed dispatcher PE_Cd_DispatchDataCallback.
 * Undetermined: the first-vs-reloaded head used for E(head) addresses (no
 * vector separates them: nothing between the loads changes the word), and the
 * order of the two loads of the chain compare.
 *
 * Host contract (not behaviour of the original): PE_Port_ShouldStop() is
 * checked before each callback dispatch and after it; once the dispatcher has
 * requested a stop (an unresolved guest callback target is a recorded boundary)
 * the callback is skipped and the handler ends at once (no pop, no tail), as the
 * retail-oracle queue-completion test expects. */

#include <stdint.h>

#include "pe_guest_ram.h"
#include "pe_plat/cd.h"
#include "game_port.h"
#include "pe_sdk.h"

#define FLUSH_HEAD   0x800A3600u
#define FLUSH_MID    0x800A3604u
#define WORD_TAIL    0x800A3600u
#define WORD_HEAD    0x800A3604u
#define WORD_COUNT   0x800A3608u
#define WORD_GCB     0x800B8AB0u
#define REC_SEQ      0x800A3510u
#define REC_STATUS   0x800A3514u
#define REC_COPY     0x800A3515u
#define ENTRY_BASE   0x800A3540u
#define ENTRY_BYTES  0x18u
#define RING         8
#define SAVED_MAX    256u

void pe_plat_ds_text_170(uint32_t status, uint32_t response)
{
    uint32_t saved[SAVED_MAX];
    int32_t count;
    uint32_t idx, pass = 0u, nsaved = 0u, i, e, previous = 0u, callback_sequence = 0u;
    int have_callback = 0;

    count = (int32_t)PE_LoadU32(WORD_COUNT);
    idx = PE_LoadU32(FLUSH_HEAD);
    while ((int32_t)pass < count) {
        uint32_t entry = ENTRY_BASE + ENTRY_BYTES * idx;
        uint32_t sequence = PE_LoadU32(entry);
        uint32_t callback = PE_LoadU32(entry + 0x10u);
        if (sequence != previous) func_8007EB88(sequence, status & 0xFFu, response);
        if (callback && (!have_callback || sequence != callback_sequence)) {
            if (nsaved < SAVED_MAX) saved[nsaved++] = callback;
            callback_sequence = sequence;
            have_callback = 1;
        }
        previous = sequence;
        idx = idx + 1u;
        if ((int32_t)idx >= RING) idx = 0u;
        pass++;
        count = (int32_t)PE_LoadU32(WORD_COUNT);
    }
    PE_StoreU32(WORD_COUNT, 0u);
    PE_StoreU32(FLUSH_MID, 0u);
    PE_StoreU32(FLUSH_HEAD, 0u);
    for (e = 0u; e < (uint32_t)RING; e++) {
        uint32_t base = ENTRY_BASE + ENTRY_BYTES * e;
        PE_StoreU32(base, 0u);
        PE_StoreU8(base + 4u, 0u);
        PE_StoreU8(base + 8u, 0u);
        PE_StoreU8(base + 7u, 0u);
        PE_StoreU8(base + 6u, 0u);
        PE_StoreU8(base + 5u, 0u);
        PE_StoreU32(base + 0xCu, 0u);
        PE_StoreU32(base + 0x10u, 0u);
        PE_StoreU32(base + 0x14u, 0u);
    }
    for (i = 0u; i < nsaved; i++) {
        if (PE_Port_ShouldStop()) break;
        (void)PE_Cd_DispatchDataCallback(saved[i], status & 0xFFu, response);
    }
}

static uint32_t entry_at(uint32_t index) { return ENTRY_BASE + ENTRY_BYTES * index; }
static uint32_t next_index(uint32_t index)
{
    uint32_t n = index + 1u;
    return (int32_t)n >= RING ? 0u : n;
}
/* Returns nonzero when a stop is (now) requested: the handler then ends at once. */
static int dispatch(uint32_t callback, uint32_t status, uint32_t response)
{
    if (callback && !PE_Port_ShouldStop())
        (void)PE_Cd_DispatchDataCallback(callback, status, response);
    return PE_Port_ShouldStop();
}

void pe_plat_ds_text_3d0(uint32_t status_word, uint32_t response)
{
    uint32_t status = status_word & 0xFFu;
    uint32_t head = PE_LoadU32(WORD_HEAD);
    uint32_t seq = PE_LoadU32(entry_at(head));

    if (seq) {
        int finish = 0;
        PE_StoreU32(REC_SEQ, seq);
        PE_StoreU8(REC_STATUS, (uint8_t)status);
        func_80080998(REC_COPY, response);
        if (status == 2u) {
            uint32_t again = PE_LoadU32(WORD_HEAD);
            uint32_t following = PE_LoadU32(entry_at(next_index(again)));
            seq = PE_LoadU32(entry_at(head));
            if (following == seq) {
                PE_StoreU32(WORD_HEAD, again + 1u);
                if ((int32_t)(again + 1u) >= RING) PE_StoreU32(WORD_HEAD, 0u);
            } else {
                finish = 1;
            }
        } else if (status == 5u) {
            uint32_t retry = PE_LoadU32(entry_at(head) + 0x14u);
            if ((int32_t)retry > 0 || retry == 0xFFFFFFFFu) {
                uint32_t tail = PE_LoadU32(WORD_TAIL);
                uint32_t again = PE_LoadU32(entry_at(head) + 0x14u);
                PE_StoreU32(WORD_HEAD, tail);
                if ((int32_t)again > 0) PE_StoreU32(entry_at(head) + 0x14u, again - 1u);
            } else {
                seq = PE_LoadU32(entry_at(head));
                finish = 1;
            }
        }
        if (finish) {
            uint32_t callback = PE_LoadU32(entry_at(head) + 0x10u);
            func_8007EB88(seq, status, response);
            if (dispatch(callback, status, response)) return;
            func_8007E5C4();
        }
    }
    if (dispatch(PE_LoadU32(WORD_GCB), status, response)) return;
    if (func_8007FBF0(0) == 1) {
        if ((int32_t)PE_LoadU32(WORD_COUNT) > 0 && func_8007FBF0(0) == 1) {
            uint32_t h = PE_LoadU32(WORD_HEAD);
            uint32_t sequence = PE_LoadU32(entry_at(h));
            if (sequence) (void)func_8007FB44(PE_LoadU8(entry_at(h) + 4u), PE_LoadU32(entry_at(h) + 0xCu));
        }
    }
}
