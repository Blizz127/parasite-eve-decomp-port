/*
 * Host prototypes for the port_absent `cd` group (created 2026-09-23; header
 * rewritten 2026-11-27 to match what the file declares today).
 *
 * This header only declares: pointer parameters/returns are 32-bit guest
 * addresses (pe_addr_t), everything else keeps the integer type.  No body is
 * defined in this file or in absent_cd_port.c any more (absent_cd_port.c now
 * defines only PE_Card_ICall and static helpers).  gen_decomp_ports.py reads
 * the declarations as canonical host signatures.  Provider of each declared
 * function, by address, from gen_decomp_ports.py (PE_PLAT_REPLACED):
 *   libspu  0x8007D614 pe_plat_spu_fidma.c
 *   libcd/libds  0x80080F64 pe_plat_cd_gen.c; 0x80081110 pe_plat_81110.c;
 *           0x800819D8 pe_plat_cd_gen2.c; 0x8008214C pe_plat_ds_gen11.c;
 *           0x80082444 pe_plat_cd_gen6.c
 *   libpad  0x8008284C and 0x800837C8 pe_plat_pad_actinfo.c;
 *           0x80084B44 pe_plat_pad_state.c
 *   libcard 0x80082B08 and 0x80083DF0 pe_plat_card_gen14.c;
 *           0x800840DC pe_plat_card_gen8.c; 0x80084EB0 pe_plat_card_gen17.c
 * Each provider carries its own provenance statement; none is made here.
 * The earlier header text (hand adaptations of matched leaves, and a note on
 * the argument register of 0x80083014, whose provider is pe_plat_pad_gen24.c
 * and which is not declared here) described bodies that are gone.
 */
#ifndef PE_HAND_ABSENT_CD_PROTOS_H
#define PE_HAND_ABSENT_CD_PROTOS_H

#include <stdint.h>
#include "psx_compat.h"

/* Guest scratch window 0x801FFB80..0x801FFBFF, disjoint from PE_HAND_HI_STACK
 * (0x801FFC00) and PE_HAND_LO_STACK_TEMP (0x801FFD00).  Nothing in the tree
 * references this macro now (the stack-buffer user was func_80082444, whose
 * provider is pe_plat_cd_gen6.c). */
#define PE_HAND_CD_STACK 0x801FFB80u

void func_8007D614(void);
void func_80080F64(int a0);
void func_80081110(void);
int  func_800819D8(int a0, pe_addr_t a1);
void func_8008214C(unsigned char a0, int a1);
void func_80082444(unsigned char a0);
int  func_8008284C(int a0, int a1, int a2);
int  func_80082B08(void);
int  func_800837C8(pe_addr_t a0, int a1);
int  func_80083DF0(pe_addr_t a0);
int  func_800840DC(pe_addr_t a0);
void func_80084B44(void);
void func_80084EB0(pe_addr_t a0);

#endif /* PE_HAND_ABSENT_CD_PROTOS_H */
