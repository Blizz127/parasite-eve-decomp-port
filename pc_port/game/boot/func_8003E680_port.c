/*
 * Phase 6D-S — Boot subsystem-init dispatcher.
 *
 * Phase 6E-B6: callback registration now goes through the REAL
 * func_80073D24 (guest-backed VBlank slot table, pe_libetc.c).  The
 * PE_Callback_Bind of guest 0x8003E91C to its host stub is host plumbing
 * (idempotent), the counterpart of retail passing the guest address.
 * All remaining BOOTSTRAP_RET callees go through the centralized
 * Bootstrap_* policy.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

extern void func_80070D10(void);
extern unsigned int func_80070D6C(void);
extern void func_80036DC8(void);
extern void func_8003E91C(void);
extern void func_800371A4(int);
extern void func_80029388(void);
extern void func_8005BCA8(void);
extern void func_800124F8(void);
extern void func_8001A890(void);
extern void func_80034F10(void);
extern void func_8006536C(void);

/* func_8003E680: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8003E680_port.c (src/func_8003E680.c); hand port retired (switch1 lane, audit klass a-replaceable, generator-verified eligible). */
