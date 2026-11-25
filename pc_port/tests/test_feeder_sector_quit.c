/* Isolated synthetic feeder quit check. Link with game_port.c and section GC;
 * pass "quit" to request quit after the first synthetic video sector, or
 * "end" to request it on the last synthetic non-video sector, or
 * "account-wrap" to exercise presentation accounting across the signed boundary.
 * No disc, real media, owner data or device is accessed. This is not registered
 * in the held production build configuration.
 */
#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "game_port.h"
#include "../platform/pe_str_feed.c"

static int inject, quit_flag, reads, closed;
static uint64_t presents;
static uint32_t vsyncs;
static uint8_t ram[0x200000];
static const uint32_t base = 0x80130000u;

static int quit_poll(void) { return quit_flag; }
PE_Disc *PE_Disc_GetActive(void) { return (PE_Disc *)1; }
uint32_t PE_Disc_UserSectorCount(const PE_Disc *d) { (void)d; return 3; }
bool PE_Disc_ReadRawSector(const PE_Disc *d, uint32_t lba,
                          uint8_t out[PE_DISC_RAW_SECTOR])
{
    (void)d;
    memset(out, 0, PE_DISC_RAW_SECTOR);
    if (inject != 2) {
        out[18] = 8;
        out[24] = 0x60; out[25] = 1; out[26] = 1; out[27] = 0x80;
        out[28] = (uint8_t)lba; out[30] = 2;
    }
    reads++;
    if ((inject == 1 && reads == 1) || (inject == 2 && reads == 3))
        quit_flag = 1;
    return true;
}
void PE_GPU_GetState(PeGpuState *out)
{ memset(out, 0, sizeof(*out)); out->vsync_count = vsyncs; }
void HostFB_GetState(int *a, int *b, int *c, int *d)
{ (void)a; (void)b; (void)d; if (c) *c = presents > INT_MAX ? INT_MAX : (int)presents; }
uint64_t HostFB_GetPresentCount(void) { return presents; }
void HostFB_Present(void) { presents++; }
uint32_t HostFB_VSync(int v) { (void)v; vsyncs++; return vsyncs; }
void PE_Spu_CdInputStop(void) { closed++; }
void PE_CdReg_GetAudioVolumes(uint8_t a[4]) { (void)a; abort(); }
void PE_Spu_CdSector(const uint8_t *p, uint8_t m, int z, uint8_t t,
                     uint8_t c, const uint8_t a[4])
{ (void)p; (void)m; (void)z; (void)t; (void)c; (void)a; abort(); }
int PE_CdReg_XaMuted(void) { abort(); }
uint32_t PE_LoadU32(pe_addr_t a)
{ if (a == GA_RING_BASE) return base; if (a == GA_RING_COUNT) return 4; abort(); }
uint16_t PE_LoadU16(pe_addr_t a)
{
    unsigned n = a - 0x80000000u;
    assert(n + 1 < sizeof ram);
    return ram[n] | ((uint16_t)ram[n + 1] << 8);
}
void PE_StoreU16(pe_addr_t a, uint16_t v)
{
    unsigned n = a - 0x80000000u;
    assert(n + 1 < sizeof ram);
    ram[n] = v; ram[n + 1] = v >> 8;
}
void *PE_Translate(pe_addr_t a, size_t n)
{
    unsigned i = a - 0x80000000u;
    assert(i + n <= sizeof ram);
    return ram + i;
}
int HostFB_WritePPM(const char *p) { (void)p; abort(); }

int main(int argc, char **argv)
{
    if (argc > 2) abort();
    if (argc == 2 && strcmp(argv[1], "account-wrap") == 0) {
        g_feed.last_presented = INT_MAX;
        presents = (uint64_t)INT_MAX + 1u;
        vsyncs = 1;
        Account();
        if (g_feed.last_presented != (uint64_t)INT_MAX + 1u || g_feed.credits != 5 ||
            g_feed.stats.vblanks != 1 || presents != (uint64_t)INT_MAX + 1u)
            abort();
        puts("account-wrap: one prior presentation, no extra present");
        return 0;
    }
    inject = argc == 1 ? 0 : strcmp(argv[1], "quit") == 0 ? 1 :
             strcmp(argv[1], "end") == 0 ? 2 : -1;
    if (inject < 0) abort();
    memset(&g_feed, 0, sizeof g_feed);
    g_feed.active = 1;
    g_feed.credits = inject == 2 ? 8 : 6;
    PE_Port_SetQuitPoll(quit_poll);
    PE_StrFeed_Pump();
    printf("reads=%d sectors=%u lba=%u quit=%d stop=%d assembling=%d "
           "pending=%d published=%u ready=%u\n", reads, g_feed.stats.sectors,
           g_feed.lba, quit_flag, g_port_stop_requested, g_feed.assembling,
           g_feed.pending, g_feed.stats.frames_published, PE_LoadU16(base));
    if (inject == 2) {
        if (reads != 3 || g_feed.stats.sectors != 3 || g_feed.lba != 3 ||
            !quit_flag || g_port_stop_requested || g_feed.active || closed != 1)
            abort();
        return 0;
    }
    if (reads != (inject ? 1 : 2) ||
        g_feed.stats.sectors != (unsigned)(inject ? 1 : 2) ||
        g_feed.lba != (unsigned)(inject ? 1 : 2) ||
        !!g_port_stop_requested != inject || g_feed.assembling != inject ||
        g_feed.pending || g_feed.stats.frames_published != (unsigned)!inject ||
        PE_LoadU16(base) != (inject ? 0 : 2))
        abort();
    return 0;
}
