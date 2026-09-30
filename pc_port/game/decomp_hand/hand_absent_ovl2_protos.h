/*
 * Host prototypes for the port_absent lane's round-2 overlay-resident hand
 * adapters (pc_port/game/decomp_hand/absent_ovl2_port.c).
 *
 * Every function is a hand adaptation of the verified matched leaf
 * `src/func_XXXXXXXX.c`: the retail `short *a1` record becomes a 32-bit guest
 * address (`pe_addr_t`), the mode argument keeps the leaf's type.
 * gen_decomp_ports.py reads these as canonical host signatures.
 */
#ifndef PE_HAND_ABSENT_OVL2_PROTOS_H
#define PE_HAND_ABSENT_OVL2_PROTOS_H

#include <stdint.h>
#include "psx_compat.h"

/* Guest scratch for retail STACK temporaries whose address these leaves hand
 * to a guest-address callee (a host stack address has no guest meaning).
 * Storage-location substitution only: the bytes written/read are retail's
 * and none outlives the call that uses them.  0x801FF800..0x801FF8FF,
 * disjoint from PE_ABSENT_OVL_STACK (0x801FFA00), PE_HAND_HI_STACK
 * (0x801FFC00) and PE_HAND_LO_STACK_TEMP (0x801FFD00).  Layout:
 *   +0x00 short[4]  `pos`
 *   +0x08 short[4]  `vec`
 *   +0x10 8 bytes   `blk` (B4 / B8 / int[2])
 *   +0x18 8 bytes   `blk2`
 *   +0x20 short[4]  `v` (func_800DEA30)
 * A retail local that the leaf reads without writing on some path (e.g.
 * func_800DD380 `pos`/`blk` when a1[4] >= 2) is whatever the scratch holds —
 * retail reads stale stack there too; no value is invented. */
#define PE_HAND_OVL2_STACK   0x801FF800u
#define PE_OVL2_POS          (PE_HAND_OVL2_STACK + 0x00u)
#define PE_OVL2_VEC          (PE_HAND_OVL2_STACK + 0x08u)
#define PE_OVL2_BLK          (PE_HAND_OVL2_STACK + 0x10u)
#define PE_OVL2_BLK2         (PE_HAND_OVL2_STACK + 0x18u)
#define PE_OVL2_V            (PE_HAND_OVL2_STACK + 0x20u)

int func_800D7764(int a0, pe_addr_t a1);
int func_800D7B70(int a0, pe_addr_t a1);
int func_800D7FBC(int a0, pe_addr_t a1);
int func_800D8E74(int a0, pe_addr_t a1);
int func_800DACA4(int a0, pe_addr_t a1);
int func_800DC5BC(int a0, pe_addr_t a1);
int func_800DD380(int a0, pe_addr_t a1);
int func_800DDD70(int a0, pe_addr_t a1);
int func_800DEA30(int a0, pe_addr_t a1);
int func_800DEFFC(int a0, pe_addr_t a1);

#endif /* PE_HAND_ABSENT_OVL2_PROTOS_H */
