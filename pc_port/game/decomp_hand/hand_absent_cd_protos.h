/*
 * Host prototypes for the port_absent `cd` group (2026-09-23): hand adapters
 * in absent_cd_port.c for matched leaves 0x8007D614..0x80084EB0 (libspu DMA
 * IRQ tail, libcard / memory-card controller).  Each is a hand adaptation of
 * the verified matched leaf src/func_XXXXXXXX.c: pointer parameters/returns
 * are 32-bit guest addresses (pe_addr_t), everything else keeps the leaf's
 * type.  gen_decomp_ports.py reads these as canonical host signatures.
 *
 * func_80083014 is declared `void func_80083014(void)` in its own leaf but
 * retail forwards the caller's live $a0 to the table callback it dispatches
 * (pe_dis.sh 0x80083014 0xC8: $a0 is never written before `jalr v0` at
 * 0x80083040); every retail caller passes the record (src/func_80082B70.c
 * declares `func_80083014(unsigned char *)`), so the register is spelled as
 * the explicit parameter here.
 */
#ifndef PE_HAND_ABSENT_CD_PROTOS_H
#define PE_HAND_ABSENT_CD_PROTOS_H

#include <stdint.h>
#include "psx_compat.h"

/* Guest scratch for retail stack buffers a leaf hands to a guest-address
 * callee (func_80082444's `char buf[8]`).  0x801FFB80..0x801FFBFF, disjoint
 * from PE_HAND_HI_STACK (0x801FFC00) and PE_HAND_LO_STACK_TEMP (0x801FFD00).
 * Storage-location substitution only: the bytes are retail's and none
 * outlive the call that uses them. */
#define PE_HAND_CD_STACK 0x801FFB80u

void func_8007D614(void);
void func_80080F64(int a0);
void func_80081110(void);
int  func_800819D8(int a0, pe_addr_t a1);
void func_8008214C(unsigned char a0, int a1);
void func_80082444(unsigned char a0);
int  func_80082778(int a0, int a1, int a2);
int  func_8008284C(int a0, int a1, int a2);
int  func_800829C4(int a0);
int  func_80082B08(void);
int  func_80082B70(void);
void func_80082CF0(void);
int  func_80082E00(pe_addr_t a0);
void func_80083014(pe_addr_t a0);
int  func_800837C8(pe_addr_t a0, int a1);
int  func_80083DF0(pe_addr_t a0);
void func_80083F04(int a0);
int  func_8008401C(pe_addr_t a0);
int  func_800840DC(pe_addr_t a0);
int  func_80084168(pe_addr_t a0);
void func_80084B44(void);
void func_80084EB0(pe_addr_t a0);

#endif /* PE_HAND_ABSENT_CD_PROTOS_H */
