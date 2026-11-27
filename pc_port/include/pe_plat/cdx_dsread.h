/* Bind the libds read-start entry DsRead (guest 0x80080E34) to the clean-room host implementation in
 * platform/pe_plat2_cdx_dsread.c.  Written clean-room from the black-box vector set cdgen49b
 * (docs/evidence/native-cdx-20261126/cdgen49b), the cdgen49 report's contract text and psx-spx
 * (CD-ROM location format); no SDK source, decompiled source, tables or disassembly were read. */
#ifndef PE_PLAT_CDX_DSREAD_H
#define PE_PLAT_CDX_DSREAD_H
#include <stdint.h>

/* pos: guest pointer to the four location bytes, or 0 for "the last position"; sectors / dest: the read
 * request published to the driver state; mode: low byte, with bit 5 set, is the command-sequence mode.
 * Returns 0 when the driver is busy or in error, otherwise the command-sequence result. */
int pe_plat_cdxdsread_read(uint32_t pos, uint32_t sectors, uint32_t dest, uint32_t mode);

#define func_80080E34 pe_plat_cdxdsread_read
/* Canonical guest prototype (read by the port generators; it expands to the pe_plat declaration above). */
int func_80080E34(uint32_t pos, uint32_t sectors, uint32_t dest, uint32_t mode);
#endif
