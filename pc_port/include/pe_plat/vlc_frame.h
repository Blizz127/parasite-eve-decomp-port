#ifndef PE_PLAT_VLC_FRAME_H
#define PE_PLAT_VLC_FRAME_H
#include <stdint.h>
/* Guest row 0x8010C89C (libpress VLC frame decoder): bounded/resumable guest adapter around the clean-room decoder
 * (platform/pe_plat_vlc_frame.c, pe_vlc_decode.c).  Returns 0 (frame complete) or 1 (bounded exit, call again with a0 == 0). */
int pe_plat_vlc_frame_host(uint32_t a0, uint32_t a1, uint32_t a2, uint32_t a3);
/* same, also returning the observed a0'/a1' register values (cursor: empirical fit; caller use unverified) */
int pe_plat_vlc_frame_host_regs(uint32_t a0, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t *a0_out, uint32_t *a1_out);
#define func_8010C89C pe_plat_vlc_frame_host
/* Canonical guest-name prototype for the binding above (generated
 * from the pe_plat prototypes; tools read callee signatures from these). */
int func_8010C89C(uint32_t a0, uint32_t a1, uint32_t a2, uint32_t a3);
#endif
