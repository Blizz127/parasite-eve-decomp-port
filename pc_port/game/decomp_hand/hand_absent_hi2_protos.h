/*
 * Host prototypes for pc_port/game/decomp_hand/absent_hi2_port.c
 * (port_absent lane round 2, 2026-09-24): SPU voice / sequence leaves
 * 0x80087000..0x8008FFFF.  Pointer parameters are guest addresses; retail
 * `long` is 32-bit, so it becomes `int` host-side.
 */
#ifndef PE_HAND_ABSENT_HI2_PROTOS_H
#define PE_HAND_ABSENT_HI2_PROTOS_H

#include "psx_compat.h"

/* Guest scratch for retail stack locals that these leaves hand to
 * guest-address callees (a host stack address has no guest meaning).
 * 0x801FF980..0x801FF9FF, disjoint from the other decomp_hand scratch areas:
 *   +0x00  u32 `acc` of func_80089784/80089980/80089B48/80089D10
 *   +0x10  68-byte RevPreset `entry` of func_8008CF70 (..+0x53) */
#define PE_HAND_HI2_STACK 0x801FF980u

void func_800878F0(int a0, pe_addr_t a1);
void func_8008900C(pe_addr_t a0, unsigned int a1, unsigned int a2, pe_addr_t a3);
void func_80089784(void);
void func_80089980(void);
void func_80089B48(void);
void func_80089D10(void);
void func_8008A400(unsigned int a0, unsigned int a1);
int func_8008CF70(int a0);

#endif /* PE_HAND_ABSENT_HI2_PROTOS_H */
