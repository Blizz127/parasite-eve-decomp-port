/* Two CD-streaming library callbacks (guest 0x80080F98 and 0x8008214C),
 * written clean-room from black-box vectors of the original code
 * (pc_port/tools/pe_cd_leaf_vectors.py cdgen11, checked by
 * pc_port/tools/pe_spu_caller_check.c), with no SDK source.
 * Behaviour is exactly the guest loads, stores and callee calls seen in the
 * vectors; each function may end by calling through a function-pointer word
 * in guest RAM, which the port does with PE_GuestCall. */
#include "pe_sdk.h"
#include "pe_guest_ram.h"
#include "pe_guestcode.h"

#include "psx_compat.h"   /* VSync (guest 0x80073A44): the port's func_80073A44 -> HostFB_VSync */

/* ------------------------------------------------------------------ */
/* 0x80080F98: transfer progress step                                  */
/* ------------------------------------------------------------------ */

/* State words (32-bit). */
#define ADDR_PROG_MODE      0x8009B6B8u  /* bit 0 selects the "fetch once" kind */
#define ADDR_PROG_COUNT     0x8009B6B4u  /* signed remaining-step counter */
#define ADDR_PROG_CURSOR    0x8009B6B0u  /* guest address of the next buffer word */
#define ADDR_PROG_WORDS     0x8009B6ACu  /* words fetched per step */
#define ADDR_PROG_DEADLINE  0x8009B6C4u  /* frame count the timeout is measured from */
#define ADDR_PROG_LAST_V    0x8009B6C8u  /* last VSync(-1) result (written) */
#define ADDR_PROG_LAST_ARG  0x8009B6BCu  /* second argument of the last "fetch once" step (written) */
#define ADDR_PROG_CALLBACK  0x8009B6D0u  /* function-pointer word, 0 when unset */

#define PROG_TIMEOUT_FRAMES 1200         /* frames past the deadline before a timeout */
#define PROG_EVENT_DONE     2            /* callback code: counter ran out in a stepped transfer */
#define PROG_EVENT_TIMEOUT  5            /* callback code: counter negative / timed out */

/* Guest callees declared in pe_sdk.h:
 *   func_80073A44(mode)            VSync; -1 returns the frame count
 *   func_80080B04(buffer, words)   fetch into buffer
 *   func_80080AE4(buffer, words)   fetch into buffer (stepped transfers)
 *   func_80081268()                finish / flush */

/* Finish the transfer, then report through the callback word when set.
 * The counter is re-read after the callback word: zero reports `done_code`,
 * any other value reports PROG_EVENT_TIMEOUT. */
static void prog_finish(int done_code, int a1)
{
    uint32_t cb;

    func_80081268();
    cb = PE_LoadU32(ADDR_PROG_CALLBACK);
    if (cb != 0) {
        int32_t count = (int32_t)PE_LoadU32(ADDR_PROG_COUNT);

        PE_GuestCall("@80080F98:f", cb, 2,
                     (uintptr_t)(uint32_t)(count == 0 ? done_code : PROG_EVENT_TIMEOUT),
                     (uintptr_t)(uint32_t)a1, 0, 0);
    }
}

void pe_plat_ds_read_progress(int a0, int a1)
{
    int32_t now, count, deadline;
    uint32_t mode;

    now = func_80073A44(-1);
    PE_StoreU32(ADDR_PROG_LAST_V, (uint32_t)now);
    mode = PE_LoadU32(ADDR_PROG_MODE);
    count = (int32_t)PE_LoadU32(ADDR_PROG_COUNT);

    if (mode & 1u) {
        /* Odd kind: one fetch while the counter is positive, otherwise it ends;
         * a zero counter reports the caller's own low byte, a negative one the
         * timeout code. */
        if (count > 0) {
            func_80080B04(PE_LoadU32(ADDR_PROG_CURSOR), (int32_t)PE_LoadU32(ADDR_PROG_WORDS));
            PE_StoreU32(ADDR_PROG_LAST_ARG, (uint32_t)a1);
            return;
        }
        prog_finish((unsigned char)a0, a1);
        return;
    }

    /* Even kind: while the counter is positive, fetch one step, advance the
     * cursor by the word count and decrement the counter. */
    if (count > 0) {
        func_80080AE4(PE_LoadU32(ADDR_PROG_CURSOR), (int32_t)PE_LoadU32(ADDR_PROG_WORDS));
        /* The fetch may touch these words, so they are read again. */
        PE_StoreU32(ADDR_PROG_CURSOR,
                    PE_LoadU32(ADDR_PROG_CURSOR) + PE_LoadU32(ADDR_PROG_WORDS) * 4u);
        PE_StoreU32(ADDR_PROG_COUNT, PE_LoadU32(ADDR_PROG_COUNT) - 1u);
    }

    deadline = (int32_t)PE_LoadU32(ADDR_PROG_DEADLINE);
    now = func_80073A44(-1);
    /* Signed comparison against the wrapped sum. */
    if (now > (int32_t)((uint32_t)deadline + (uint32_t)PROG_TIMEOUT_FRAMES)) {
        PE_StoreU32(ADDR_PROG_COUNT, (uint32_t)-1);
        (void)func_80073A44(-1);
        prog_finish(PROG_EVENT_DONE, a1);
    } else if ((int32_t)PE_LoadU32(ADDR_PROG_COUNT) == 0) {
        prog_finish(PROG_EVENT_DONE, a1);
    } else {
        (void)func_80073A44(-1);
    }
}

/* ------------------------------------------------------------------ */
/* 0x8008214C: pending-event dispatch                                  */
/* ------------------------------------------------------------------ */

#define ADDR_EV_ACTIVE      0x8009B708u  /* nonzero while an event source is active */
#define ADDR_EV_PENDING     0x8009B70Cu  /* nonzero when an event is pending (cleared here) */
#define ADDR_EV_HANDLER_A   0x8009B700u  /* value passed to func_800824C8 */
#define ADDR_EV_HANDLER_B   0x8009B704u  /* value passed to func_800824DC */
#define ADDR_EV_CALLBACK    0x8009B6F4u  /* function-pointer word, 0 when unset */

#define EV_KIND_POLL        2            /* a0 value that polls the queue instead of dispatching */

/* Guest callees declared in pe_sdk.h:
 *   func_8007F778()       queue length (0 when empty)
 *   func_80082204()       queue service
 *   func_800824C8(cb), func_800824DC(cb)   handler registration */
void pe_plat_ds_text_3d8(unsigned char a0, int a1)
{
    uint32_t cb;

    if (PE_LoadU32(ADDR_EV_ACTIVE) == 0)
        return;
    if (PE_LoadU32(ADDR_EV_PENDING) == 0)
        return;

    if (a0 == EV_KIND_POLL) {
        if (func_8007F778() == 0)
            func_80082204();
        return;
    }

    (void)func_800824C8((int)PE_LoadU32(ADDR_EV_HANDLER_A));
    (void)func_800824DC((int)PE_LoadU32(ADDR_EV_HANDLER_B));
    PE_StoreU32(ADDR_EV_PENDING, 0);
    cb = PE_LoadU32(ADDR_EV_CALLBACK);
    if (cb != 0)
        PE_GuestCall("@8008214C:f", cb, 3, (uintptr_t)a0, (uintptr_t)(uint32_t)a1, 0, 0);
}
