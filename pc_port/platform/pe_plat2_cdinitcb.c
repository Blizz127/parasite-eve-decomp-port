/* libcd "initialize callbacks" (retail entry 0x8007F994): the one-shot start-up step of the CD driver that runs
 * the device and volume initializers, clears three state words, runs the stream-state initializer and the guarded
 * state setter, installs two callback addresses, registers the status pump in VSync slot 0 and raises the two
 * "initialized" words.  Returns 1.
 *
 * PROVENANCE (clean-room, black-box, second author).  Re-derived by sw-cdrw-d from the retail-derived vector set
 * cdgen59 ONLY (400 cases; docs/evidence/native-cdx-20261126/cdgen59/REPORT.md and init.txt: the "S" stores in
 * order, "RETS" scripted callee returns, "CALL" recorded callee calls with their "@n" = how many of the case's
 * S stores precede the call), the checker pc_port/tools/pe_cdx_cdgen59_check.c on pe_cdx_common_canary.h (what a
 * case compares), the public headers pe_plat/cd.h, pe_plat/cdx_stream.h, pe_plat/cdx_etc.h, pe_callback.h,
 * game_port.h, and the callers' contracts in the suite (tests/test_cd_init_native.h, tests/test_cd_lowlevel_init.h)
 * and pe_spu_caller_check.c.  Written without opening the previous author's version of this file, its report, the
 * deleted older provider, src/, asm/, vendor/, game/decomp_hand/, game/boot bodies, pe_lib*.c, *_retail.c, any
 * oracle source, retail_*_cases.h or evidence quoting original code.  Nothing is claimed about how the original
 * was written; the behaviour below is exactly what the vectors show.
 *
 * Observed sequence (every one of the 400 cases; the return values of the first two callees are scripted to
 * every value and never change what follows):
 *   1. 0x8007BBFC()                          device initializer (a hardware wait: the host adapter stops here if
 *                                            the port is stopping)
 *   2. 0x8007BAC0()                          volume initializer (same stop rule)
 *   3. words 0x800A36A8, 0x800A36A4, 0x800A36A0 <- 0           (@3 at every later call until step 6)
 *   4. 0x8007FA2C()                          stream state initializer
 *   5. 0x800812F4(0)                         guarded state setter
 *   6. words 0x8009AFB4 <- 0x80080164, 0x8009AFB8 <- 0x80080778 (@5 at step 7)
 *   7. 0x80073D58(0, 0x8007FE24)             VSync callback registration, slot 0, the status pump
 *   8. words 0x8009AFD8 <- 1, 0x8009B554 <- 1                  (after the last call)
 *   9. returns 1
 * The checker compares only the number of stores before each call, so the order inside a group is not pinned;
 * the groups themselves are.  The host adapter additionally binds the guest pump address 0x8007FE24 to the host
 * pump just before step 7 (the checker counts that one host-only call per case).  If the bind is refused the
 * adapter raises the explicit unresolved-boundary stop and returns 0 with steps 7-9 not taken (the suite's
 * binding-conflict contract: steps 1-6 already happened, the existing binding is kept).  The _retail core is the
 * same sequence without the bind and without the stop checks. */
#include <stdint.h>

#include "pe_sdk.h"
#include "pe_callback.h"
#include "pe_plat/cd.h"
#include "pe_plat/cdx_etc.h"
#include "pe_plat/cdx_stream.h"
#include "game_port.h"

/* ---- guest words written (little-endian guest RAM) ---------------------- */
#define W_STATE_A8      0x800A36A8u
#define W_STATE_A4      0x800A36A4u
#define W_STATE_A0      0x800A36A0u
#define W_CALLBACK_1    0x8009AFB4u   /* receives guest address 0x80080164 */
#define W_CALLBACK_2    0x8009AFB8u   /* receives guest address 0x80080778 */
#define W_INIT_FLAG     0x8009AFD8u
#define W_HOST_BOOTED   0x8009B554u

#define CALLBACK_1      0x80080164u
#define CALLBACK_2      0x80080778u
#define VSYNC_SLOT      0u
#define PUMP_ADDRESS    0x8007FE24u   /* guest address of the per-VBlank status pump */

/* One body for both entry points; `host` selects the adapter behaviour (stop checks, host binding). */
static int cd_init_callbacks(int host)
{
    (void)func_8007BBFC();
    if (host && PE_Port_ShouldStop()) return 0;
    (void)func_8007BAC0();
    if (host && PE_Port_ShouldStop()) return 0;

    PE_StoreU32(W_STATE_A8, 0u);
    PE_StoreU32(W_STATE_A4, 0u);
    PE_StoreU32(W_STATE_A0, 0u);

    func_8007FA2C();
    func_800812F4(0u);

    PE_StoreU32(W_CALLBACK_1, CALLBACK_1);
    PE_StoreU32(W_CALLBACK_2, CALLBACK_2);

    if (host && PE_Callback_Bind(PUMP_ADDRESS, func_8007FE24) != 0) {
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
        return 0;
    }
    (void)func_80073D58(VSYNC_SLOT, PUMP_ADDRESS);

    PE_StoreU32(W_INIT_FLAG, 1u);
    PE_StoreU32(W_HOST_BOOTED, 1u);
    return 1;
}

int pe_plat_cd_initialize_callbacks_retail(void)
{
    return cd_init_callbacks(0);
}

int pe_plat_cd_initialize_callbacks(void)
{
    return cd_init_callbacks(1);
}
