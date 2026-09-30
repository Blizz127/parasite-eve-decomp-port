/*
 * Native control-flow test for the M0000I transition outer loop
 * (func_8019234C, original 8019234C..80192740).
 *
 * Every direct callee is link-wrapped so the loop's own ordering, the
 * retained empty-run index / effect handle, the exit byte semantics and the
 * double-buffer flip can be asserted without the translated callees'
 * internal state.  HostFB_VSync is wrapped because func_80073A44 is an
 * inline over it.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game_port.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char trace[256];
static unsigned trace_len;
static unsigned vsync_mode_calls;
static int frame_count;
static uint32_t rand_value;
static uint32_t rand_calls;
static int stop_after_frame;      /* request stop inside 801942FC */
static int exit_frame;            /* 80193478 sets 0x8019C024 on this frame */

static void note(char id, long arg)
{
    if (trace_len + 24u < sizeof(trace))
        trace_len += (unsigned)snprintf(trace + trace_len,
                                       sizeof(trace) - trace_len,
                                       "%c%ld;", id, arg);
}

static void reset_fixture(void)
{
    PE_RamReset();
    PE_Port_RunControlReset();
    trace[0] = '\0';
    trace_len = 0;
    vsync_mode_calls = 0;
    frame_count = 0;
    rand_value = 0;
    rand_calls = 0;
    stop_after_frame = 0;
    exit_frame = 0;
}

/* ── wrapped callees ─────────────────────────────────────────────────── */

void __wrap_func_80196498(void) { note('C', 0); }
void __wrap_func_80191DE8(int a0) { note('D', a0); }
uint32_t __wrap_HostFB_VSync(int mode)
{
    if (mode < 0) { note('V', -1); return 0x12345678u; }
    note('V', mode);
    vsync_mode_calls++;
    return (uint32_t)(mode == 1 ? 0x99u : 0u);
}
void __wrap_HostFB_SetDispMask(int mask) { note('M', mask); }
void __wrap_func_80071A64(uint32_t seed) { note('S', (long)seed); }
unsigned int __wrap_func_80071A54(void)
{
    unsigned int value = rand_value;
    rand_calls++;
    if (rand_calls == 1u) value = 111u;
    note('q', (long)value);
    return value;
}void __wrap_func_8003EB04(void) { note('I', 0); }
void __wrap_func_8006A25C(void) { note('N', 0); }
void __wrap_func_801942FC(void)
{
    note('U', 0);
    if (stop_after_frame) PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
}
void __wrap_func_8018F05C(void) { note('F', 0); }
void __wrap_func_8018F92C(pe_addr_t a) { note('B', (long)a); }
int __wrap_func_80194108(int level) { note('G', level); return level + 1; }
void __wrap_func_80192740(void) { note('H', 0); }
void __wrap_func_80192800(void) { note('R', 0); }
void __wrap_func_80193478(void)
{
    frame_count++;
    note('K', frame_count);
    if (exit_frame && frame_count >= exit_frame)
        PE_StoreU16(0x8019C024u, 1u);
}
int32_t __wrap_func_80191E30(uint32_t id) { note('E', (long)id); return 0x55; }
void __wrap_func_80191EFC(uint32_t handle, pe_addr_t pos)
{
    note('e', (long)handle);
    note('p', (long)pos);
}
void __wrap_func_80037870(void) { note('T', 0); }
int __wrap_func_80074DC0(int mode) { note('m', mode); return 0; }
void __wrap_func_80193AB0(void) { note('A', 0); }
int __wrap_func_80074A44(int mode) { note('a', mode); return 0; }
pe_addr_t __wrap_func_80075424(pe_addr_t env) { note('d', (long)env); return env; }
pe_addr_t __wrap_func_800755F0(pe_addr_t env) { note('s', (long)env); return env; }
int __wrap_func_800753B4(pe_addr_t ot) { note('o', (long)ot); return 0; }
void __wrap_func_800752AC(pe_addr_t ot, int n) { note('Z', (long)ot); note('z', n); }
void __wrap_func_8019BF8C(pe_addr_t d) { note('L', (long)d); }
void __wrap_func_80192030(void) { note('X', 0); }
uint32_t __wrap_PE_TransitionCompactOT(uint32_t start) { note('O', (long)start); return 0; }

