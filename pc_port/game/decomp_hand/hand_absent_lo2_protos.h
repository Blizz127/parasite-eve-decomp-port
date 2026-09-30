/*
 * Host prototypes for pc_port/game/decomp_hand/absent_lo2_port.c — the
 * port_absent lane round 2 (2026-09-24): matched leaves in
 * 0x80038000..0x8005FFFF that were `absent`.  Retail pointer parameters and
 * returns are 32-bit guest addresses (pe_addr_t / int); every other
 * parameter keeps the leaf's type.
 */
#ifndef PE_HAND_ABSENT_LO2_PROTOS_H
#define PE_HAND_ABSENT_LO2_PROTOS_H

#include "psx_compat.h"

/* Guest scratch for retail stack temporaries of this group handed to a
 * guest-address callee (0x801FF900..0x801FF97F).  None of the current
 * leaves needs one; reserved so the range is documented and disjoint from
 * PE_HAND_LO_STACK_TEMP / PE_HAND_HI_STACK / the round-1 lanes. */
#define PE_HAND_LO2_STACK 0x801FF900u

int func_800389DC(int slot);                 /* save-slot record load    */
unsigned char func_80039184(void);           /* glyph page back          */
unsigned char func_80039310(void);           /* glyph page forward       */
unsigned char func_8003944C(unsigned char key);
unsigned char func_80039678(unsigned char key);
unsigned char func_80039898(void);
void func_8003E0A4(pe_addr_t a0, int a1, int a2);
void func_80045FA4(pe_addr_t self);
void func_8004E704(int mode);
void func_8004E97C(void);
int func_8004ECB4(pe_addr_t self, int key);
int func_8005DAFC(unsigned int a0);          /* returns a guest address  */

#endif /* PE_HAND_ABSENT_LO2_PROTOS_H */
