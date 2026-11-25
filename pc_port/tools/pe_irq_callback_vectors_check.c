/* Vector check for platform/pe_plat_irq_callback.c against the OUT.txt written
 * by pe_irq_callback_vectors.py.  Build from the repo root:
 *   cc -std=c99 -Wall -Wextra -Werror -Ipc_port/include -Ipc_port/platform \
 *      pc_port/tools/pe_irq_callback_vectors_check.c pc_port/platform/pe_plat_irq_callback.c -o check
 *   ./check OUT.txt build/disc1.candidate.exe
 * Host stand-ins: guest RAM (PE_LoadU8..32, PE_StoreU8..32), the hardware register bus
 * (PE_MMIO_Store/Load; I_STAT write acknowledges: I_STAT &= value) and the
 * host BIOS models declared in pe_plat/bios_sys.h. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "pe_guest_ram.h"
#include "pe_mmio.h"
#include "pe_plat/bios_sys.h"
#include "pe_plat/irq_callback.h"
#include "pe_bios_string.h"

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

void pe_plat_bios_change_clear_pad(uint32_t flag)
{ char s[64]; snprintf(s, 64, "BIOS CLEARPAD %08X", flag); ev(s); }
uint32_t pe_plat_bios_change_clear_rcnt(uint32_t t, uint32_t flag)
{
    char s[64]; uint32_t old = rcnt_flag[t & 3];
    snprintf(s, 64, "BIOS CLEARRCNT %08X %08X", t, flag); ev(s); rcnt_flag[t & 3] = flag; return old;
}
int pe_plat_bios_enter_critical(void) { ev("BIOS ENTER"); return 0; }
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
/* Test setup only (the original VSyncCallbacks): store when changed, return old. */
static uint32_t setup_vsync_slot(uint32_t slot, uint32_t h)
{
    uint32_t a = 0x8009568Cu + slot * 4u, old = PE_LoadU32(a);
    if (old != h) PE_StoreU32(a, h);
    return old;
}
void pe_plat_bios_exit_critical(void) { ev("BIOS EXIT"); }

int main(int argc, char **argv)
{
    FILE *f, *e; char line[256]; static char want[40000][64]; unsigned nwant = 0, cases = 0, bad = 0, k;
    static uint8_t exe[0x200000]; uint32_t t_addr, t_size; int in_case = 0;
    if (argc != 3 || !(f = fopen(argv[1], "r")) || !(e = fopen(argv[2], "rb"))) return 2;
    (void)fread(exe, 1, sizeof exe, e); memcpy(&t_addr, exe + 0x18, 4); memcpy(&t_size, exe + 0x1C, 4);
    while (fgets(line, sizeof line, f)) {
        unsigned id, im, is, dirty; uint32_t dicr0; char kind[16]; uint32_t a, b, c;
        line[strcspn(line, "\n")] = 0;
        if (sscanf(line, "CASE %u %x %x %u %x", &id, &im, &is, &dirty, &dicr0) == 5) {
            memset(ram, 0, sizeof ram); memcpy(ram + (t_addr & 0x1FFFFFu), exe + 0x800, t_size);
            if (dirty) { memset(ram + 0x945E4, 0xA5, 0x41A * 4); ram[0x945E4] = ram[0x945E5] = 0; }
            memcpy(init_ram, ram, sizeof ram); memset(touched, 0, sizeof touched);
            reg_imask = im; reg_istat = is; reg_dpcr = 0; reg_dicr = dicr0; memset(reg_t, 0, sizeof reg_t);
            memset(rcnt_flag, 0, sizeof rcnt_flag); ngot = nwant = 0; in_case = 1; cases++;
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
            char s[64]; uint32_t r = !strcmp(kind, "RESET") ? pe_plat_reset_callback()
                                   : (!strcmp(kind, "DMACB") || !strcmp(kind, "DMASET")) ? pe_plat_dma_callback(a, b)
                                   : !strcmp(kind, "VSYNCCBS") ? setup_vsync_slot(a, b)
                                   : !strcmp(kind, "VSYNCH") ? pe_plat_vsync_handler()
                                   : !strcmp(kind, "DMAH") ? pe_plat_dma_handler()
                                   : !strcmp(kind, "STARTV") ? pe_plat_start_vsync()
                                   : !strcmp(kind, "STARTD") ? pe_plat_start_dma()
                                   : !strcmp(kind, "DISP") ? pe_plat_irq_dispatch()
                                   : !strcmp(kind, "STOP") ? pe_plat_stop_callback()
                                   : !strcmp(kind, "RESTART") ? pe_plat_restart_callback()
                                                           : pe_plat_interrupt_callback(a, b);
            snprintf(s, 64, "RET %08X", r); ev(s);
        } else if (!strncmp(line, "MMIO", 4) || !strncmp(line, "RET", 3) || !strncmp(line, "STORE", 5) ||
                   !strncmp(line, "CB ", 3)) {
            snprintf(want[nwant++], 64, "%.63s", line);
        } else if (!strncmp(line, "BIOS", 4)) {
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
        } else if (!strcmp(line, "END") && in_case) {
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
            in_case = 0;
        }
    }
    printf("checked %u cases, %u mismatches\n", cases, bad);
    return bad != 0;
}
