/*
 * Host prototypes for the hand adapters in pc_port/game/decomp_hand/ whose
 * matched leaf sits at VRAM >= 0x80060000 (port_stubs lane).
 *
 * Every function here is a hand adaptation of the verified matched leaf
 * `src/func_XXXXXXXX.c` (see docs/ai_context/PC_PORT_FROM_DECOMP.md): retail
 * pointer parameters/returns become 32-bit guest addresses (`pe_addr_t`),
 * everything else keeps the leaf's type.  gen_decomp_ports.py reads these
 * prototypes as the canonical host signatures, so generated callers bind to
 * the hand adapter instead of a loud boundary.
 *
 * Where a leaf forwards a caller's live argument register it never writes
 * (proved from the retail disassembly, tools/analysis/pe_dis.sh), the
 * register is spelled as an explicit trailing parameter and documented at
 * the definition.
 */
#ifndef PE_HAND_HI_PROTOS_H
#define PE_HAND_HI_PROTOS_H

#include <stdint.h>
#include "psx_compat.h"

/* Guest scratch for retail STACK buffers whose address a leaf passes to a
 * guest-address callee (a host stack address has no guest meaning).  Lives
 * in the top-of-RAM retail stack region, disjoint from the port_absent lane
 * (PE_HAND_LO_STACK_TEMP = 0x801FFD00) and the older 0x801FFExx scratch. */
#define PE_HAND_HI_STACK 0x801FFC00u   /* 0x80 bytes: 0x801FFC00..0x801FFC7F */

/* ── SPU voice / control registers (spu_voice_port.c) ──────────────── */
void func_80087728(unsigned int a0);
void func_80087798(int voice, int a1, int a2);
void func_800877BC(int a0, unsigned short a1);
void func_8008780C(int voice, int a1, unsigned int a2);
void func_8008783C(int a0, int a1);
void func_80087864(unsigned int a0, unsigned int a1);
void func_8008788C(int voice, unsigned int field, unsigned int mode);
void func_8008770C(unsigned int a0);
void func_80087744(unsigned int a0);
void func_80087760(unsigned int a0);
void func_8008777C(unsigned int a0);
void func_800877D4(int a0, unsigned int a1);
void func_800877F0(int a0, unsigned int a1);
void func_80089F08(unsigned int a0, pe_addr_t a1);

/* ── serial stream cursor readers (stream_cursor_port.c) ───────────── */
void func_8009059C(pe_addr_t arg0);
void func_800905C4(pe_addr_t arg0);
void func_80090AAC(int arg0, int arg1);

/* ── small state / copy helpers (misc_hi_port.c) ───────────────────── */
void func_8008D820(pe_addr_t arg0, pe_addr_t arg1, unsigned int arg2);
int func_800824DC(int value);
void func_800850C0(void);
pe_addr_t func_80080B44(int arg0, pe_addr_t arg1);
int func_8007FCBC(int a0, int a1);
void func_8008AF08(void);
void func_8008AFB8(pe_addr_t a0);
int func_80081D18(int a0, int a1, int a2);
void func_8008D7D0(void);

