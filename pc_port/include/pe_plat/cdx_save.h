/*
 * pe_plat libpad bring-up pair (batch cdgen45): the communication start
 * wrapper 0x80082534 and the pad record initialiser 0x800844E4.
 *
 * PROVENANCE: written clean-room from the black-box vector sets cdgen45
 * (docs/evidence/native-cdx-20261126), the recorded seam contracts and
 * psx-spx; no SDK source, decompiled source, tables or disassembly were
 * read.  Provider: pc_port/platform/pe_plat2_cdx_save.c.  The retail names
 * are bound by object-like macros, so no func_XXXXXXXX symbol is defined by
 * the port for these two entries.
 *
 * Naming: the provider symbols use the prefix pe_plat_cdxsave_ because
 * gen_decomp_ports.py and pe_plat_canonical_protos.py only accept
 * `#define func_X pe_plat_<word>` bindings.
 */
#ifndef PE_PLAT_CDX_SAVE_H
#define PE_PLAT_CDX_SAVE_H
#include <stdint.h>

/* 0x80082534: libpad communication start (PadStartCom). */
void pe_plat_cdxsave_pad_comm_start(void);
/* 0x800844E4: pad record / hook-table initialiser (PadInitDirect); the
 * vectors record a return value of 1 that no caller uses (void here). */
void pe_plat_cdxsave_pad_init_direct(uint32_t first, uint32_t second);

#define func_80082534 pe_plat_cdxsave_pad_comm_start
#define func_800844E4 pe_plat_cdxsave_pad_init_direct

/* Canonical guest-name prototypes for the bindings above (copied from the
 * pe_plat_cdxsave_ prototypes; tools read callee signatures from these). */
void func_80082534(void);
void func_800844E4(uint32_t first, uint32_t second);

#endif
