/* Isolated framebuffer counter check; not registered in the held build.
 * Link with section GC. The GPU, RAM, notification and hooks are doubles. */
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../platform/host_framebuffer.c"

static int observed, calls, allowed = 1;
int PE_Port_FramePresentationAllowed(void) { return allowed; }
void PE_Port_FramePresented(int n) { observed = n; calls++; }
void PE_Port_InvokePresentHook(void) { }
uint8_t PE_LoadU8(pe_addr_t a) { (void)a; return 0; }
uint16_t PE_LoadU16(pe_addr_t a) { (void)a; return 0; }
int PE_GPU_GetRenderScale(void) { return 1; }
uint64_t PE_GPU_GetRenderGeneration(void) { return 0; }
int PE_GPU_ReadRenderPixel(uint32_t x, uint32_t y, uint16_t *p)
{ (void)x; (void)y; (void)p; return 0; }
int PE_GPU_ReadVRAM(uint32_t x, uint32_t y, uint16_t *p)
{ (void)x; (void)y; (void)p; return 0; }
void HostVRAM_DecodePixel(uint16_t p, uint8_t rgb[3])
{ (void)p; memset(rgb, 0, 3); }

int main(int argc, char **argv)
{
    if (argc != 3) abort();
    int path = atoi(argv[1]), mode = atoi(argv[2]);
    if (path < 0 || path > 1 || mode < 0 || mode > 3) abort();
    fb_presented = mode == 0 ? (uint64_t)INT_MAX - 1 :
                   mode == 1 ? (uint64_t)INT_MAX : UINT64_MAX;
    if (mode == 3) { fb_presented = INT_MAX; allowed = 0; }
    if (path == 0) HostFB_Present(); else HostFB_PresentDispEnv(0);
    uint64_t expected = mode == 0 ? (uint64_t)INT_MAX :
                        mode == 1 ? (uint64_t)INT_MAX + 1 :
                        mode == 2 ? UINT64_MAX : (uint64_t)INT_MAX;
    int legacy = -1;
    HostFB_GetState(NULL, NULL, &legacy, NULL);
    if (HostFB_GetPresentCount() != expected || legacy != INT_MAX ||
        calls != (mode == 3 ? 0 : 1) || (calls && observed != INT_MAX)) abort();
    printf("path=%d mode=%d count=%llu legacy=%d calls=%d\n", path, mode,
           (unsigned long long)expected, legacy, calls);
    return 0;
}
