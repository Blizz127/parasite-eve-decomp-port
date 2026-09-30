/*
 * Host prototypes for pc_port/game/decomp_hand/absent_r3_port.c
 * (port_absent lane round 3, 2026-09-24).  Pointer parameters of the
 * matched leaves are 32-bit guest addresses (pe_addr_t).
 */
#ifndef PE_HAND_ABSENT_R3_PROTOS_H
#define PE_HAND_ABSENT_R3_PROTOS_H

#include "psx_compat.h"

/* Guest scratch for retail stack buffers these leaves hand to
 * guest-address callees: 0x801FF600..0x801FF6FF.
 *   +0x00 short b1[3]   func_80016910 op 0x960 -> func_800E00CC
 *   +0x08 short b2[3]   func_80016910 op 0x962 -> func_800E00CC
 *   +0x10 u8    d[2]    func_80016910 op 0x9C4 -> func_80035038
 *   +0x18 short buf[3]  func_8003495C mode 1   -> func_800375E0 */
#define PE_HAND_R3_STACK 0x801FF600u

int func_80015964(pe_addr_t a0);
int func_80016910(pe_addr_t a);
void func_8003495C(void);
void func_80042020(int slot, int n);
void func_8004A0C8(void);
int func_8005C688(int min_b, int min_c);
int func_8007A360(int a0);
int func_80081D74(int a0, int a1);
int func_80083F44(pe_addr_t a0);

#endif /* PE_HAND_ABSENT_R3_PROTOS_H */
