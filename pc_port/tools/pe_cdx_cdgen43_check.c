/* Vector check for batch cdgen43 (pe_cdx_irq_vectors.py): libetc veneers / callback helpers and the
 * retail interrupt dispatchers.  Two builds from this one source (run from the repo root; objects
 * from `ninja -C build/vec CMakeFiles/pe_field_runtime.dir/platform/pe_libetc.c.o
 * CMakeFiles/pe_field_runtime.dir/platform/pe_plat_irq_callback.c.o`):
 * BUILD-LIBETC: cc -std=gnu11 -Wall -Wextra -Werror -DCDX43_LIBETC -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_cdx_cdgen43_check.c build/vec/CMakeFiles/pe_field_runtime.dir/platform/pe_libetc.c.o -o check43a
 * BUILD-IRQ: cc -std=gnu11 -Wall -Wextra -Werror -DCDX43_IRQ -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_cdx_cdgen43_check.c build/vec/CMakeFiles/pe_field_runtime.dir/platform/pe_plat_irq_callback.c.o -o check43b
 * BUILD-VSYNC: cc -std=gnu11 -Wall -Wextra -Werror -DCDX43_VSYNC -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_cdx_cdgen43_check.c build/vec/CMakeFiles/pe_field_runtime.dir/platform/pe_libetc.c.o build/vec/CMakeFiles/pe_field_runtime.dir/platform/pe_plat_irq_callback.c.o -o check43c
 * BUILD-ETC-LIBETC: cc -std=gnu11 -Wall -Wextra -Werror -DCDX43_LIBETC -DCDX43_ETC -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_cdx_cdgen43_check.c build/vec/CMakeFiles/pe_field_runtime.dir/platform/pe_plat2_cdx_etc.c.o -o check43a
 * BUILD-ETC-VSYNC: cc -std=gnu11 -Wall -Wextra -Werror -DCDX43_VSYNC -DCDX43_ETC -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_cdx_cdgen43_check.c build/vec/CMakeFiles/pe_field_runtime.dir/platform/pe_plat2_cdx_etc.c.o build/vec/CMakeFiles/pe_field_runtime.dir/platform/pe_plat_irq_callback.c.o -o check43c
 *   (the legacy BUILD-LIBETC / BUILD-VSYNC lines above link the translated pe_libetc.c.o, which no longer carries the nine entries
 *    under test and now references pe_plat_cdxetc_reset_state; pe_cdx_run_all.sh uses the ETC lines.  -DCDX43_ETC: the libetc/VSync entries under test are the clean-room cdx-etc symbols pe_plat_cdxetc_*, pe_plat2_cdx_etc.c;
 *    PE_Sdk_ResetState is stood in by the checker and resets that model; the old pe_plat_bios_* host seams the veneers forward
 *    to (and the IRQ provider calls) are inert stand-ins.)
 * Run: ./check43a vectors.txt build/disc1.candidate.exe [noop|flip]   (vectors: the libetc families; ./check43b: dispatcher families)
 * Contract: ordered event trace per case: register stores (MMIO), guest callback invocations (CB, by target),
 * returns (RET), dispatch-table stores (STORE, in order and width) and the final value of every RAM word the
 * original wrote (RAM); in the IRQ build also the BIOS service calls the retail code makes (BIOS ...).  In the
 * LIBETC build the BIOS veneers are the code under test: the native models the BIOS host-side, so the BIOS
 * event lines are not observable and are dropped; RET carries their effect.  Interpreter-only setup (RESET /
 * INTR / DMACB / VSCBS used to reach a state) runs through the IRQ build's providers only.
 * Controls: argv[3] = noop (entry replaced by a stub returning 0) / flip (native result corrupted by one bit):
 * both must FAIL.  Host stand-ins: guest RAM, the register bus (I_STAT write acknowledges; DICR flag rules after
 * psx-spx), PE_IRQ_ExchangeMask / PE_Callback_SetSlot expressed as register / table accesses. */
