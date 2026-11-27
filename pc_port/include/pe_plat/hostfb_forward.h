/* Host-contract forwards: the guest-callable names for the display-sync and
 * display-mask entry points, bound to the host framebuffer API
 * (HostFB_VSync, HostFB_SetDispMask). No original behaviour is implemented
 * here; each function only forwards its argument to the host call. */
#ifndef PE_PLAT_HOSTFB_FORWARD_H
#define PE_PLAT_HOSTFB_FORWARD_H
#include <stdint.h>
uint32_t pe_plat_hostfb_vsync(int mode);
void pe_plat_hostfb_set_disp_mask(int mask);
#define func_80073A44 pe_plat_hostfb_vsync
#define func_80074D28 pe_plat_hostfb_set_disp_mask
/* Canonical guest prototypes (read by the port generators). */
uint32_t func_80073A44(int mode);
void func_80074D28(int mask);
#endif
