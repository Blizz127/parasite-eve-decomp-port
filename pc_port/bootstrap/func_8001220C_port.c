/*
 * Phase 6C — Native adaptation of func_8001220C (main).
 *
 * PE_PORT from PARKED candidate (semantically complete, 187 words).
 * MIPS scratchpad stack atom replaced with safe host adapter.
 * All register pins, empty asm barriers, and scheduling contortions removed.
 *
 * This is the REAL Parasite Eve main control flow, not a host shortcut.
 */

#include "psx_compat.h"
#include "game_port.h"
#include "stub_registry.h"
#include "pe_guestcode.h"
#include "pe_sdk.h"
#include <stdio.h>

/* ── Declarations ──────────────────────────────────────────────────── */

extern void func_800725DC(void);
extern void func_8003E610(void);
extern void func_8003E680(void);
extern void func_8006A5BC(void);
extern void func_8006A64C(void);
extern void func_8006A9E4(void);
extern int func_8006AD40(void);
extern int func_8006ECEC(void);
extern void func_8006F044(void);
extern int  func_8006E834(void);
extern void func_80069B08(int);
extern void func_8003F3C4(void);
extern void func_8019234C(void);
extern int  func_801909B4(void);
extern int  func_8006E9A0(int);
extern int  func_800698D4(void);
extern uint32_t func_80073A44(int mode);
extern void func_80074D28(int mask);

/* D_800B0CD8 is a guest-RAM lvalue macro (psx_compat.h) */

/* D_800A7918: guest-memory macro from psx_compat.h */

/* ── Scratchpad host adapter ───────────────────────────────────────── */

/*
 * Retail atom (ROM 0x2B20..0x2B44, 10 words):
 *   lui $a1, 0x1F80; ori $a1, 0x3FC   → address 0x1F8003FC (scratchpad top)
 *   sw $sp, 0($t0)                     → save host SP to scratchpad
 *   addiu $t0, -4; addu $sp, $t0, $zero → SP = scratchpad - 4
 *   jal func_8019234C; nop             → call overlay on PS1 stack
 *   addiu $sp, 4; lw $sp, 0($sp)       → restore host SP
 *
 * PE_PORT adapter: trace, call natively, no stack manipulation.
 */
static void PE_Port_ScratchpadCall(void (*overlay)(void))
{
    Trace_Direct("scratchpad_atom_begin");
    Stub_Record("scratchpad_stack_handoff", "HOST_ADAPTED");
    if (overlay) overlay();
    Trace_Direct("scratchpad_atom_end");
}

/* func_801235DC is the credits main loop in ovl_0457.  func_8006F044 has
 * just reloaded PE.IMG [0x39F,0x3C5) (ovl_039F, libpress) to 0x8010BCF8 and
 * [0x457,0x4FC) (ovl_0457) to 0x80120D00.  The registry binds the address by
 * the fingerprint of the bytes now there (generated ovl_0457__func_801235DC);
 * anything else is a loud boundary, never a silent return.
 *
 * The loop drives the drive itself.  With D_8009D1A0 bit 16 clear it opens
 * \XASTREAM\CREDITS.XA;1 (Disc 2) and queues CdlSetmode 0xC8 and
 * CdlSetfilter through func_8007EE84, waiting on func_8007F418 for each.  It
 * then plays the stream by CdlReadS and CdlGetlocP (func_801258EC,
 * func_80125A40).  Production's host-mode libds does not serve those commands,
 * so the entry attaches the register-level device around the loop
 * (PE_Cd_AttachDevice) and detaches it afterwards.
 *
 * With bit 16 set (the nine-step pad code in func_8003EB04 sets 0x12000)
 * there is no XA setup.  An edge on D_8009D1F4 bit 26 or 27 runs
 * func_80125894, which waits for func_8007F418(0, 0) == 2 and then sends
 * CdlSetfilter through func_80080DC4, so that mode needs the attached drive
 * too.  The 2 it waits for is the CdlComplete of the CdlPause that ends
 * every read (func_80081268), which the host read now publishes
 * (pe_libcd.c func_8006E6D4).
 *
 * Two cases stop on a named boundary instead of entering:
 *   - credits_xa_file_absent: bit 16 is clear and the active disc lacks the
 *     overlay's own file name (D_80125B48).  func_8012562C would retry
 *     CdSearchFile forever.
 *   - credits_cd_attach: the device cannot attach (no disc, libds busy, or
 *     already attached).
 *
 * The generated ovl_0457 loops never test PE_Port_ShouldStop.  The entry arms
 * the stop escape (game_port.h) around the loop, so a stop inside it (a
 * boundary, or a host quit) returns here instead of spinning.  The attach
 * record is static, so it stays valid across the longjmp. */
