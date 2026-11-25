/* Isolated present-keyed pad-script counter check. Compile with section GC;
 * the framebuffer counter is a double. Not registered in the held build. */
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#define main unused_port_main
#include "../src/port_main.c"
#undef main

static uint64_t fixture_presented;
uint64_t HostFB_GetPresentCount(void) { return fixture_presented; }
uint32_t PE_GPU_VSyncQuery(void) { abort(); }

int main(void)
{
    unsetenv("PE_PAD_SCRIPT_CLOCK");
    g_script_pad_count = 2;
    g_script_pad[0].frame = 1;
    g_script_pad[0].mask = 0x1234;
    g_script_pad[1].frame = 3;
    g_script_pad[1].mask = 0x5678;
    fixture_presented = 0;
    if (ScriptPadSource() != 0xffff) abort();
    fixture_presented = 1;
    if (ScriptPadSource() != 0x1234) abort();
    fixture_presented = 3;
    if (ScriptPadSource() != 0x5678) abort();
    fixture_presented = INT_MAX;
    uint16_t before = ScriptPadSource();
    fixture_presented = (uint64_t)INT_MAX + 1u;
    uint16_t after = ScriptPadSource();
    if (before != 0x5678 || after != 0x5678) abort();
    printf("present script: before=%04x after=%04x\n", before, after);
    return 0;
}
