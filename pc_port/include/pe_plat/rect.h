/* Retail libgpu RECT (x, y, w, h halfwords), shared by psx_compat.h and the
 * pe_plat bindings that take a RECT pointer. */
#ifndef PE_PLAT_RECT_H
#define PE_PLAT_RECT_H
#include <stdint.h>
typedef struct { int16_t x, y, w, h; } RECT;
#endif