static PeCdAttach g_credits_cd;

static int credits_boundary(const char *name)
{
    Bootstrap_ReturnVoid1(name, "func_8001220C", D_8009D1A0 & 0x30000u);
    PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
    return 0;
}

int PE_Main_EnterCredits(void)
{
    jmp_buf escape;
    if (!PE_Cd_XaServiceAvailable()) return credits_boundary("credits_cd_xa_service");
    if (!(D_8009D1A0 & 0x10000u) && !PE_Cd_FilePresent(PE_LoadU32(0x80125B48u)))
        return credits_boundary("credits_xa_file_absent");
    if (!PE_Cd_AttachDevice(&g_credits_cd)) return credits_boundary("credits_cd_attach");
    if (setjmp(escape) == 0) {
        PE_Port_EscapeArm(&escape);
        PE_Port_CreditsPadArm(1);
        (void)PE_GuestCall("func_8001220C", 0x801235DCu, 0u, 0u, 0u, 0u, 0u);
    }
    PE_Port_CreditsPadArm(0);
    PE_Port_EscapeDisarm();
    PE_Cd_DetachDevice(&g_credits_cd);
    return !PE_Port_ShouldStop();
}

/* ── Main — adapted from PARKED candidate ──────────────────────────── */

/* Host adaptation: retail waits for a disc indefinitely (the
 * func_800698D4 wait loop only exits on a successful mount, return 0).
 * A headless host run without --disc-image/--bootstrap-disc would spin
 * forever, so the port bounds the wait and then abandons the boot with a
 * loud diagnostic.  With a disc (real or fixture) the loop behaves
 * exactly like retail and the bound is never reached. */
#define PE_PORT_DISC_WAIT_LIMIT 100000

