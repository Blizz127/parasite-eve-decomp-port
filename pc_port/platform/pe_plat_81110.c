/* libds-block transfer-step gate (guest 0x80081110), written clean-room from
 * black-box vectors of the original code (pc_port/tools/pe_cd_leaf_vectors.py
 * cdgen36) and psx-spx, with no SDK source and without reading the previous
 * provider.  The behaviour below is exactly what the 6000 vectors of this row
 * show; no claim is made about the original's source.
 *
 * State words (all 32 bits, guest RAM):
 *   0x8009B6AC  step size (words)          0x8009B6C4  start tick of the wait
 *   0x8009B6B0  running byte counter       0x8009B6D0  registered callback (0 = none)
 *   0x8009B6B4  remaining-steps / status   0x8009B6BC  callback response word
 *
 * Behaviour (no arguments; returns the value the vectors show in $v0):
 *   1. 0x8009B6B0 += 4 * word[0x8009B6AC]  (wrapping); 0x8009B6B4 -= 1 (wrapping).
 *      Both are stored before the first VSync call.
 *   2. v = VSync(-1).  A tick count `v` is "late" when, as signed 32-bit words,
 *      (word[0x8009B6C4] + 1200) < v (the sum wraps first; distance 1200 itself
 *      is not late, 1201 is; a plain signed difference compare is NOT the same
 *      near the wrap and the vectors separate them).
 *   3. When late: store -1 to 0x8009B6B4.
 *   4. Reload 0x8009B6B4.  When it is non-zero: v = VSync(-1) a second time
 *      (0x8009B6C4 reloaded, same test); when that second tick is not late the
 *      function returns that second VSync value with nothing else done.
 *   5. Otherwise (the counter is zero, or the second tick was late) call the
 *      libds status pump 0x80081268 (no arguments, its result is the default
 *      return value).  Then load the callback word 0x8009B6D0; when it is
 *      non-zero: reload 0x8009B6B4 and call the callback with
 *      (a0 = 5 when that word is negative as a signed 32-bit value, else 2;
 *       a1 = word[0x8009B6BC]); its result becomes the return value.
 *      Loads of 0x8009B6B4 / 0x8009B6BC / the callback word happen only on
 *      those paths (the vectors would fail a native that loads them early).
 * Vectors cannot show: any use of the third 0x8009B6B4 load other than the
 * a0 choice (it is consumed that way here); the pump argument registers; what
 * happens past the observed exits (the original has no repeat in the vectors).
 *
 * Host contracts (not behaviour of the original; the vectors never exercise
 * them): PE_Port_ShouldStop() is checked after every callee (VSync, pump,
 * callback) and leaves with the value in hand.  There is no repeat inside this
 * body (at most two VSync calls in any vector), hence no stuck-wait budget to
 * arm.  HOST CHOICE, not evidenced: one pe_plat_sio_poll_tick() on entry, by
 * analogy with the other hardware-polling natives of this block; nothing in
 * the vectors shows that guest callers poll this step, and the tick advances
 * the host device/clock on every call in production, which the vectors cannot
 * validate.  The integrator should confirm or drop it. */
#include <stdint.h>

#include "pe_guest_ram.h"
#include "pe_guestcode.h"
#include "host_framebuffer.h"
#include "game_port.h"
#include "pe_plat/cd.h"
#include "pe_sdk.h"

#define W_STEP      0x8009B6ACu
#define W_COUNT     0x8009B6B0u
#define W_REMAIN    0x8009B6B4u
#define W_RESPONSE  0x8009B6BCu
#define W_START     0x8009B6C4u
#define W_CALLBACK  0x8009B6D0u
#define PUMP        0x80081268u
#define LATE_TICKS  1200u

/* Compile-time mutants (checker self-test only; never defined in a build). */
#ifndef MUT_THR1
#define MUT_THR1 0
#endif
#ifndef MUT_THR2
#define MUT_THR2 0
#endif

/* which = 1 for the first test, 2 for the second; the MUT_* variants below can target one. */
static int late(uint32_t start, uint32_t now, int thr_adj, int which)
{
    (void)which;
#if defined(MUT_UNSIGNED1) || defined(MUT_UNSIGNED2) || defined(MUT_UNSIGNED)
    if (
#if defined(MUT_UNSIGNED)
        1
#elif defined(MUT_UNSIGNED1)
        which == 1
#else
        which == 2
#endif
        )
        return (uint32_t)(start + LATE_TICKS + (uint32_t)thr_adj) < now;
#endif
#if defined(MUT_SUBFORM1) || defined(MUT_SUBFORM2) || defined(MUT_SUBFORM)
    if (
#if defined(MUT_SUBFORM)
        1
#elif defined(MUT_SUBFORM1)
        which == 1
#else
        which == 2
#endif
        )
        return (int32_t)(now - start) > (int32_t)(LATE_TICKS + (uint32_t)thr_adj);
#endif
#if defined(MUT_LE1) || defined(MUT_LE2) || defined(MUT_LE)
    if (
#if defined(MUT_LE)
        1
#elif defined(MUT_LE1)
        which == 1
#else
        which == 2
#endif
        )
        return (int32_t)(start + LATE_TICKS + (uint32_t)thr_adj) <= (int32_t)now;
#endif
    return (int32_t)(start + LATE_TICKS + (uint32_t)thr_adj) < (int32_t)now;
}