/* ── scenarios ───────────────────────────────────────────────────────── */

/* Case A: the whole held mask (0x0F000006) triggers the button exit. */
static int case_button_exit(void)
{
    const char *expected =
        "C0;D1;V-1;S305419896;q111;q222;I0;V0;M0;N0;U0;";
    reset_fixture();
    PE_StoreU16(0x8019C024u, 0u);
    PE_StoreU8(0x8019C00Eu, 0u);
    PE_StoreU32(0x8009D1A0u, 0x0F000006u);
    rand_value = 222u;
    stop_after_frame = 1;
    func_8019234C();
    if (strcmp(trace, expected) != 0) {
        fprintf(stderr, "button-exit trace\n got %s\n want %s\n", trace, expected);
        return 1;
    }
    if (PE_LoadU8(0x8019C00Eu) != 1u || PE_LoadU16(0x8019C024u) != 1u) {
        fprintf(stderr, "button-exit did not latch C00E/C024\n");
        return 1;
    }
    if (PE_LoadU32(0x8019C03Cu) != 222u) {
        fprintf(stderr, "button-exit RNG store %u\n", PE_LoadU32(0x8019C03Cu));
        return 1;
    }
    return 0;
}

/* Case B: one full frame.  80193478 ends the loop at the bottom check;
 * the C044/C045 single-shot enables the message update. */
static int case_full_frame(void)
{
    reset_fixture();
    PE_StoreU16(0x8019C024u, 0u);
    PE_StoreU8(0x8019C00Eu, 0u);
    PE_StoreU32(0x8009D1A0u, 0u);
    PE_StoreU16(0x8019C02Au, 0u);
    PE_StoreU8(0x8019C044u, 1u);
    PE_StoreU8(0x8019C045u, 0u);
    PE_StoreU32(0x8019C0C0u, 0u);
    PE_StoreU32(0x801EA578u, 0x800B0000u);
    PE_StoreU32(0x8019C9C0u, 0x80140000u);
    PE_StoreU32(0x8019C1FCu, 0x80160000u);
    PE_StoreU32(0x8019C274u, 0x80160000u);
    exit_frame = 1;
    rand_value = 7u;
    func_8019234C();
    if (PE_Port_ShouldStop()) { fprintf(stderr, "full frame stopped\n"); return 1; }
    const char *expected =
        "C0;D1;V-1;S305419896;q111;q7;"          /* construct + seed */
        "I0;"                                    /* 3EB04 */
        "U0;"                                    /* 942FC */
        "F0;B2149172016;H0;R0;K1;"               /* transforms + scene */
        "E2750;"                                 /* 91E30(0xABE) */
        "T0;"                                    /* 37870 */
        "O0;"                                    /* compact OT */
        "V1;m0;V2;A0;a1;d2148794376;s2148794468;"
        "o16380;Z2148925440;z4096;L2149171704;"  /* display + flip */
        "X0;";                                   /* 92030 exit selector */
    if (strcmp(trace, expected) != 0) {
        fprintf(stderr, "full-frame trace\n got %s\n want %s\n", trace, expected);
        return 1;
    }
    if (PE_LoadU32(0x8019CC14u) != 0x99u) {
        fprintf(stderr, "VSync(1) result not stored: %08X\n", PE_LoadU32(0x8019CC14u));
        return 1;
    }
    if (PE_LoadU32(0x8019C9C0u) != 0x8019C1F8u) {
        fprintf(stderr, "descriptor not flipped: %08X\n", PE_LoadU32(0x8019C9C0u));
        return 1;
    }
    if (PE_LoadU32(0x8009CDDCu) != 1u) {
        fprintf(stderr, "draw-bank index not toggled\n");
        return 1;
    }
    if (PE_LoadU8(0x8019C00Eu) != 0u) { fprintf(stderr, "exit byte set on normal frame\n"); return 1; }
    return 0;
}

