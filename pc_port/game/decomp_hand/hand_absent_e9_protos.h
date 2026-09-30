/*
 * Host prototypes for pc_port/game/decomp_hand/e9_guest_ptr_port.c: leaves
 * the generator stopped emitting when E9/E9b learned that a loaded 32-bit
 * guest word is not a host pointer (2026-09-23).  Signatures are the ones the
 * generated TUs exported, so existing callers bind unchanged.
 */
#ifndef PE_HAND_ABSENT_E9_PROTOS_H
#define PE_HAND_ABSENT_E9_PROTOS_H

#include "psx_compat.h"

void func_80030640(void);
int func_80019FE0(void);
int func_80018D50(pe_addr_t pe_a0);
int func_80018DD4(pe_addr_t pe_a0);
void func_8004006C(pe_addr_t pe_dst, pe_addr_t pe_fmt);
void func_80088F6C(pe_addr_t pe_a0, int a1);
void func_8004A6CC(void);

/* absent_save_port.c (batch-7 save-block serialisers) */
void func_8003F800(void);
void func_8003FBD8(void);
int func_80040B80(pe_addr_t o);

/* rcnt2_port.c (root counter 2, pe_rcnt2.h) */
void func_80084FC4(unsigned int limit);
int func_80084FE4(void);

#endif
