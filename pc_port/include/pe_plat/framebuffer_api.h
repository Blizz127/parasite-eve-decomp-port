/* Host framebuffer/presentation API: display mask, vsync, present and read-back.
 * Declaration-only header (no function bodies): the port's host model API for the host framebuffer and presentation.
 * Declarations were moved unchanged from the port's own host-model header platform/host_framebuffer.h
 * (commit e15e3ee7c); comments describe hardware behaviour as documented publicly (psx-spx)
 * in the project's own words.  No SDK source, tables or disassembly are reproduced here. */
#ifndef PE_PLAT_FRAMEBUFFER_API_H
#define PE_PLAT_FRAMEBUFFER_API_H

#include <stdint.h>

#include "pe_guest_ram.h"

#define PE_PORT_FB_WIDTH  320
#define PE_PORT_FB_HEIGHT 240

void HostFB_Init(void);
void HostFB_ClearImage(int x, int y, int w, int h, uint8_t r, uint8_t g, uint8_t b);
void HostFB_Present(void);
/* Phase 6E-PRS1 — display presentation.  `env` addresses the port's own host
 * display-environment record in guest RAM; its first four 16-bit words are the
 * display window x, y, w, h.  Copies that VRAM window into the host
 * framebuffer, decoded through the GPU VRAM authority.  A masked display
 * presents black.  Read-only on guest state; keeps the present counter and
 * frame-limit budget. */
void HostFB_PresentDispEnv(pe_addr_t env);
void HostFB_SetDispMask(int mask);
/* Host VSync query.  Returns the absolute VBlank counter for negative modes
 * and the entry-time 16-bit timer1 delta for every other mode.  Callers store
 * the negative-mode and mode-1 query results, so a void shim cannot preserve
 * behavior.  Waiting modes also refresh the timer1 baseline the host counter
 * is measured against. */
uint32_t HostFB_VSync(int mode);
/* Current timer1-lane counter (host model of 1F801110 in the VBlank lane). */
uint32_t HostFB_VSyncTimer(void);
/* Installs the dispatcher the host calls for a VBlank edge that does not come
 * from the device model (set by PE_Callback_Init). */
void HostFB_SetVBlankDispatch(int (*dispatch)(void));
/* Advance the modeled CD device clock by the quantum a guest poll-loop
 * iteration stands for, then deliver its IRQs.  A guest stream wait of about
 * 2000 poll iterations represents far more real CPU time than one 1024-cycle
 * counter query, so a stream wait uses this larger quantum; otherwise the
 * modeled 2x drive delivers only ~3 sectors inside the guest's own retry
 * budget and the player restarts mid-frame from sector 0.
 * Read-only on guest state beyond the device/IRQ model. */
void HostFB_StreamTick(void);
/* One CD-status poll: device time with the device attached (pe_cdreg). */
void HostFB_DevicePoll(void);
/* Quiet device window: the CD device model advances (drive events and interrupts are delivered) while the
 * emulated clocks, VBlank edges and the VSync call/scanline counters stay where they were; End restores the
 * counters.  For the first disc mount, which happens before the game has any timeline. */
void HostFB_DeviceQuietBegin(void);
void HostFB_DeviceQuietEnd(void);
void HostFB_DrawSync(int mode);
int  HostFB_WritePPM(const char *path);
const uint8_t *HostFB_GetPixels(void);
/* Window presentation only; screenshots/capture must retain GetPixels().
 * Returns native 320x240 when scale is 1 or the second scan-out is unavailable. */
const uint8_t *HostFB_GetPresentationPixels(int *width, int *height);
/* Box-downsample the shadow to a separate native-sized presentation frame.
 * Effective only at render scales 2..4; GetPixels() always remains native. */
void HostFB_SetSSAA(int enabled);
int HostFB_GetSSAA(void);
/* Explicit diagnostic export of the second scan-out, with no capture hooks. */
int HostFB_WritePresentationPPM(const char *path);
void HostFB_GetState(int *vsync, int *drawsync, int *presented, int *mask);
/* Full presentation count; the legacy int view saturates at INT_MAX. */
uint64_t HostFB_GetPresentCount(void);
/* Display-only refresh after a VBlank scan-out (not a present). */
void HostFB_SetScanoutHook(void (*hook)(void));
/* Harness aid: screenshots (HostFB_WritePPM) pass the pixels through this
 * filter first (NULL = raw framebuffer).  port_main installs the dev overlay
 * compositor under PE_SHOT_OVERLAY=1 so headless captures include the menu. */
void HostFB_SetShotFilter(const uint8_t *(*filter)(const uint8_t *rgb, int w, int h));
/* Number of idle-VBlank scan-outs so far (diagnostics, PE_FRAME_TRACE). */
uint32_t HostFB_ScanOutCount(void);

#endif