#ifdef CDX43_ETC
#define PE_CDXETC_HOST_SEAMS 1   /* the checker stands the raw host seams in; the veneers under test are the cdxetc symbols */
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "pe_guest_ram.h"
#if defined(CDX43_IRQ) || defined(CDX43_VSYNC)
#include "pe_mmio.h"
#include "pe_plat/bios_sys.h"
#include "pe_plat/irq_callback.h"
#include "pe_bios_string.h"
#endif
#if !defined(CDX43_IRQ)
uint32_t pe_plat_bios_change_clear_rcnt(uint32_t, uint32_t); void pe_plat_bios_change_clear_pad(uint32_t);
int pe_plat_bios_enter_critical(void); void pe_plat_bios_exit_critical(void); void pe_plat_bios_flush_cache(void);
uint16_t pe_plat_irq_set_mask(uint16_t); uint32_t pe_plat_vsync_callback(uint32_t); uint32_t pe_plat_vsync_callbacks(uint32_t, uint32_t);
#endif

#ifdef CDX43_ETC
#include "pe_plat/cdx_etc.h"
/* The old host bookkeeping seams (called by the clean-room veneers for IRQ-gate / diagnostics state, and by the IRQ provider)
 * are inert stand-ins here: the veneers' BIOS events are dropped in these builds, the results are ignored. */
void pe_plat_bios_flush_cache(void) {}
void pe_plat_bios_change_clear_pad(uint32_t flag) { (void)flag; }
uint32_t pe_plat_bios_change_clear_rcnt(uint32_t t, uint32_t flag) { (void)t; (void)flag; return 0; }
int pe_plat_bios_enter_critical(void) { return 0; }
void pe_plat_bios_exit_critical(void) {}
#define pe_plat_bios_flush_cache pe_plat_cdxetc_flush_cache
#define pe_plat_bios_change_clear_pad pe_plat_cdxetc_change_clear_pad
#define pe_plat_bios_change_clear_rcnt pe_plat_cdxetc_change_clear_rcnt
#define pe_plat_bios_enter_critical pe_plat_cdxetc_enter_critical
#define pe_plat_bios_exit_critical pe_plat_cdxetc_exit_critical
#define pe_plat_irq_set_mask pe_plat_cdxetc_set_intr_mask
#define pe_plat_vsync_callback pe_plat_cdxetc_vsync_callback
#define pe_plat_vsync_callbacks pe_plat_cdxetc_vsync_callbacks
#endif
static uint8_t ram[0x200000], init_ram[0x200000], touched[0x200000];
static char got[40000][64]; static unsigned ngot;
static uint32_t reg_istat, reg_imask, reg_dpcr, reg_dicr, reg_t[16];
static uint32_t rcnt_flag[4];

static uint32_t off(pe_addr_t a, unsigned n)
{
    if ((a & 0x1FFFFFFFu) >= 0x200000u || (a & 0x1FFFFFu) + n > 0x200000u) {
        fprintf(stderr, "guest access outside main RAM: %08X\n", a); exit(2);
    }
    return a & 0x1FFFFFu;
}
static void ev(const char *s) { if (ngot < 40000) snprintf(got[ngot++], 64, "%s", s); }
uint8_t  PE_LoadU8 (pe_addr_t a) { return ram[off(a, 1)]; }
uint16_t PE_LoadU16(pe_addr_t a) { uint16_t v; memcpy(&v, ram + off(a, 2), 2); return v; }
uint32_t PE_LoadU32(pe_addr_t a) { uint32_t v; memcpy(&v, ram + off(a, 4), 4); return v; }
static void mark(uint32_t o, unsigned n, uint32_t v)
{
    unsigned k = n;
    while (k--) touched[o + k] = 1;
    /* Dispatch-table stores are compared in order with their width. */
    if ((o >= 0x945E4u && o < 0x9461Cu) || (o >= 0x9568Cu && o < 0x956E0u)) {
        char s[64]; snprintf(s, 64, "STORE %08X %u %08X", 0x80000000u | o, n, v); ev(s);
    }
}
void PE_StoreU8 (pe_addr_t a, uint8_t v)  { uint32_t o = off(a, 1); ram[o] = v; mark(o, 1, v); }
void PE_StoreU16(pe_addr_t a, uint16_t v) { uint32_t o = off(a, 2); memcpy(ram + o, &v, 2); mark(o, 2, v); }
void PE_StoreU32(pe_addr_t a, uint32_t v) { uint32_t o = off(a, 4); memcpy(ram + o, &v, 4); mark(o, 4, v); }