/* ── getters / setters / wrappers (small_hi_port.c) ───────────────── */
pe_addr_t func_8007A354(void);
pe_addr_t func_8007FC28(void);
void func_8007FBC0(pe_addr_t cb);
pe_addr_t func_8007A4A8(pe_addr_t callback);
pe_addr_t func_80081254(pe_addr_t callback);
int func_80081E5C(int value);
int func_800824B4(int value);
pe_addr_t func_800C2B10(int arg0);
pe_addr_t func_800C2B28(int arg0);
void func_80085728(pe_addr_t base);
pe_addr_t func_80071944(pe_addr_t arg0);
pe_addr_t func_800719C4(pe_addr_t arg0);
void func_8007A468(uint32_t a0, pe_addr_t a1);
int func_8007A8CC(int a0, unsigned int a1);
int func_8007A8AC(unsigned int a0, unsigned int a1);
void func_80091080(pe_addr_t a0, unsigned int a1);
void func_8007FBCC(pe_addr_t callback);
void func_8007FBD8(pe_addr_t callback);
void func_8007FBE4(pe_addr_t callback);
void func_8008C16C(pe_addr_t arg0);
void func_8008C270(pe_addr_t arg0);
void func_80074330(pe_addr_t arg0, int arg1);
void func_800744A4(pe_addr_t arg0, int arg1);
void func_8007474C(pe_addr_t arg0, int arg1);
void func_80077A28(pe_addr_t arg0, unsigned char arg1, int arg2);
pe_addr_t func_80078C94(pe_addr_t a0, pe_addr_t a1);
void func_80077A00(void);
void func_80082ADC(void);
int func_800C7D00(pe_addr_t base, int unused, int index, unsigned int value, int arg4, int arg5);
int func_800C8E44(pe_addr_t base, int unused, int index, unsigned int value, int arg4, int arg5);
int func_800C9B3C(pe_addr_t base, int unused, int index, unsigned int value, int arg4, int arg5);
int func_800CA6D4(pe_addr_t base, int unused, int index, unsigned int value, int arg4, int arg5);
int func_800CBEE0(pe_addr_t base, int unused, int index, unsigned int value, int arg4, int arg5);
int func_800CCEBC(pe_addr_t base, int unused, int index, unsigned int value, int arg4, int arg5);
int func_800CD89C(pe_addr_t base, int unused, int index, unsigned int value, int arg4, int arg5);
int func_800CE118(pe_addr_t base, int unused, int index, unsigned int value, int arg4, int arg5);
pe_addr_t func_80071964(pe_addr_t a0);
pe_addr_t func_80071994(pe_addr_t a0);

/* ── batch 4 (batch4_hi_port.c) ──────────────────────────────────── */
void func_8008B1D0(pe_addr_t arg0);
void func_8008227C(unsigned char a0);
void func_800878C0(int voice, unsigned int low, unsigned int mode);
pe_addr_t func_800755BC(pe_addr_t a0);
pe_addr_t func_80075AE8(pe_addr_t a0);
unsigned int func_8007A400(unsigned int a0);
unsigned int func_8007A434(unsigned int a0);
void func_8007C444(int a0, unsigned int a1);
int func_8008594C(int a0);
int func_80083790(pe_addr_t a0);
void func_8008B580(int arg0);
void func_800C811C(int a0, int a1, pe_addr_t a2);
void func_800CAB88(int a0, int a1, pe_addr_t a2);
void func_800CD07C(int a0, int a1, pe_addr_t a2);
void func_800CC244(int a0, int a1, pe_addr_t a2);
void func_800CC284(int a0, int a1, pe_addr_t a2);
void func_800CC440(int a0, int a1, pe_addr_t a2);
void func_80082400(unsigned char a0);
void func_8008B040(int a0);
void func_8008B084(pe_addr_t arg0);
void func_8008AB1C(pe_addr_t out0, pe_addr_t out1, unsigned int a2);
void func_8008B124(int arg0);
void func_8008B0C8(pe_addr_t a0);

/* ── batch 5 (batch5_hi_port.c) ──────────────────────────────────── */
void func_8008CB08(pe_addr_t a0);
void func_8008ABF0(void);
void func_8008AB9C(pe_addr_t a0);
pe_addr_t func_80080CDC(pe_addr_t dst);
void func_80083D9C(pe_addr_t a0);
int func_80085A04(int a0, pe_addr_t a1);
void func_800DFB20(pe_addr_t a0);
int func_800C6584(pe_addr_t a0, int a1, pe_addr_t a2, int a3);
pe_addr_t func_800629BC(int a0);
void func_8006EC84(int base, int count);
void func_80084644(pe_addr_t a0);
void func_8008B168(pe_addr_t a0);
void func_80082204(void);
void func_80089250(unsigned int a0);
void func_800CCFA0(void);
void func_800CD980(void);