void func_8001220C(void)
{
    unsigned int *data      = &D_800B0CD8;
    int            dispatch = 0;
    unsigned char *flagbyte = (unsigned char *)data + 0xF5;
    unsigned int   state_val = 0xA9400048u;
    unsigned int   bitmask;
    unsigned int   v;

    func_800725DC();
    func_8003E610();
    bitmask = 0x00100000u;

    for (;;) {
        int disc_wait = 0;
        if (!PE_Port_BeginMainIteration()) return;
        func_8006A5BC();

        /* Retail (asm/disc1/2A0C.s:33): loop while func_800698D4() != 0;
         * proceed when it returns 0 (mount succeeded). */
        while (func_800698D4() != 0) {
            func_80073A44(0);
            if (PE_Port_ShouldStop()) return;
            if (++disc_wait >= PE_PORT_DISC_WAIT_LIMIT) {
                fprintf(stderr,
                        "[PORT] no disc mounted after %d polls "
                        "(retail waits for disc insertion); aborting boot\n",
                        PE_PORT_DISC_WAIT_LIMIT);
                return;
            }
        }

        func_8006A64C();
        func_8003E680();
        func_8006A9E4();
        if (PE_Port_ShouldStop()) return;
        D_8009D280 = state_val;

        while (1) {
            PEPortDmaIrqCheckpointResult checkpoint_result;

            if (PE_Port_ShouldStop()) return;

            v = *data;
            if (v & bitmask) {
                func_80069B08(dispatch);
                *data &= 0xFFEFFFFFu;
            }

            func_8006AD40();
            PE_Field_PassContinues = 0;
            /* B53I-B2 deterministic hardware opportunity.  It deliberately
             * runs before the sticky host stop is honored: the admitted
             * retail prefix may have issued one DMA whose hardware/IRQ
             * cleanup cannot be suppressed by a frame-limit or older B50
             * stop.  The checkpoint captures and services at most one DMA
             * token and propagates the nested idle-pump boundary directly. */
            checkpoint_result = PE_Port_ServiceDmaIrqCheckpoint();
            if (checkpoint_result == PE_PORT_DMA_IRQ_CHECKPOINT_BOUNDARY) {
                return;
            }
            /* B53I-D deterministic later hardware opportunity.  The first
             * checkpoint must return normally before this distinct call can
             * capture the callback-issued second token.  Two explicit calls,
             * never a loop, prevent service of any third DMA here. */
            if (checkpoint_result == PE_PORT_DMA_IRQ_CHECKPOINT_RETURNED) {
                checkpoint_result = PE_Port_ServiceDmaIrqCheckpoint();
                if (checkpoint_result ==
                    PE_PORT_DMA_IRQ_CHECKPOINT_BOUNDARY) {
                    return;
                }
            }
            /* Honor any nested hardware/provider stop before consuming the
             * completed function's destination state.  B54K-M itself now
             * returns normally; this guard remains caller-wide policy. */
            if (PE_Port_ShouldStop()) return;
            v = D_8009D280;
            D_8009D1C4 = v;

            /* A8-code three-way dispatch */
            if (v == state_val) {
                func_8006E834();
                v = func_801909B4();
                /* A translated overlay prefix can expose an honest nested
                 * provider.  Do not consume its provisional return value. */
                if (PE_Port_ShouldStop()) return;
                func_8006E9A0(v);
                *data |= 0x3;
            } else if (state_val < v) {
                if (v == 0xAA108448u) {
                    if (*flagbyte & 0x2) {
                        func_8006F044();
                        if (PE_Port_ShouldStop()) return;
                        if (!PE_Main_EnterCredits()) return;
                        if (PE_Port_ShouldStop()) return;
                        D_8009D280 = 0xA80830C8u;
                        *data |= 0x1;
                    } else {
                        dispatch = 2;
                        *data |= bitmask;
                    }
                } else {
                    func_8003F3C4();
                }
            } else {
                if (v == 0xA8000048u) {
                    func_8006ECEC();
                    if (PE_Port_ShouldStop()) return;
                    PE_Port_ScratchpadCall(func_8019234C);
                    if (PE_Port_ShouldStop()) return;
                    *data |= 0x1;
                } else {
                    func_8003F3C4();
                }
            }

            /* 7-constant skip chain */
            v = D_8009D280;
            if (v == 0xA80651C8u) continue;
            if (v == 0xA8065248u) continue;
            if (v == 0xA80652C8u) continue;
            if (v == 0xA80660C8u) continue;
            if (v == 0xA8066148u) continue;
            if (v == 0xA80661C8u) continue;
            if (v == 0xA8066348u) continue;

            /* volume gate.  Retail reaches it only once 3F3C4 returns (field
             * exit at 3F68C); a host one-pass field tick that retail would
             * still be looping in does not raise the disc-swap request, so
             * a story bump past 0x258 inside a room swaps at the next room
             * transition, not over the live room overlay.  Same as retail. */
            v = D_800A7918;
            if (PE_Field_PassContinues) {
                /* retail is still inside the 3F3C4 inner loop */
            } else if (v < 0x258u) {
                if (!(*flagbyte & 0x1)) {
                    dispatch = 1;
                    *data |= bitmask;
                }
            } else {
                if (!(*flagbyte & 0x2)) {
                    dispatch = 2;
                    *data |= bitmask;
                }
            }

            /* 124A4 andi 0x100 / beq continue 12294.
             * Taken: VSync(0), SetDispMask(0), andi ~0x100
             * (addiu 0xFEFF), j 1224C outer 6A5BC restart.
             * Retail clears 0x100 only, not 0x001. */
            if (*data & 0x100) {
                func_80073A44(0);
                func_80074D28(0);
                *data &= ~0x100u;
                break;
            }
        }
    }
}
