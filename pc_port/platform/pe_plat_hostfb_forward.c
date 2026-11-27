/* Host-contract forwards, no original behaviour: the guest-callable
 * display-sync and display-mask names call the host framebuffer API with the
 * same argument. Provenance: written for this port as thin glue; it contains
 * no translated code and no SDK structure. */
#include "pe_plat/hostfb_forward.h"
#include "host_framebuffer.h"

uint32_t pe_plat_hostfb_vsync(int mode) { return HostFB_VSync(mode); }

void pe_plat_hostfb_set_disp_mask(int mask) { HostFB_SetDispMask(mask); }
