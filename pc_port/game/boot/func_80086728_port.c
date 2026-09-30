/*
 * Phase 6E-B38 — func_80086728: streaming-mode command-byte switch.
 *
 * Retail body: 18 words / 0x48 bytes,
 * 0x80086728..0x8008676C (exclusive end 0x80086770), file offset
 * 0x76F28 (PS-X EXE).  All 18 words exe-verified against
 * SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 *
 * Maps the caller's argument to a CD streaming command byte and
 * dispatches via func_8008CBA8 (the streaming command dispatcher,
 * already translated in pe_stream.c):
 *
 *   a0 == 1 → cmd = 0x81
 *   a0 == 2 → cmd = 0x82
 *   else    → cmd = 0x80
 *
 * Stores the command at D_800BCD80, calls func_8008CBA8(), returns void.
 *
 * Retail delay-slot semantics:
 *   beq $a0,1 case_1  →  sw $ra,0x10($sp) (always)
 *   beq $a0,2 common  →  addiu $v0,$zero,0x82 (always; sets cmd for case 2)
 *   j   common        →  addiu $v0,$zero,0x80 (always; sets cmd for default)
 *   jal func_8008CBA8 →  nop
 *   jr  $ra           →  nop
 *
 * Sole call site: func_80052790 @ 0x8005279C.
 * func_80052790 passes a0 = (original_a0 < 1) ? 1 : 0, so the
 * reachable commands from that caller are 0x81 (true) and 0x80 (false).
 * The a0 == 2 path (cmd 0x82) is unreachable from func_80052790 but
 * exists in the retail binary for potential other callers.
 *
 * Classification: 1 — translated retail logic over already-real provider.
 */
#include "psx_compat.h"
#include "pe_sdk.h"
#include "pe_port_compat.h"

#define GA_D_800BCD80 0x800BCD80u

/* func_80086728: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80086728_port.c (src/func_80086728.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

#define GA_D_800BCD84 0x800BCD84u
#define GA_D_800BCD90 0x800BCD90u

/*
 * func_80086464 is 13 words (0x80086464..0x80086498):
 * D_800BCD80=0x10, D_800BCD84=a0, jal 8CBA8. Cmd 0x10 is
 * 85084(*CD84); magic fail returns -1. Stream-complete is
 * not invented.
 */
int func_80086464(pe_addr_t a0)
{
    PE_StoreU32(GA_D_800BCD80, 0x10u);
    PE_StoreU32(GA_D_800BCD84, a0);
    return func_8008CBA8();
}

/* func_80086498: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80086498_port.c (src/func_80086498.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_800864F8: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800864F8_port.c (src/func_800864F8.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_80086770: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80086770_port.c (src/func_80086770.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_80086C1C: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80086C1C_port.c (src/func_80086C1C.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_80086C5C: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80086C5C_port.c (src/func_80086C5C.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_80086CA4: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80086CA4_port.c (src/func_80086CA4.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* 866A4..866F0: enqueue the battle-ready volume command. */
void func_800866A4(unsigned int a0, unsigned int a1)
{
    PE_StoreU32(GA_D_800BCD80, 0x21u);
    PE_StoreU32(GA_D_800BCD84, a0 & 0xFFFFu);
    PE_StoreU32(0x800BCD88u, a1 & 0xFFFFFFu);
    (void)func_8008CBA8();
}

/* 86608..866A4: validate the sound bank and enqueue the retail effect
 * command. 8CBA8 returns its allocated playback handle. */
int32_t func_80086608(pe_addr_t sound, uint32_t key, uint32_t pan, uint32_t volume)
{
    PE_M34StackInvalidate();
    int32_t result=func_80085084(sound);
    if (result) return result;
    PE_StoreU32(0x800BCD80u,0x24u);
    PE_StoreU32(0x800BCD84u,sound+4u);
    PE_StoreU32(0x800BCD88u,key&0xFFFFFFu);
    PE_StoreU32(0x800BCD8Cu,pan&0xFFu);
    PE_StoreU32(0x800BCD90u,volume&0x7Fu);
    return func_8008CBA8();
}

/* Original 868F0..86948 / 86A28..86A80: update an existing effect's
 * volume/pan. The different handle masks are intentional retail behavior. */
/* func_800868F0: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800868F0_port.c (src/func_800868F0.c); hand port retired (port3 switch-over A1). */

/* func_80086A28: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80086A28_port.c (src/func_80086A28.c); hand port retired (port3 switch-over A1). */