/* Case C: two frames exercise the retained effect handle -> 91EFC path. */
static int case_two_frames_handle(void)
{
    reset_fixture();
    PE_StoreU16(0x8019C024u, 0u);
    PE_StoreU8(0x8019C00Eu, 0u);
    PE_StoreU32(0x8009D1A0u, 0u);
    PE_StoreU16(0x8019C02Au, 0u);
    PE_StoreU8(0x8019C044u, 0u);
    PE_StoreU8(0x8019C045u, 0u);
    PE_StoreU32(0x8019C0C0u, 0u);
    PE_StoreU32(0x801EA578u, 0x800B0000u);
    PE_StoreU32(0x8019C9C0u, 0x80140000u);
    PE_StoreU32(0x8019C1FCu, 0x80160000u);
    PE_StoreU32(0x8019C274u, 0x80160000u);
    exit_frame = 2;
    rand_value = 5u;
    func_8019234C();
    if (frame_count != 2) { fprintf(stderr, "expected 2 frames, got %d\n", frame_count); return 1; }
    if (!strstr(trace, "E2750;")) { fprintf(stderr, "first frame did not start effect\n"); return 1; }
    /* RNG period was seeded once per loop entry (0xC03C == 5). */
    if (PE_LoadU32(0x8019C03Cu) != 5u) {
        fprintf(stderr, "two-frame RNG store %u\n", PE_LoadU32(0x8019C03Cu));
        return 1;
    }
    /* handle 0x55 at position 0x800B001C (0x800B0000 + 0x1C). */
    if (!strstr(trace, "e85;p2148204572;")) {
        fprintf(stderr, "second frame did not update handle: %s\n", trace);
        return 1;
    }
    if (vsync_mode_calls < 4u) { fprintf(stderr, "too few VSync waits\n"); return 1; }
    return 0;
}

/* Case D: a nonzero entry-mode half skips the frame loop and only runs the
 * exit selector when the exit byte is clear. */
static int case_entry_mode_skip(void)
{
    reset_fixture();
    PE_StoreU16(0x8019C024u, 5u);
    PE_StoreU8(0x8019C00Eu, 0u);
    PE_StoreU32(0x8019C03Cu, 0xDEADBEEFu);
    rand_value = 33u;
    func_8019234C();
    const char *expected = "C0;D1;V-1;S305419896;q111;q33;X0;";
    if (strcmp(trace, expected) != 0) {
        fprintf(stderr, "entry-mode-skip trace\n got %s\n want %s\n", trace, expected);
        return 1;
    }
    if (PE_LoadU32(0x8019C03Cu) != 33u) {
        fprintf(stderr, "entry-mode-skip RNG store\n");
        return 1;
    }
    return 0;
}

/* Case E: mode 10 forces 120000, passes a0=0 to 91DE8 and skips the frame
 * loop entirely; the exit selector still runs because the exit byte is
 * clear. */
static int case_mode10_override(void)
{
    reset_fixture();
    PE_StoreU16(0x8019C024u, 10u);
    PE_StoreU8(0x8019C00Eu, 0u);
    PE_StoreU32(0x8019C03Cu, 0u);
    rand_value = 4u;
    func_8019234C();
    const char *expected = "C0;D0;V-1;S305419896;q111;q4;X0;";
    if (strcmp(trace, expected) != 0) {
        fprintf(stderr, "mode10 trace\n got %s\n want %s\n", trace, expected);
        return 1;
    }
    if (PE_LoadU32(0x8019C03Cu) != 120000u) {
        fprintf(stderr, "mode10 did not override period\n");
        return 1;
    }
    return 0;
}

int main(void)
{
    PE_RamInit();
    int failures = 0;
    failures += case_button_exit();
    failures += case_full_frame();
    failures += case_two_frames_handle();
    failures += case_entry_mode_skip();
    failures += case_mode10_override();
    if (failures) { fprintf(stderr, "%d outer-loop case(s) failed\n", failures); PE_RamDestroy(); return 1; }
    printf("PASS: 5 transition outer-loop control-flow cases\n");
    PE_RamDestroy();
    return 0;
}
