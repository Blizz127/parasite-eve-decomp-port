#ifndef PE_GTE_COMMANDS_H
#define PE_GTE_COMMANDS_H
#include <stdint.h>
/* Native arithmetic only. Unknown operation returns -1 without a state write. */
int PE_GTE_ExecuteNative(uint32_t word);
/* Host presentation only (Graphics > Sub-pixel Geometry).  While capture is
 * on, every RTPS/RTPT result also stores the fraction of its screen position
 * under the packed SXY word it writes; no GTE register or FLAG changes.
 * Off by default; switching it clears the table. */
void PE_GTE_SetSubpixelCapture(int on);
int PE_GTE_GetSubpixelCapture(void);
/* 1 with *frac = x | y << 8 (each in 1/256 pixel) when sxy was recorded. */
int PE_GTE_SubpixelLookup(uint32_t sxy, uint32_t *frac);
#endif
