/*
 * Host prototypes for pc_port/game/decomp_hand/absent_r4_port.c
 * (port_absent lane round 4, 2026-09-24).
 */
#ifndef PE_HAND_ABSENT_R4_PROTOS_H
#define PE_HAND_ABSENT_R4_PROTOS_H

#include "psx_compat.h"

/* Guest scratch reserved for round-4 retail stack buffers handed to
 * guest-address callees: 0x801FF500..0x801FF5FF.  None of the four leaves
 * needs it today (no stack buffer escapes to a callee). */
#define PE_HAND_R4_STACK 0x801FF500u

int func_80084B78(pe_addr_t a0);
void func_80087FA0(pe_addr_t v, unsigned int bit);
void func_8008D844(void);
int func_8008E2DC(pe_addr_t a0);

#endif
