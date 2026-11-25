/* Standalone guest-RAM replay checker for the bounded/resumable C89C adapter (pe_plat_vlc_frame.c + pe_vlc_decode.c).
 *   cc -std=gnu99 -Wall -Wextra -Werror -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap -Ipc_port/tests pc_port/tools/pe_c89c_replay_check.c pc_port/platform/pe_plat_vlc_frame.c pc_port/platform/pe_vlc_decode.c pc_port/platform/pe_guest_ram.c -Wl,--unresolved-symbols=ignore-all -o check
 *   PE_DISC1_BIN=<Disc 1 .bin> ./check
 * Replays pc_port/tests/retail_mdec_vlc_bound_cases.h (original's bounded/resume chains) and retail_c89c_real_sequence.h
 * (real playback sequence) through the adapter in guest RAM.  Exit 0 = every step matches except the listed known ones. */
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "pe_guest_ram.h"
#include "pe_plat/vlc_frame.h"
#include "pe_vlc_decode.h"
#pragma GCC diagnostic ignored "-Wunused-const-variable"
#include "retail_mdec_vlc_cases.h"
#include "retail_mdec_vlc_bound_cases.h"
#include "retail_c89c_real_sequence.h"
#include "game_port.h"
#include "pe_bootstrap.h"

static uint64_t MdecVlcFnv(const uint8_t *p, size_t n)
{
    uint64_t h = UINT64_C(14695981039346656037);
    while (n--) { h ^= *p++; h *= UINT64_C(1099511628211); }
    return h;
}
static int g_stop;
void PE_Port_RequestStop(PEPortStopReason r) { (void)r; g_stop = 1; }
int PE_Port_ShouldStop(void) { return g_stop; }
void Bootstrap_ReturnVoid1(const char *symbol, const char *caller, uint32_t arg0) { (void)caller; if (getenv("C89C_REPLAY_VERBOSE")) fprintf(stderr, "[named stop] %s %08X\n", symbol, (unsigned)arg0); }
#define C89CR_RESET() do { PE_RamReset(); g_stop = 0; } while (0)
#define C89CR_STOPPED() (g_stop)
#include "c89c_replay_core.h"

static int ReadRaw(void *ctx, uint32_t lba, uint8_t raw[2352])
{
    FILE *f = ctx;
    if (fseek(f, (long)lba * 2352L, SEEK_SET)) return 0;
    return fread(raw, 1, 2352, f) == 2352;
}

int main(void)
{
    const char *disc = getenv("PE_DISC1_BIN");
    char err[300] = "";
    unsigned ns, nk, nc, nr;
    FILE *f;
    if (!disc || !(f = fopen(disc, "rb"))) { fprintf(stderr, "set PE_DISC1_BIN to the Disc 1 .bin\n"); return 2; }
    PE_RamInit();
    if (!C89CReplayBound(ReadRaw, f, err, sizeof(err), &ns, &nk, &nc)) { printf("MISMATCH (bound chains): %s\n", err); return 1; }
    printf("bound/resume chains: %u cases, %u steps exact, %u known-divergent steps\n", nc, ns, nk);
    if (!C89CReplayReal(ReadRaw, f, err, sizeof(err), &nr)) { printf("MISMATCH (real sequence): %s\n", err); return 1; }
    printf("real playback sequence: %u/%u calls exact (ret, end word, bound word, header, arena fit)\n", nr, (unsigned)(sizeof(C89CREAL_seq) / sizeof(C89CREAL_seq[0])));
    printf("PASS\n");
    return 0;
}