/* ── batch 6 (batch6_hi_port.c) ──────────────────────────────────── */
void func_800C7DE4(pe_addr_t a0);
void func_800C8F28(pe_addr_t a0);
void func_800CA7B8(pe_addr_t a0);
void func_800686A0(pe_addr_t a0, int a1, int a2, int a3);
void func_80071034(pe_addr_t a0, pe_addr_t a1);
void func_8008A354(int a0, pe_addr_t a1);
void func_8008C374(void);
void func_8008C3E4(pe_addr_t a0);
void func_8008C55C(void);
void func_8008C5D8(void);
void func_8008C654(void);
pe_addr_t func_800742B8(void);
pe_addr_t func_8006E590(pe_addr_t a0, int a1);
void func_800C8BC0(int a0, pe_addr_t a1, pe_addr_t a2);
void func_800C9974(int a0, pe_addr_t a1, pe_addr_t a2);
void func_800CA4B4(int a0, pe_addr_t a1, pe_addr_t a2);
void func_800CBB30(int a0, pe_addr_t a1, pe_addr_t a2);
void func_800CD5EC(int a0, pe_addr_t a1, pe_addr_t a2);

/* ── batch 7 (batch7_hi_port.c) ──────────────────────────────────── */
unsigned int func_8008E840(int a0, unsigned int a1, int a2);
void func_800CD50C(int a0, int a1, pe_addr_t a2);
void func_800CECAC(void);
int func_8007F608(int a0);
void func_800CDF4C(int a0, pe_addr_t a1, pe_addr_t a2);
int func_800C66C8(pe_addr_t a0, pe_addr_t a1, pe_addr_t a2);
int func_800C6764(pe_addr_t a0, pe_addr_t a1, pe_addr_t a2);
int func_800C6800(pe_addr_t a0, pe_addr_t a1, pe_addr_t a2);
pe_addr_t func_80074218(void);
void func_800C4E50(pe_addr_t r);

/* ── batch 8 (batch8_hi_port.c) ──────────────────────────────────── */
void func_80061B80(int a0, int a1, pe_addr_t p);
void func_800C8064(int a0, int a1, pe_addr_t a2);
void func_800C91A8(int a0, int a1, pe_addr_t a2);
void func_800C7AE0(pe_addr_t a0, pe_addr_t a1, int a2, pe_addr_t a3);
int func_80083C3C(pe_addr_t a0);
int func_800C6C18(pe_addr_t a0);
void func_800CC7AC(int a0, int a1, pe_addr_t a2);
void func_800CC55C(int a0, int a1, pe_addr_t a2);

/* ── batch 9 (batch9_hi_port.c) ──────────────────────────────────── */
int func_800DFB78(pe_addr_t a0);
void func_8008B1FC(pe_addr_t a0);
void func_800CC480(int a0, pe_addr_t a1, pe_addr_t a2);
void func_800CD1FC(int a0, int a1, pe_addr_t a2);
pe_addr_t func_800625B8(int a0, pe_addr_t a1);
int func_8006BD68(void);
void func_8008C46C(void);

/* ── batch 10 (batch10_hi_port.c) ────────────────────────────────── */
void func_800C7E50(int a0, int a1, pe_addr_t a2);
void func_800C7F60(int a0, int a1, pe_addr_t a2);
void func_800C90A4(int a0, int a1, pe_addr_t a2);
void func_800CA934(int a0, int a1, pe_addr_t a2);
void func_800C815C(int a0, int a1, pe_addr_t a2);
void func_800CABC8(int a0, int a1, pe_addr_t a2);
int func_800C8D34(pe_addr_t a0);
int func_800C7BA0(pe_addr_t a0);
int func_800CA574(pe_addr_t a0);
void func_800C8870(int a0, int a1, pe_addr_t a2);
void func_800C9868(int a0, int a1, pe_addr_t a2);

/* ── batch 11 (batch11_hi_port.c) ────────────────────────────────── */
void func_800C8F94(int a0, int a1, pe_addr_t a2);
void func_800C8970(int a0, int a1, pe_addr_t a2);
void func_800CB8E0(int a0, int a1, pe_addr_t a2);
void func_800CD2EC(int a0, int a1, pe_addr_t a2);
void func_800C8A88(int a0, int a1, pe_addr_t a2);
void func_800CB9F8(int a0, int a1, pe_addr_t a2);
void func_800C8270(int a0, int a1, pe_addr_t a2);
void func_800C9268(int a0, int a1, pe_addr_t a2);
void func_800CACDC(int a0, int a1, pe_addr_t a2);

