#ifndef PE_PLAT_VLC_FRAME_H
#define PE_PLAT_VLC_FRAME_H
#include <stdint.h>
/* Guest-shaped adapter for the libpress VLC frame decoder row 0x8010C89C (see pe_plat_vlc_frame.c).
 * NOT bound yet: binding = delete the hand implementation of func_8010C89C, add the two lines below
 * (as pe_plat/mdec_expand.h does for 0x8010BD4C), and keep platform/pe_plat_vlc_frame.c in the build.
 *   #define func_8010C89C pe_plat_vlc_frame_host
 *   int func_8010C89C(uint32_t a0, uint32_t a1, uint32_t a2, uint32_t a3);   (canonical prototype) */
int pe_plat_vlc_frame_host(uint32_t a0, uint32_t a1, uint32_t a2, uint32_t a3);
/* same, also returning the observed a0'/a1' register values (cursor: empirical fit; caller use unverified) */
int pe_plat_vlc_frame_host_regs(uint32_t a0, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t *a0_out, uint32_t *a1_out);
#endif
