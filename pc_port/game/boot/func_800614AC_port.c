/*
 * Phase 6E-B32 — func_800614AC: packed-color interpolation leaf.
 *
 * Retail body: 36 instructions / 0x90 bytes,
 * 0x800614AC..0x80061538 (exclusive end 0x8006153C), file offset
 * 0x51CAC, live split asm/disc1/51CAC.s.  The complete transcription is
 * independently checked and executed by tools/b32_oracle.py.
 *
 * The leaf masks the input to 24 bits, averages the adjacent color bytes,
 * packs the result with the retail saturation branches, stores the masked
 * source at D_8009D14C and the packed result at D_8009D150, and returns the
 * packed result.  It has no callees or platform activity.
 */
#include "psx_compat.h"
#include "pe_guest_ram.h"

#define GA_614AC_SOURCE 0x8009D14Cu /* retail $gp + 0x3DC */
#define GA_614AC_RESULT 0x8009D150u /* retail $gp + 0x3E0 */

/* func_800614AC: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800614AC_port.c (src/func_800614AC.c); hand port retired (port3 switch-over A1). */
