/*
 * Host prototypes for the port_absent lane's SDK-range hand adapters
 * (pc_port/game/decomp_hand/absent_sdk_port.c), 2026-09-23.
 *
 * Each is a hand adaptation of the verified matched leaf src/func_X.c:
 * retail pointer parameters/returns are 32-bit guest addresses (pe_addr_t),
 * code-pointer return values are the retail VMA, everything else keeps the
 * leaf's type.  gen_decomp_ports.py reads these as canonical host
 * signatures.
 */
#ifndef PE_HAND_ABSENT_SDK_PROTOS_H
#define PE_HAND_ABSENT_SDK_PROTOS_H

#include <stdint.h>
#include "psx_compat.h"

/* Guest scratch reserved for this lane's retail stack temporaries
 * (0x801FFB00..0x801FFB7F).  Currently unused: no leaf here hands a stack
 * buffer to a guest-address callee. */
#define PE_HAND_ABSENT_SDK_STACK 0x801FFB00u

void      func_80038954(pe_addr_t a0, pe_addr_t a1, pe_addr_t a2, int a3);
int       func_8005CAEC(void);
int       func_800727B4(pe_addr_t name, int arg);
int       func_80072950(pe_addr_t a0, int a1, int a2);
void      func_80073D88(void);
void      func_80073DB8(void);
pe_addr_t func_80073E28(void);
void      func_80073F00(void);
pe_addr_t func_800743B4(void);
void      func_8007440C(void);
pe_addr_t func_800744D4(void);
int       func_80074C14(int a0);
int       func_80074CC8(int a0);
void      func_80074FD4(int a0, unsigned char a1, unsigned char a2,
                        unsigned char a3);
pe_addr_t func_800751E4(pe_addr_t a0, int a1);
unsigned int func_80075B1C(void);
void      func_80075CE8(pe_addr_t dd, pe_addr_t env);
/* trailing a1: the caller's live $a1, forwarded (see the definitions) */
void      func_80090AEC(pe_addr_t a0, unsigned int a1);
void      func_80090B5C(pe_addr_t a0, unsigned int a1);

#endif /* PE_HAND_ABSENT_SDK_PROTOS_H */