int pe_plat_ds_81110_value(void)
{
    uint32_t now, remain, cb;
    int ret;
#if defined(MUT_NOOP)
    return 0;
#endif

    /* lead integration edit: the fresh body's entry pe_plat_sio_poll_tick() was dropped (a host choice not evidenced by the vectors; the port VSync entry already services the devices, matching the previously bound body and the suite). */

#if !defined(MUT_NO_STORE_COUNT)
#if defined(MUT_COUNT_NO_X4)
    PE_StoreU32(W_COUNT, PE_LoadU32(W_COUNT) + PE_LoadU32(W_STEP));
#else
    PE_StoreU32(W_COUNT, PE_LoadU32(W_COUNT) + PE_LoadU32(W_STEP) * 4u);
#endif
#else
    (void)PE_LoadU32(W_COUNT); (void)PE_LoadU32(W_STEP);
#endif
#if !defined(MUT_NO_STORE_DEC)
#if defined(MUT_DEC2)
    PE_StoreU32(W_REMAIN, PE_LoadU32(W_REMAIN) - 2u);
#else
    PE_StoreU32(W_REMAIN, PE_LoadU32(W_REMAIN) - 1u);
#endif
#else
    (void)PE_LoadU32(W_REMAIN);
#endif

    now = HostFB_VSync(-1);
    if (PE_Port_ShouldStop()) return (int)now;
    if (late(PE_LoadU32(W_START), now, MUT_THR1, 1)) {
#if !defined(MUT_NO_STORE_M1)
        PE_StoreU32(W_REMAIN, 0xFFFFFFFFu);
#endif
    }
    remain = PE_LoadU32(W_REMAIN);
#if defined(MUT_REMAIN_POS)
    if ((int32_t)remain > 0) {
#elif defined(MUT_REMAIN_BIT0)
    if (remain & 1u) {
#else
    if (remain != 0u) {
#endif
        now = HostFB_VSync(-1);
        if (PE_Port_ShouldStop()) return (int)now;
        if (!late(PE_LoadU32(W_START), now, MUT_THR2, 2)) return (int)now;
    }

    ret = PE_GuestCall("func_80081110_pump", PUMP, 0u, 0u, 0u, 0u, 0u);
#if defined(MUT_PUMP_RET0)
    ret = 0;
#endif
    if (PE_Port_ShouldStop()) return ret;
#if defined(MUT_CB_ALWAYS)
    cb = PE_LoadU32(W_CALLBACK);
    if (1) {
#else
    cb = PE_LoadU32(W_CALLBACK);
    if (cb != 0u) {
#endif
        uint32_t a0, a1;
#if defined(MUT_NO_RELOAD_B4)
        remain = 0u;
#else
        remain = PE_LoadU32(W_REMAIN);
#endif
#if defined(MUT_A0_FIVE)
        a0 = 5u;
#elif defined(MUT_A0_TWO)
        a0 = 2u;
#elif defined(MUT_A0_LE0)
        a0 = ((int32_t)remain <= 0) ? 5u : 2u;
#elif defined(MUT_A0_GT)
        a0 = (remain > 0x80000000u) ? 5u : 2u;
#elif defined(MUT_A0_EQM1)
        a0 = (remain == 0xFFFFFFFFu) ? 5u : 2u;
#elif defined(MUT_A0_SWAP)
        a0 = ((int32_t)remain < 0) ? 2u : 5u;
#else
        a0 = ((int32_t)remain < 0) ? 5u : 2u;
#endif
#if defined(MUT_A1_ZERO)
        a1 = 0u;
#elif defined(MUT_A1_ADDR)
        a1 = W_RESPONSE;
#else
        a1 = PE_LoadU32(W_RESPONSE);
#endif
#if defined(MUT_CB_SKIP)
        (void)a0; (void)a1; (void)cb;
#elif defined(MUT_CB_DISPATCH)
        /* the address-keyed dispatcher returns a handled flag, not the callback's $v0 */
        (void)a0;
        ret = PE_Cd_DispatchDataCallback(cb, a0, a1);
#else
        ret = PE_GuestCall("D_8009B6D0 (card poll callback)", cb, 2u, a0, a1, 0u, 0u);
#endif
    }
#if defined(MUT_RET_ZERO)
    return 0;
#else
    return ret;
#endif
}

void pe_plat_ds_81110(void)
{
    (void)pe_plat_ds_81110_value();
}
