/* Vector check for batch cdgen49 (pe_cdx_cdgen49_vectors.py): the live functions of platform/pe_libcd.c, linked as a
 * built OBJECT (never read; names from `nm`).  Two entry groups, one source, three builds:
 *
 *   readissue      retail 0x8006E6D4.  Native symbol: func_8006E6D4 (the generator's own TU only; since cdx-dsread the hand object pe_libcd.c.o no longer defines it, so the hand build skips this group), or the generator's own
 *                  translation (pc_port/game/decomp/func_8006E6D4_port.c as emitted by `gen_decomp_ports.py --e21` with the
 *                  pe_libcd.c definition masked; -DCDX49_GEN).
 *   timeout slices retail 0x8007B0DC (poll arm) and 0x8007B7B8 (issue arm): native symbol PE_Cd_TimeoutRecover.
 *
 * Stand-ins (the retail callee each one stands for; every choice is a modelling choice, documented in the REPORT):
 *   pe_plat_ds_system_status 8007F72C(0)   pe_plat_cd_queue_len 8007F778(0)   pe_plat_cd_disc_type 8007F7A8(0)
 *   pe_plat_cd_int_to_pos (alias lowest 8007A930 of 80080B44)(2)   pe_plat_cd_flush 8007B9EC(0)
 *   PE_Printf_Guest -> 80071A74 (arity per entry: 3 for readissue, 5 for the timeout arm), except a call whose format
 *                      word is 0x80011B18 -> puts 80073C5C(1)
 *   PE_Port_RequestStop / pe_plat_bios_exit -> 800719E4(1), TERMINAL (case ends)
 *   DsRead 80080E34(loc, sectors, dest, 0x80): generated build: PE_Decomp_BoundaryFrom; hand build: the host disc seam
 *                      PE_Disc_ReadUserData stands for it (arguments mapped: arg0 = a pseudo frame pointer (the hand object has no
 *                      loc), sectors = len / 2048, dest = guest address of the buffer; result nonzero <=> scripted DsRead result
 *                      nonzero).  The stand-in writes NO data (retail fills the destination asynchronously by DMA).
 *   host-only (counted, not compared): HostFB_GetState, PE_GPU_DMA2Pending, PE_Port_ServiceDmaIrqCheckpoint, pe_plat_ds_text_5f4
 *   PE_Disc_GetActive returns a non-NULL dummy disc; PE_Disc_FindFile fails the case when reached; PE_GuestCall records the callee.
 * Env: CDX49_LOG=1 prints every stand-in call (per case, to stderr).
 *
 * BUILD-hand: cc -std=gnu11 -Wall -Wextra -Werror -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_cdx_cdgen49_check.c build/vec/CMakeFiles/pe_field_runtime.dir/platform/pe_libcd.c.o -o build/chk/check49_hand
 * BUILD-gen: cc -std=gnu11 -Wall -Wextra -Werror -DCDX49_GEN -Ipc_port -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_cdx_cdgen49_check.c $CDX49_GENOBJ -o build/chk/check49_gen
 *   (CDX49_GENOBJ = the scratch object compiled from the generator's output, see the REPORT)
 * Run: ./check49_hand SET.txt [noop|flip|retflip|storeflip]
 */
#include "pe_cdx_common.h"
#include "game_port.h"
#include <stdbool.h>

static unsigned printf_arity = 3;
static int logging;
static void lg(const char *what, uint32_t a, uint32_t b, uint32_t c, uint32_t d)
{
    if (logging) fprintf(stderr, "    stand-in %s %X %X %X %X\n", what, a, b, c, d);
}

#ifndef CDX49_GEN
CALLEE(pe_plat_cd_flush, 0x8007B9ECu, 0)
#endif
CALLEE(pe_plat_ds_system_status, 0x8007F72Cu, 0)
CALLEE(pe_plat_cd_queue_len, 0x8007F778u, 0)
CALLEE(pe_plat_cd_disc_type, 0x8007F7A8u, 0)
uint32_t pe_plat_cd_int_to_pos(int32_t lba, uint32_t loc) { return cdx_callee(0x8007A930u, 2, (uint32_t)lba, loc, 0, 0, 0); }

int32_t PE_Printf_Guest(pe_addr_t fmt, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5, uint32_t a6, uint32_t a7)
{
    (void)a5; (void)a6; (void)a7;
    lg("PE_Printf_Guest", fmt, a1, a2, a3);
    if (fmt == 0x80011B18u) return (int32_t)cdx_callee(0x80073C5Cu, 1, fmt, 0, 0, 0, 0);
    {   uint32_t a[8] = {0}; a[0] = fmt; a[1] = a1; a[2] = a2; a[3] = a3; a[4] = a4;
        return (int32_t)cdx_calleeN(0x80071A74u, printf_arity, a); }
}

static void terminate(void) { case_terminated = 1; if (escape_armed) longjmp(case_escape, 1); }
static int stopped;
int PE_Port_ShouldStop(void) { return stopped; }
void PE_Port_RequestStop(PEPortStopReason reason)
{
    (void)reason; lg("PE_Port_RequestStop", (uint32_t)reason, 0, 0, 0);
    stopped = 1;
    cdx_callee(0x800719E4u, 1, 1, 0, 0, 0, 0);
    terminate();
}
uint32_t pe_plat_bios_exit(uint32_t mode) { cdx_callee(0x800719E4u, 1, mode, 0, 0, 0, 0); stopped = 1; terminate(); return 0; }