#if defined(CDX43_IRQ) || defined(CDX43_VSYNC)
static uint32_t *reg(pe_addr_t a)
{
    switch (a) {
    case 0x1F801070u: return &reg_istat;
    case 0x1F801074u: return &reg_imask;
    case 0x1F8010F0u: return &reg_dpcr;
    case 0x1F8010F4u: return &reg_dicr;
    }
    if (a >= 0x1F801100u && a < 0x1F801130u && !(a & 3)) return &reg_t[(a - 0x1F801100u) >> 2];
    fprintf(stderr, "register %08X not in this harness\n", a); exit(2);
}
void PE_MMIO_Store(pe_addr_t a, size_t size, uint32_t v)
{
    char s[64]; snprintf(s, 64, "MMIO %08X %u %08X", a, (unsigned)size, v); ev(s);
    if (a == 0x1F801070u) reg_istat &= v & 0xFFFFu;
    else if (a == 0x1F8010F4u) {
        /* psx-spx DICR: 0-23 read/write, flags 24-30 write-1-to-reset,
         * bit 31 = bit15 | (bit23 & any(enable16-22 & flag24-30)). */
        uint32_t flags = reg_dicr & 0x7F000000u & ~v, low = v & 0x00FFFFFFu;
        int on = (low >> 15 & 1u) || ((low >> 23 & 1u) && ((low >> 16) & (flags >> 24) & 0x7Fu));
        reg_dicr = low | flags | (on ? 0x80000000u : 0u);
    } else *reg(a) = v;
}
uint32_t PE_MMIO_Load(pe_addr_t a, size_t size) { (void)size; return *reg(a); }

#ifdef CDX43_IRQ
void pe_plat_bios_change_clear_pad(uint32_t flag)
{ char s[64]; snprintf(s, 64, "BIOS CLEARPAD %08X", flag); ev(s); }
uint32_t pe_plat_bios_change_clear_rcnt(uint32_t t, uint32_t flag)
{
    char s[64]; uint32_t old = rcnt_flag[t & 3];
    snprintf(s, 64, "BIOS CLEARRCNT %08X %08X", t, flag); ev(s); rcnt_flag[t & 3] = flag; return old;
}
int pe_plat_bios_enter_critical(void) { ev("BIOS ENTER"); return 0; }
#endif
/* Host services the handlers use: guest callback invocation and BIOS printf. */
int pe_plat_invoke_guest_callback(uint32_t fn)
{
    char s[64];
    /* The host runs the two default interrupt handlers as their clean-room
     * functions (the original runs their code, so no CB stub is reached). */
    if (fn == 0x8007440Cu) { (void)pe_plat_vsync_handler(); return 1; }
    if (fn == 0x80074520u) { (void)pe_plat_dma_handler(); return 1; }
    snprintf(s, 64, "CB %08X", fn); ev(s);
    /* Same stubs as the vector tool: these callbacks latch more DMA flags. */
    if (fn == 0x80130800u) reg_dicr |= 0x10000000u;
    if (fn == 0x80130840u) reg_dicr |= 0x04000000u;
    if (fn == 0x80130880u) reg_dicr &= ~0x00100000u;
    if (fn == 0x801308C0u) reg_istat |= 0x0004u;   /* raises CPU source 2 */
    if (fn == 0x80130900u) reg_istat |= 0x0001u;   /* raises CPU source 0 */
    return 1;
}
/* DMA MADR of a channel, as seeded by the vector tool. */
uint32_t pe_plat_read_dma_madr(uint32_t ch) { return 0x00010000u * (ch + 1u) + 0x123u; }
int32_t PE_Bios_Printf(pe_addr_t fmt, const uint32_t *args, int nargs)
{
    char s[64];
    snprintf(s, 64, "BIOS PRINTF %08X %08X %08X %08X", fmt, nargs > 0 ? args[0] : 0u,
             nargs > 1 ? args[1] : 0u, nargs > 2 ? args[2] : 0u);
    ev(s); return 0;
}
#ifdef CDX43_IRQ
void pe_plat_bios_exit_critical(void) { ev("BIOS EXIT"); }
#endif

