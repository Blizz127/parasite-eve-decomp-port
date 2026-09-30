/* Host framebuffer — headless, PPM output */
#ifndef HOST_FRAMEBUFFER_H
#define HOST_FRAMEBUFFER_H

#include <stdint.h>

#include "pe_guest_ram.h"

#define PE_PORT_FB_WIDTH  320
#define PE_PORT_FB_HEIGHT 240

void HostFB_Init(void);
void HostFB_ClearImage(int x, int y, int w, int h, uint8_t r, uint8_t g, uint8_t b);
void HostFB_Present(void);
/* Phase 6E-PRS1 — display presentation.  Reads the DISPENV disp RECT
 * (x/y/w/h halfwords at env+0/2/4/6) from guest RAM and copies that VRAM
 * window into the host framebuffer, decoded through the GPU VRAM
 * authority.  Mask off presents black (retail blanks).  Read-only on
 * guest state; keeps the present counter and frame-limit budget. */
void HostFB_PresentDispEnv(pe_addr_t env);
void HostFB_SetDispMask(int mask);
/* Returns the same value the original 80073A44 produces from its host-side
 * device model: the absolute VBlank counter (800956AC) for negative modes and
 * the entry-time 16-bit timer1 delta for every other mode.  The outer
 * transition loop stores negative and mode-1 query results, so a void shim
 * cannot preserve behavior.  Waiting modes also refresh the timer1 baseline
 * the host counter is measured against. */
uint32_t HostFB_VSync(int mode);
/* Current timer1-lane counter (host model of 1F801110 in the VBlank lane). */
uint32_t HostFB_VSyncTimer(void);
/* libetc VBlank callback dispatcher for the non-device VBlank edge (set by
 * PE_Callback_Init; func_8007440C semantics). */
void HostFB_SetVBlankDispatch(int (*dispatch)(void));
/* Advance the modeled CD device clock by the quantum a guest poll-loop
 * iteration stands for, then deliver its IRQs.  The guest's 2000-iteration
 * stream waits (func_80121270 call batches) represent far more real CPU time
 * than one 1024-cycle counter query, so a stream wait uses this larger
 * quantum; otherwise the modeled 2x drive delivers only ~3 sectors inside the
 * guest's own retry budget and the player restarts mid-frame from sector 0.
 * Read-only on guest state beyond the device/IRQ model. */
void HostFB_StreamTick(void);
void HostFB_DrawSync(int mode);
int  HostFB_WritePPM(const char *path);
const uint8_t *HostFB_GetPixels(void);
void HostFB_GetState(int *vsync, int *drawsync, int *presented, int *mask);
/* Display-only refresh after a VBlank scan-out (not a present). */
void HostFB_SetScanoutHook(void (*hook)(void));
/* Number of idle-VBlank scan-outs so far (diagnostics, PE_FRAME_TRACE). */
uint32_t HostFB_ScanOutCount(void);

#endif