/* DsRead */
#ifdef CDX49_GEN
/* since cdgen49c DsRead is a bound clean-room provider (pe_plat_cdxdsread_read, retail 0x80080E34), no longer a loud boundary:
 * the generated-TU build stands it in as a stopped callee (checked separately by pe_cdx_cdgen49b_check.c). */
int pe_plat_cdxdsread_read(uint32_t pos, uint32_t sectors, uint32_t dest, uint32_t mode)
{ return (int)cdx_callee(0x80080E34u, 4, pos, sectors, dest, mode, 0); }
#endif
int PE_Decomp_BoundaryFrom(const char *caller, const char *symbol, unsigned vma, unsigned arity, uintptr_t a0, uintptr_t a1, uintptr_t a2, uintptr_t a3)
{
    (void)caller; (void)symbol;
    return (int)cdx_callee(vma, arity, (uint32_t)a0, (uint32_t)a1, (uint32_t)a2, (uint32_t)a3, 0);
}
static char dummy_disc[16];
void *PE_Disc_GetActive(void) { lg("PE_Disc_GetActive", 0, 0, 0, 0); return dummy_disc; }
bool PE_Disc_ReadUserData(const void *d, uint32_t lba, uint32_t offset, void *out, uint32_t len)
{
    uint32_t dest = (uint32_t)(0x80000000u | (uint32_t)((uint8_t *)out - ram));
    uint32_t r;
    (void)d;
    lg("PE_Disc_ReadUserData", lba, offset, dest, len);
    r = cdx_callee(0x80080E34u, 4, PE_GUEST_FRAME_HI - 0x20u, len / 2048u, dest, 0x80u, 0);
    return r != 0;
}
bool PE_Disc_FindFile(const void *d, const char *path, uint32_t *a, uint32_t *b, char *n, size_t ns)
{ (void)d; (void)path; (void)a; (void)b; (void)n; (void)ns; bad_call = 1; trap_name = "PE_Disc_FindFile"; return false; }
void HostFB_GetState(int *vsync, int *drawsync, int *presented, int *mask)
{ host_only_calls++; if (vsync) *vsync = 0; if (drawsync) *drawsync = 0; if (presented) *presented = 0; if (mask) *mask = 0; }
int PE_GPU_DMA2Pending(void) { host_only_calls++; lg("PE_GPU_DMA2Pending", 0, 0, 0, 0); return 0; }
PEPortDmaIrqCheckpointResult PE_Port_ServiceDmaIrqCheckpoint(void) { host_only_calls++; lg("PE_Port_ServiceDmaIrqCheckpoint", 0, 0, 0, 0); return PE_PORT_DMA_IRQ_CHECKPOINT_IDLE; }
void pe_plat_ds_text_5f4(uint32_t s, uint32_t st, uint32_t r) { host_only_calls++; lg("pe_plat_ds_text_5f4", s, st, r, 0); }
int PE_GuestCall(const char *site, pe_addr_t fn, unsigned arity, uintptr_t a, uintptr_t b, uintptr_t c, uintptr_t d)
{ if (logging) fprintf(stderr, "    stand-in PE_GuestCall site=\"%s\" fn=%X arity=%u\n", site ? site : "(null)", fn, arity); lg("PE_GuestCall", fn, a, b, c); return (int)cdx_callee(fn, arity, (uint32_t)a, (uint32_t)b, (uint32_t)c, (uint32_t)d, 0); }
/* per the doc comment of pe_guest_ram.h: main RAM is the KSEG0 window [PE_RAM_BASE, PE_RAM_END); KUSEG/KSEG1 aliases are canonicalised separately (PE_RamCanonical) and are not 'RAM' here */
bool PE_RangeIsRam(pe_addr_t a, size_t n) { return a >= PE_RAM_BASE && (uint64_t)a + n <= (uint64_t)PE_RAM_END; }
void PE_Fill(pe_addr_t a, uint32_t len, uint8_t v) { uint32_t i; for (i = 0; i < len; i++) PE_StoreU8(a + i, v); }

/* weak: the hand definition was removed from pe_libcd.c (cdx-dsread); only the -DCDX49_GEN build links the generated TU */
int func_8006E6D4(int, int, pe_addr_t, int) __attribute__((weak));
/* the former host timeout seam no longer exists in the tree; the timeout slice is a local no-op */
__attribute__((unused)) static void PE_Cd_TimeoutRecover(void) {}
static int call(uint32_t entry, const uint32_t *a, uint32_t *ret)
{
    stopped = 0;
    logging = getenv("CDX49_LOG") != NULL;
    if (entry == 0x8006E6D4u) {
        printf_arity = 3;
        if (!func_8006E6D4) return -1;
        if (mutant == 1) { *ret = 0; return 1; }
        *ret = (uint32_t)func_8006E6D4((int)a[0], (int)a[1], a[2], (int)a[3]);
        return 1;
    }
    if (entry == 0x8007B0DCu || entry == 0x8007B7B8u) {
#ifdef CDX49_GEN
        return -1;
#else
        printf_arity = 5;
        if (mutant == 1) return 0;
        PE_Cd_TimeoutRecover();
        return 0;
#endif
    }
    return -1;
}
