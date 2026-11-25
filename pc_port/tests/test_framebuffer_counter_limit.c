/* Isolated framebuffer/frame-limit integration check. Link with game_port.c
 * and section GC; the feeder reset is a double. Not registered in the held
 * production build configuration. No window, device, or game is started. */
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "../platform/host_framebuffer.c"

void PE_StrFeed_Reset(void) { }

static void expect_present(uint64_t start, int limit, uint64_t expected,
                           int stopped)
{
    PE_Port_RunControlReset();
    PE_Port_SetFrameLimit(limit);
    fb_presented = start;
    HostFB_Present();
    if (HostFB_GetPresentCount() != expected || !!PE_Port_ShouldStop() != stopped)
        abort();
    if (stopped && PE_Port_GetStopReason() != PE_PORT_STOP_FRAME_LIMIT)
        abort();
}

int main(void)
{
    expect_present(0, 1, 1, 1);
    expect_present((uint64_t)INT_MAX - 1u, INT_MAX, INT_MAX, 1);
    expect_present(INT_MAX, 0, (uint64_t)INT_MAX + 1u, 0);
    expect_present((uint64_t)INT_MAX + 1u, 1, (uint64_t)INT_MAX + 2u, 1);
    puts("frame limits: ordinary, maximum, unlimited and late limit pass");
    return 0;
}