/* ── batch 12 (batch12_hi_port.c) ────────────────────────────────── */
void func_80088E64(pe_addr_t a0, int a1);
void func_800CD404(int a0, int a1, pe_addr_t a2);
void func_800CD0BC(int a0, int a1, pe_addr_t a2);
void func_800CA824(int a0, int a1, pe_addr_t a2);
void func_8008B2CC(pe_addr_t a0);
void func_8008B410(pe_addr_t a0);

/* ── batch 13 (batch13_hi_port.c) ────────────────────────────────── */
void func_800CAA38(int a0, int a1, pe_addr_t a2);
void func_800CC0E0(int a0, int a1, pe_addr_t a2);
int func_800DF9B0(int a0, pe_addr_t a1);
void func_80071754(pe_addr_t a0);

/* ── batch 14 (batch14_hi_port.c) ────────────────────────────────── */
void func_8008E4E8(pe_addr_t a0, unsigned int a1);
void func_8008E664(pe_addr_t a0, unsigned int a1);
void func_800CC2C4(int a0, int a1, pe_addr_t a2);
void func_800CB750(int a0, int a1, pe_addr_t a2);

/* ── batch 15 (batch15_hi_port.c) ────────────────────────────────── */
void func_800CF4B4(int a0, int a1, pe_addr_t a2);
int func_800C61A8(pe_addr_t a0, pe_addr_t a1);
void func_800CF6F8(pe_addr_t a0, pe_addr_t a1, int a2);
int func_800CCBA8(pe_addr_t a0);

/* ── batch 16 (batch16_hi_port.c) ────────────────────────────────── */
int func_800D7A1C(int a0, pe_addr_t a1);
int func_800D7E78(int a0, pe_addr_t a1);
int func_800D8D14(int a0, pe_addr_t a1);
int func_800DC910(int a0, pe_addr_t a1);
int func_800DBCD8(int a0, pe_addr_t a1);
int func_800D9E5C(int a0, pe_addr_t a1);
int func_800DA780(int a0, pe_addr_t a1);
int func_800D5CE4(int a0, pe_addr_t a1);
int func_800D6C58(int a0, pe_addr_t a1);

/* ── batch 17 (batch17_hi_port.c) ────────────────────────────────── */
int func_800D5898(int a0, pe_addr_t a1);
int func_800D4EA4(int a0);
void func_800CBFC4(int a0, int a1, pe_addr_t a2);
int func_800D629C(int a0, pe_addr_t a1);
void func_800D1AE0(pe_addr_t a0, int a1, int a2, int a3);

/* ── batch 18 (batch18_hi_port.c) ────────────────────────────────── */
int func_800DB0D0(int a0, pe_addr_t a1);
int func_800DF6AC(int a0, pe_addr_t a1);
int func_800C5EB0(pe_addr_t a0, int a1, pe_addr_t a2);

/* ── batch 19 (batch19_hi_port.c) ────────────────────────────────── */
int func_800CBCA4(pe_addr_t a0);
int func_80065C38(int a0, int a1);
unsigned short func_8007485C(int a0, int a1, int a2);
unsigned short func_800748C0(int a0, int a1, int a2);

/* ── batch 20 (batch20_hi_port.c) ────────────────────────────────── */
int func_800D6E3C(int a0, pe_addr_t a1);
int func_80085A64(int a0);
void func_8007BBB0(void);

/* ── batch 21 (batch21_hi_port.c) ────────────────────────────────── */
int func_800679C4(int dx, int dy, int dz);
int func_800DAB98(int a0, pe_addr_t a1);
int func_800DAF8C(int a0, pe_addr_t a1);
int func_800DB5F4(int a0, pe_addr_t a1);