#endif
#if defined(CDX43_LIBETC) || defined(CDX43_VSYNC)
/* libetc's host seams, expressed at the hardware level.  I_MASK exchange: a halfword read then a
 * halfword write of the register; the slot table of PE_Callback_SetSlot is the guest table at 0x8009568C
 * (store only when the handler changes, as the retail table writer does). */
uint16_t PE_IRQ_ExchangeMask(uint16_t m)
{ char s[64]; uint16_t old = (uint16_t)reg_imask; reg_imask = m; snprintf(s, 64, "MMIO 1F801074 2 %08X", (unsigned)m); ev(s); return old; }
uint16_t PE_IRQ_GetMask(void) { return (uint16_t)reg_imask; }
uint32_t PE_Callback_SetSlot(uint32_t slot, pe_addr_t h)
{
    uint32_t a = 0x8009568Cu + slot * 4u, old = PE_LoadU32(a);
    if (old != h) PE_StoreU32(a, h);
    return old;
}
/* reset / diagnostic seams with no retail counterpart: no-ops */
void func_8008CA84(void) {} void PE_AkaoVerify_Tick(void) {} int PE_AudioDriver_Active(void) { return 0; }
void PE_BiosCard_Deliver(uint32_t a, uint32_t b) { (void)a; (void)b; } void PE_CdReg_Reset(void) {} void PE_GPU_Init(void) {}
void PE_IRQ_DeliveryTraceReset(void) {} void PE_IRQ_Reset(void) {} void PE_MDEC_Init(void) {} void PE_MMIO_Reset(void) {}
void PE_Overlay_ResetResidency(void) {} void PE_Rcnt2_Reset(void) {} void PE_Sio0_Reset(void) {} void PE_SpuDma_Reset(void) {}
void PE_Timer1_Reset(void) {}
static int stop_requested;
void PE_Port_RequestStop(int r) { (void)r; stop_requested = 1; } int PE_Port_ShouldStop(void) { return 0; }
void *PE_Translate(pe_addr_t a, size_t n) { return ram + off(a, (unsigned)n); }
uint8_t g_pe_gte[65536];
#endif

