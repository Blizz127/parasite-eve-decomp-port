#include "pe_port_compat.h"
/*
 * func_8006A5BC — boot init with two VSync-polled wait loops (Boot Rung 1,
 * main's callee). Four setup calls, two structurally identical
 * `while (f() != 1) VSync(0);` loops, then stores func_8007F7A8()'s return
 * as an unsigned short. func_80073A44 = VSync (SDK, see sdk_map.md);
 * func_8007F7A8 already C (matched 5EL). D_800B0DD4 typed unsigned short by
 * its lhu reader (~func_8006E744).
 * ROM: asm/disc1/55430.s @ file 0x5ADBC, 36 words (0x90 bytes), frame 0x18.
 */

extern void func_80085644(void);
extern void func_80086FF8(void);
extern void func_80087024(void);
extern void func_8008682C(int a);
extern int  func_8007ED58(void);
extern int  func_8007F72C(void);
extern uint32_t func_80073A44(int a);   /* VSync (SDK) */
extern int  func_8007F7A8(void);    /* already C, 5EL */
/* D_800B0DD4 is a guest-RAM lvalue macro (psx_compat.h) */

/* func_8006A5BC: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8006A5BC_port.c (src/func_8006A5BC.c); hand port retired (port3 switch-over A). */