/* ── sequence opcode handlers (seq_ops_hi_port.c) ───────────────── */
void func_8008FCB4(pe_addr_t a0);
void func_800904A0(pe_addr_t a0);
void func_80090A0C(pe_addr_t a0);
void func_80090C38(pe_addr_t a0);
void func_80090C4C(pe_addr_t a0);
void func_80090C60(pe_addr_t a0);
void func_80090C74(pe_addr_t a0);
void func_80090F54(pe_addr_t a0);
void func_8008F868(pe_addr_t a0);
void func_8008F880(pe_addr_t a0);
void func_8008F84C(pe_addr_t a0);
void func_8008FFC0(pe_addr_t a0);
void func_800900E4(pe_addr_t a0);
void func_8008FBD4(pe_addr_t a0);
void func_8008FCBC(pe_addr_t a0);
void func_80090948(pe_addr_t a0);
void func_8008F4E8(pe_addr_t a0);
void func_8008FBFC(pe_addr_t a0);
void func_8008FCE4(pe_addr_t a0);
void func_80090574(pe_addr_t a0);
void func_800905EC(pe_addr_t a0);
void func_80090614(pe_addr_t a0);
void func_8009063C(pe_addr_t a0);
void func_80090664(pe_addr_t a0);
void func_8009068C(pe_addr_t a0);
void func_800906B4(pe_addr_t a0);
void func_8008F224(pe_addr_t a0);
void func_8008F274(pe_addr_t a0);
void func_8008F37C(pe_addr_t a0);
void func_8008F328(pe_addr_t a0);
void func_8008F430(pe_addr_t a0);
void func_8008F470(pe_addr_t a0);
void func_8008F514(pe_addr_t a0);
void func_8008F59C(pe_addr_t a0);
void func_8008F608(pe_addr_t a0);
void func_8008F6B0(pe_addr_t a0);
void func_8008F6F4(pe_addr_t a0);
void func_8008F784(pe_addr_t a0);
void func_8008F7BC(pe_addr_t a0);
void func_8008FB00(pe_addr_t a0);
void func_8008FC28(pe_addr_t a0);
int func_8008FC78(pe_addr_t obj);
void func_8008FE10(pe_addr_t a0);
void func_8008FE68(pe_addr_t a0);
void func_8008FFE4(pe_addr_t a0);
void func_80090108(pe_addr_t a0);
void func_80090078(pe_addr_t a0);
void func_8009071C(pe_addr_t a0);
int func_8009090C(pe_addr_t a0);
void func_80090754(pe_addr_t a0);
void func_800907DC(pe_addr_t a0);
void func_8009086C(pe_addr_t a0);
void func_80090970(pe_addr_t a0);
void func_800909C0(pe_addr_t a0);
void func_80090A20(pe_addr_t a0);
void func_80090A64(pe_addr_t a0);
void func_80090B30(pe_addr_t a0);
void func_80090BA0(pe_addr_t a0);
void func_80090D54(pe_addr_t a0, int a1);
void func_80090E20(pe_addr_t a0, int a1);
void func_80090C88(pe_addr_t a0);
void func_8008F898(pe_addr_t a0, unsigned int a1);
void func_8008F9CC(pe_addr_t a0, unsigned int a1);
void func_8008FD10(pe_addr_t a0);
void func_8008FEFC(pe_addr_t a0);
void func_800904C4(pe_addr_t a0);

/* ── memory-card tag setters (card_tag_hi_port.c) ──────────────────── */
void func_80083E50(pe_addr_t a0, unsigned char a1);
void func_80083E84(pe_addr_t a0, unsigned char a1);
void func_80083EA4(pe_addr_t a0, unsigned char a1);
void func_80083EC4(pe_addr_t a0, unsigned char a1);
void func_800835C0(pe_addr_t a0);
void func_8008389C(pe_addr_t a0);

/* ── remaining leaves (rest_hi_port.c) ───────────────────────────────── */
int func_80085D84(int a0);
void func_8008F178(pe_addr_t a0, int a1);
unsigned short func_80074774(int a0, int a1, int a2, int a3, int a4, int a5, int a6);
void func_800CED3C(int a0);

#endif /* PE_HAND_HI_PROTOS_H */