#if defined(CDX43_LIBETC) || defined(CDX43_VSYNC)
void PE_Sdk_ResetState(void);
#ifdef CDX43_ETC
void PE_Sdk_ResetState(void) { pe_plat_cdxetc_reset_state(); }   /* the SDK host state under test is the cdx-etc BIOS model */
#endif
#endif
static int mutant;   /* 0 native, 1 noop, 2 flip */
static unsigned ent_cases[32], ent_bad[32]; static char ent_name[32][48]; static unsigned nent, cur_ent, case_bad;
static uint32_t run_kind(const char *kind, uint32_t a, uint32_t b)
{
    uint32_t r = 0; int known = 1;
#ifdef CDX43_IRQ
    r = !strcmp(kind, "RESET") ? pe_plat_reset_callback()
      : !strcmp(kind, "DMACB") ? pe_plat_dma_callback(a, b)
      : !strcmp(kind, "DMAH") ? (mutant == 1 ? 0 : pe_plat_dma_handler())
      : !strcmp(kind, "DISP") ? (mutant == 1 ? 0 : pe_plat_irq_dispatch())
      : !strcmp(kind, "INTR") ? pe_plat_interrupt_callback(a, b) : (known = 0);
#elif defined(CDX43_VSYNC)
    r = !strcmp(kind, "RESET") ? pe_plat_reset_callback()
      : !strcmp(kind, "VSCB") ? (mutant == 1 ? 0 : pe_plat_vsync_callback(a))
#ifdef CDX43_ETC
      : !strcmp(kind, "VSCBS") ? (mutant == 1 ? 0 : pe_plat_vsync_callbacks(a, b))
      : !strcmp(kind, "VSTBL") ? (mutant == 1 ? 0 : pe_plat_cdxetc_vsync_table_write(a, b)) : (known = 0);
#else
      : !strcmp(kind, "VSCBS") || !strcmp(kind, "VSTBL") ? (mutant == 1 ? 0 : pe_plat_vsync_callbacks(a, b)) : (known = 0);
#endif
#else
    if (mutant == 1) return 0;
    r = !strcmp(kind, "FLUSH") ? (pe_plat_bios_flush_cache(), 0u)
      : !strcmp(kind, "CPAD") ? (pe_plat_bios_change_clear_pad(a), 0u)
      : !strcmp(kind, "CRCNT") ? pe_plat_bios_change_clear_rcnt(a, b)
      : !strcmp(kind, "ENTER") ? (uint32_t)pe_plat_bios_enter_critical()
      : !strcmp(kind, "EXIT") ? (pe_plat_bios_exit_critical(), 0u)
      : !strcmp(kind, "SETMASK") ? pe_plat_irq_set_mask((uint16_t)a)
      : !strcmp(kind, "VSCB") ? pe_plat_vsync_callback(a)
#ifdef CDX43_ETC
      : !strcmp(kind, "VSCBS") ? pe_plat_vsync_callbacks(a, b)
      : !strcmp(kind, "VSTBL") ? pe_plat_cdxetc_vsync_table_write(a, b) : (known = 0);
#else
      : !strcmp(kind, "VSCBS") || !strcmp(kind, "VSTBL") ? pe_plat_vsync_callbacks(a, b) : (known = 0);
#endif
#endif
    if (!known) { fprintf(stderr, "unknown kind %s\n", kind); exit(2); }
    return mutant == 2 ? r ^ 1u : r;
}
int main(int argc, char **argv)
{
    FILE *f, *e; char line[256]; static char want[40000][64]; unsigned nwant = 0, cases = 0, bad = 0, k;
    static uint8_t exe[0x200000]; uint32_t t_addr, t_size; int in_case = 0;
    if (argc < 3 || !(f = fopen(argv[1], "r")) || !(e = fopen(argv[2], "rb"))) return 2;
    if (argc > 3) mutant = !strcmp(argv[3], "noop") ? 1 : !strcmp(argv[3], "flip") ? 2 : 0;
    (void)fread(exe, 1, sizeof exe, e); memcpy(&t_addr, exe + 0x18, 4); memcpy(&t_size, exe + 0x1C, 4);
    while (fgets(line, sizeof line, f)) {
        unsigned id, im, is, dirty; uint32_t dicr0; char kind[16]; uint32_t a, b, c;
        line[strcspn(line, "\n")] = 0;
        if (sscanf(line, "CASE %u %x %x %u %x", &id, &im, &is, &dirty, &dicr0) == 5) {
#if defined(CDX43_LIBETC) || defined(CDX43_VSYNC)
            PE_Sdk_ResetState();   /* the native's host state (BIOS model) starts fresh in every case; its own stores are not the case's */
#endif
            memset(ram, 0, sizeof ram); memcpy(ram + (t_addr & 0x1FFFFFu), exe + 0x800, t_size);
            if (dirty) { memset(ram + 0x945E4, 0xA5, 0x41A * 4); ram[0x945E4] = ram[0x945E5] = 0; }
            memcpy(init_ram, ram, sizeof ram); memset(touched, 0, sizeof touched);
            reg_imask = im; reg_istat = is; reg_dpcr = 0; reg_dicr = dicr0; memset(reg_t, 0, sizeof reg_t);
            memset(rcnt_flag, 0, sizeof rcnt_flag); ngot = nwant = 0; in_case = 1; cases++; case_bad = bad;

        } else if (sscanf(line, "POKEW %x %x", &a, &b) == 2) {
            memcpy(ram + off(a, 4), &b, 4);   /* setup, not an event */
        } else if (sscanf(line, "POKEI %x", &a) == 1) {
            reg_istat = a;
        } else if (sscanf(line, "POKEP %x", &a) == 1) {
            reg_dpcr = a;
        } else if (sscanf(line, "POKED %x", &a) == 1) {
            reg_dicr = a;
        } else if (sscanf(line, "POKE %x", &a) == 1) {
            reg_imask = a;
        } else if (sscanf(line, "CALL %15s %x %x", kind, &a, &b) == 3) {
            char s[64]; uint32_t r = run_kind(kind, a, b);
            if (!strcmp(kind, "FLUSH") || !strcmp(kind, "CPAD") || !strcmp(kind, "EXIT")) snprintf(s, 64, "RETV");   /* void natives */
            else snprintf(s, 64, "RET %08X", r);
            ev(s);
        } else if (!strncmp(line, "CB ", 3)) {
            unsigned fn; sscanf(line, "CB %x", &fn); snprintf(want[nwant++], 64, "CB %08X", fn);   /* by target */
        } else if (!strncmp(line, "MMIO", 4) || !strncmp(line, "RET", 3) || !strncmp(line, "STORE", 5)) {
            snprintf(want[nwant++], 64, "%.63s", line);
        } else if (!strncmp(line, "BIOS", 4)) {
#if defined(CDX43_LIBETC) || defined(CDX43_VSYNC)
            (void)0;   /* the veneers are the code under test: their BIOS call is not observable on the native */
        } else if (0) {
#endif
            /* CLEARPAD takes one argument, ENTER/EXIT none: trim recorded extras. */
            char nm[16]; sscanf(line, "BIOS %15s %x %x", nm, &a, &b);
            if (!strcmp(nm, "CLEARPAD")) snprintf(want[nwant++], 64, "BIOS CLEARPAD %08X", a);
            else if (!strcmp(nm, "CLEARRCNT") || !strcmp(nm, "PRINTF")) snprintf(want[nwant++], 64, "%.63s", line);
            else snprintf(want[nwant++], 64, "BIOS %s", nm);
        } else if (sscanf(line, "RAM %x %x %x", &a, &b, &c) == 3) {
            uint32_t o = a & 0x1FFFFFu, v; memcpy(&v, ram + o, 4);
            if (v != b && bad < 40) fprintf(stderr, "case %u RAM %08X = %08X, want %08X\n", cases - 1, a, v, b);
            if (v != b) { bad++; }
            touched[o] = touched[o + 1] = touched[o + 2] = touched[o + 3] = 0;
        } else if (!strncmp(line, "END", 3) && in_case) {
            { const char *nm = line[3] ? line + 4 : "?"; unsigned q;
              for (q = 0; q < nent && strcmp(ent_name[q], nm); q++) ;
              if (q == nent) { snprintf(ent_name[nent++], 48, "%s", nm); } cur_ent = q; ent_cases[q]++; }
            int ok = ngot == nwant;
            for (k = 0; ok && k < nwant; k++) ok = !strcmp(got[k], want[k]);
            if (!ok) {
                bad++;
                if (bad < 40) {
                    fprintf(stderr, "case %u event mismatch (got %u, want %u)\n", cases - 1, ngot, nwant);
                    for (k = 0; k < (ngot > nwant ? ngot : nwant); k++)
                        fprintf(stderr, "  %-34s | %s\n", k < ngot ? got[k] : "-", k < nwant ? want[k] : "-");
                }
            }
            for (k = 0; k < 0x200000u; k++) if (touched[k] && ram[k] != init_ram[k]) {
                if (bad < 40) fprintf(stderr, "case %u unexpected RAM write at %08X\n", cases - 1, 0x80000000u | k);
                bad++; break;
            }
            if (bad != case_bad) ent_bad[cur_ent]++;
            in_case = 0;
        }
    }
    printf("mode=%s checked %u cases, %u mismatches\n", mutant == 1 ? "noop" : mutant == 2 ? "flip" : "native", cases, bad);
    { unsigned q; for (q = 0; q < nent; q++) printf("  %-32s cases=%u failing_cases=%u\n", ent_name[q], ent_cases[q], ent_bad[q]); }
    return mutant ? 0 : bad != 0;
}
