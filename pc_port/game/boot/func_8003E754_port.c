/*
 * Phase 6E-A — func_8003E754: video init (DISPENV/DRAWENV double-buffer setup).
 *
 * ROM: asm/disc1/2EF54.s @ file 0x2EF54, 114 words, frame 0x38.
 * Caller: func_8003E610 with (w=0x140, h=0xE0).
 * Classification: 1 (translated game logic); all SDK callees are now real
 * (pe_libgpu.c, host_framebuffer.c).
 *
 * Retail ordering notes (reproduced deliberately):
 *   - isrgb24 = 1 is stored to both DISPENVs BEFORE the SetDefDispEnv calls;
 *     SetDefDispEnv then zeroes +0x11 again.  Final retail state: isrgb24 = 0.
 *   - screen overrides (screen.x = 0, screen.y = 8, screen.h = h; screen.w
 *     stays 0) land between the SetDefDispEnv and SetDefDrawEnv calls.
 *   - DRAWENV overrides (tpage = 0, dtd = 1, dfe = 0, isbg = 1, r0/g0/b0 = 0)
 *     land after both SetDefDrawEnv calls.
 */
#include "psx_compat.h"
#include "pe_sdk.h"

/* func_8003E754: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8003E754_port.c (src/func_8003E754.c); hand port retired (audit batch, port3). */
