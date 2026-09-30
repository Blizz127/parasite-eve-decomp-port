/*
 * Host prototypes for the port_absent lane's overlay-resident hand adapters
 * (pc_port/game/decomp_hand/absent_ovl_port.c, VRAM 0x800C0000..).
 *
 * Every function here is a hand adaptation of the verified matched leaf
 * `src/func_XXXXXXXX.c`: retail pointer parameters become 32-bit guest
 * addresses (`pe_addr_t`), every other parameter keeps the leaf's type.
 * gen_decomp_ports.py reads these as canonical host signatures.
 */
#ifndef PE_HAND_ABSENT_OVL_PROTOS_H
#define PE_HAND_ABSENT_OVL_PROTOS_H

#include <stdint.h>
#include "psx_compat.h"

/* Guest scratch for retail STACK temporaries whose address these leaves hand
 * to a guest-address callee (a host stack address has no guest meaning).
 * Storage-location substitution only: the bytes written/read are retail's
 * and none outlives the call that uses them.  0x801FFA00..0x801FFAFF,
 * disjoint from PE_HAND_HI_STACK (0x801FFC00) and PE_HAND_LO_STACK_TEMP
 * (0x801FFD00).  Layout (byte offsets):
 *   +0x00 short[4]  position buffer (`pos`/`buf`)
 *   +0x08 short[4]  angle vector (`vec`)
 *   +0x10 8 bytes   colour block (`blk`/`b`, int[2] or B4)
 *   +0x20 M16       func_800C65E4 `m`   (copy of D_800C213C)
 *   +0x30 M16       func_800C65E4 `t1`
 *   +0x40 M16       func_800C65E4 `t2`
 *   +0x50 Vec       func_800C65E4 `d`
 */
#define PE_ABSENT_OVL_STACK      0x801FFA00u
#define PE_ABSENT_OVL_POS        (PE_ABSENT_OVL_STACK + 0x00u)
#define PE_ABSENT_OVL_VEC        (PE_ABSENT_OVL_STACK + 0x08u)
#define PE_ABSENT_OVL_BLK        (PE_ABSENT_OVL_STACK + 0x10u)
#define PE_ABSENT_OVL_C65E4_M    (PE_ABSENT_OVL_STACK + 0x20u)
#define PE_ABSENT_OVL_C65E4_T1   (PE_ABSENT_OVL_STACK + 0x30u)
#define PE_ABSENT_OVL_C65E4_T2   (PE_ABSENT_OVL_STACK + 0x40u)
#define PE_ABSENT_OVL_C65E4_D    (PE_ABSENT_OVL_STACK + 0x50u)

void func_800C65E4(pe_addr_t a0, pe_addr_t a1, pe_addr_t a2);
int func_800D5010(int a0, pe_addr_t a1);
int func_800D6A1C(int a0, pe_addr_t a1);
int func_800D8B6C(int a0, pe_addr_t a1);
int func_800D9554(int a0, pe_addr_t a1);
int func_800D9FD4(int a0, pe_addr_t a1);
int func_800DA5D4(int a0, pe_addr_t a1);
int func_800DA934(int a0, pe_addr_t a1);
int func_800DBA9C(int a0, pe_addr_t a1);
int func_800DBE6C(int a0, pe_addr_t a1);
int func_800DC750(int a0, pe_addr_t a1);
int func_800DCA80(int a0, pe_addr_t a1);
int func_800DCE94(int a0, pe_addr_t a1);
int func_800DD19C(int a0, pe_addr_t a1);
int func_800DD76C(int a0, pe_addr_t a1);
void func_800E026C(pe_addr_t a0);
void func_800E03A0(pe_addr_t a0);

#endif /* PE_HAND_ABSENT_OVL_PROTOS_H */
