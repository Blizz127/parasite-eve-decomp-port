/*
 * BIOS A-function string/memory routines reached through the EXE's
 * `li t2,0xA0; jr t2; li t1,N` trampolines, adapted to guest memory.
 *
 * The PS1 BIOS is not in this tree, so the semantics come from the psx-spx
 * kernel/BIOS reference, sections "BIOS String Functions" and "BIOS Memory
 * Fill/Copy/Compare" (https://psx-spx.consoledev.net/kernelbios/, source
 * psx-spx.github.io docs/kernelbios.md), quoted at each definition.  All
 * pointers are 32-bit guest addresses; every byte goes through PE_Load/Store
 * (so the KUSEG/KSEG1 mirrors and bounds checks apply).
 *
 *   trampoline     BIOS     routine
 *   func_80072A54  A(17h)   strcmp(str1, str2)
 *   func_80071A04  A(18h)   strncmp(str1, str2, maxlen)
 *   func_80071A14  A(19h)   strcpy(dst, src)
 *   func_80072314  A(1Bh)   strlen(src)
 *   func_80071A34  A(2Ah)   memcpy(dst, src, len)
 *   func_80071A44  A(2Bh)   memset(dst, fillbyte, len)
 *   (A(28h) bzero is func_80071A24_port.c.)
 */
#ifndef PE_BIOS_STRING_H
#define PE_BIOS_STRING_H
#include <stdint.h>
#include "pe_guest_ram.h"

int32_t func_80072A54(pe_addr_t str1, pe_addr_t str2);
int32_t func_80071A04(pe_addr_t str1, pe_addr_t str2, int32_t maxlen);
pe_addr_t func_80071A14(pe_addr_t dst, pe_addr_t src);
int32_t func_80072314(pe_addr_t src);
pe_addr_t func_80071A34(pe_addr_t dst, pe_addr_t src, int32_t len);
pe_addr_t func_80071A44(pe_addr_t dst, int32_t fillbyte, int32_t len);

/* Host C-library sprintf over guest memory for the EXE's Psy-Q libc
 * formatter func_80071A84 (variadic; its full frame-level translation is
 * the separate PE_FormatterFrame model).  Supports the conversions the
 * game's formats use: flags '-' '0' ' ' '+', width, precision, length
 * modifiers h/l (ignored: int is 32-bit), and d i u x X o c s p %.
 * `args` are the 32-bit argument words in call order (a2, a3, stack...);
 * %s arguments are guest addresses.  Writes the NUL; returns the length. */
int32_t PE_Bios_Sprintf(pe_addr_t dst, pe_addr_t fmt, const uint32_t *args, int nargs);
/* Same, into a host buffer of `cap` bytes (always NUL-terminated). */
int32_t PE_Bios_SprintfHost(char *dst, uint32_t cap, pe_addr_t fmt, const uint32_t *args, int nargs);

#endif
