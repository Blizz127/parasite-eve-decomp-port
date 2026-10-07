/*
 * pe-leaf-oracle-tests — differential check of hand adapters against the
 * matched C, with no disc and no retail bytes in git.
 *
 * pc_port/tools/build_leaf_oracle.py compiles the matched src/ leaves with the
 * era toolchain into build/leaf-oracle/image.bin (git-ignored).  This test
 * loads that image into guest RAM, runs each leaf in the TEST-ONLY R3000A
 * oracle (tests/pe_akao_interp_oracle.c), restores the same randomised guest
 * state, runs the native C, and compares all of guest RAM.  The port binary
 * itself never contains the oracle (native only, 2026-10-07).
 *
 * Exit 0 = all cases equal, 1 = a mismatch, 77 = image missing (skip).
 * PE_LEAF_ORACLE_DIR overrides the image directory; PE_LEAF_ORACLE_CASES the
 * case count per leaf (default 400).
 */
#include "pe_guest_ram.h"
#include "pe_audio_driver.h"
#include "pe_guest_decomp.h"
#include "pe_sdk.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void func_8008A750(pe_addr_t a0, pe_addr_t s, unsigned int m, int a3);
void func_8008AC40(int arg0);
void func_8008A92C(pe_addr_t pe_a0, int a1, int a2);
void func_8008B084(pe_addr_t arg0);
void func_8008B168(pe_addr_t a0);
void func_8008B124(int arg0);
void func_8008B0C8(pe_addr_t a0);
void func_80090C88(pe_addr_t a0);
void Trace_Direct(const char *event) { (void)event; }   /* host trace hook (port_main.c) */

static uint32_t g_rng = 0x12345678u;
static uint32_t rnd(void)
{
    g_rng ^= g_rng << 13; g_rng ^= g_rng >> 17; g_rng ^= g_rng << 5;
    return g_rng;
}

static struct { char name[64]; uint32_t addr; } g_sym[64];
static unsigned g_nsym;
static uint32_t g_img_base, g_img_size;
static uint8_t g_img[0x10000];

static int load_image(const char *dir)
{
    char path[512], name[64];
    unsigned addr, size;
    FILE *f;
    snprintf(path, sizeof path, "%s/manifest.txt", dir);
    if (!(f = fopen(path, "r"))) return 0;
    if (fscanf(f, "image %x %u\n", &addr, &size) != 2 || size > sizeof g_img) { fclose(f); return 0; }
    g_img_base = addr; g_img_size = size;
    while (g_nsym < 64 && fscanf(f, "%63s %x\n", name, &addr) == 2) {
        snprintf(g_sym[g_nsym].name, sizeof g_sym[0].name, "%s", name);
        g_sym[g_nsym++].addr = addr;
    }
    fclose(f);
    snprintf(path, sizeof path, "%s/image.bin", dir);
    if (!(f = fopen(path, "rb"))) return 0;
    if (fread(g_img, 1, size, f) != size) { fclose(f); return 0; }
    fclose(f);
    return 1;
}

static uint32_t sym(const char *n)
{
    for (unsigned i = 0; i < g_nsym; i++)
        if (!strcmp(g_sym[i].name, n)) return g_sym[i].addr;
    fprintf(stderr, "leaf-oracle: %s not in the image (rebuild it)\n", n);
    exit(1);
}

static void install_image(void)
{
    uint8_t *ram = (uint8_t *)PE_Translate(g_img_base, g_img_size);
    memcpy(ram, g_img, g_img_size);
}

/* Random AKAO state: the sound driver's globals and voice banks
 * (0x8009C000..0x800BE000), plus a control-record window at 0x80100000 that
 * D_8009D2C8 points into. */
static void randomise(void)
{
    uint8_t *ram = (uint8_t *)PE_Translate(0x80000000u, PE_RAM_SIZE);
    for (uint32_t a = 0x9C000u; a < 0xBE000u; a += 4) {
        uint32_t r = rnd();
        memcpy(ram + a, &r, 4);
    }
    for (uint32_t a = 0x100000u; a < 0x120000u; a += 4) {
        uint32_t r = rnd();
        memcpy(ram + a, &r, 4);
    }
    PE_StoreU32(0x8009D2C8u, 0x80100000u + (rnd() % 0x300u) * 4u);
    /* func_8008AB1C's lookup tables (u16 offsets / byte base). */
    PE_StoreU32(0x8009D240u, 0x80104000u + (rnd() % 0x100u) * 2u);
    PE_StoreU32(0x8009D260u, 0x80108000u + (rnd() % 0x100u));
}

static uint8_t *g_snap, *g_ref;

static int compare(const char *what, unsigned c)
{
    uint8_t *ram = (uint8_t *)PE_Translate(0x80000000u, PE_RAM_SIZE);
    unsigned shown = 0, nd = 0;
    for (uint32_t i = 0; i < PE_RAM_SIZE; i += 4) {
        uint32_t a, b;
        memcpy(&a, g_ref + i, 4); memcpy(&b, ram + i, 4);
        if (a != b) {
            if (shown++ < 6)
                fprintf(stderr, "  %s case %u: %08X oracle=%08X native=%08X\n",
                        what, c, 0x80000000u + i, a, b);
            nd++;
        }
    }
    return nd == 0;
}

typedef void (*CaseFn)(uint32_t entry, int oracle, uint32_t *args);

static int run_leaf(const char *what, uint32_t entry, unsigned cases,
                    void (*setup)(uint32_t *args), void (*native)(const uint32_t *args))
{
    unsigned bad = 0, faults = 0;
    uint32_t args[4];
    uint8_t *ram = (uint8_t *)PE_Translate(0x80000000u, PE_RAM_SIZE);
    for (unsigned c = 0; c < cases; c++) {
        int ok = 0;
        randomise();
        setup(args);
        install_image();
        memcpy(g_snap, ram, PE_RAM_SIZE);
        (void)PE_AudioDriver_CallGuest(entry, args[0], args[1], args[2], args[3], &ok);
        if (!ok) { faults++; if (faults < 4) fprintf(stderr, "  %s case %u: oracle fault\n", what, c); }
        memcpy(g_ref, ram, PE_RAM_SIZE);
        memcpy(ram, g_snap, PE_RAM_SIZE);
        native(args);
        if (!compare(what, c)) bad++;
    }
    printf("LEAF_ORACLE %s: %u cases, %u mismatched, %u oracle faults -> %s\n",
           what, cases, bad, faults, (bad || faults) ? "FAIL" : "PASS");
    return !(bad || faults);
}

/* func_8008A750(voice, stream record, voice bit, arg3): voices are bank
 * slots (0x800B8AC0/0x800BA560/0x800BC000 + k*0x11C); the record is any
 * word-aligned RAM in the randomised window. */
static void setup_a750(uint32_t *a)
{
    static const uint32_t banks[3] = {0x800B8AC0u, 0x800BA560u, 0x800BC000u};
    unsigned k = rnd() % 24u;
    a[0] = banks[rnd() % 3u] + k * 0x11Cu;
    if (a[0] + 0x11Cu > 0x800BE000u) a[0] = 0x800B8AC0u + k * 0x11Cu;
    a[1] = 0x80100000u + (rnd() % 0x3C0u) * 4u;
    a[2] = (rnd() & 1u) ? (1u << (rnd() % 24u)) : rnd();
    a[3] = (rnd() & 1u) ? (rnd() % 64u) : rnd();
    /* func_8008F178 indexes D_800B2900 by its second argument (0 here). */
}
static void native_a750(const uint32_t *a)
{
    func_8008A750(a[0], a[1], a[2], (int)a[3]);
}

static void setup_ac40(uint32_t *a)
{
    a[0] = (rnd() & 1u) ? (rnd() % 0x10000u) : rnd();
    a[1] = a[2] = a[3] = 0;
}
static void native_ac40(const uint32_t *a)
{
    func_8008AC40((int)a[0]);
}

/* Stream-record callers (func_8008A92C and the AKAO opcode hand ports that
 * call it).  rec is word-aligned RAM in the window; rec+8 (the stop mask
 * handed to func_8008A400) is 0 half the time; p1/p2 are 0 a third of the
 * time each (the "no zone" cases). */
static uint32_t rec_addr(void) { return 0x80110000u + (rnd() % 0x800u) * 4u; }
static uint32_t maybe0(void) { return (rnd() % 3u) == 0 ? 0u : rnd(); }
static void setup_a92c(uint32_t *a)
{
    a[0] = rec_addr();
    if (rnd() & 1u) PE_StoreU32(a[0] + 8u, 0u);
    a[1] = maybe0();
    a[2] = maybe0();
    a[3] = 0;
}
static void native_a92c(const uint32_t *a) { func_8008A92C(a[0], (int)a[1], (int)a[2]); }

static void setup_rec(uint32_t *a)
{
    a[0] = rec_addr();
    if (rnd() & 1u) PE_StoreU32(a[0] + 8u, 0u);
    a[1] = a[2] = a[3] = 0;
}
static void native_b084(const uint32_t *a) { func_8008B084(a[0]); }
static void native_b124(const uint32_t *a) { func_8008B124((int)a[0]); }
static void native_b0c8(const uint32_t *a) { func_8008B0C8(a[0]); }

/* func_8008B168: *(a0+4) points at two u16 offsets (0xFFFF = none). */
static void setup_b168(uint32_t *a)
{
    uint32_t base = 0x80118000u + (rnd() % 0x400u) * 4u;
    setup_rec(a);
    PE_StoreU32(a[0] + 4u, base);
    if ((rnd() % 3u) == 0) PE_StoreU16(base, 0xFFFFu);
    if ((rnd() % 3u) == 0) PE_StoreU16(base + 2u, 0xFFFFu);
}
static void native_b168(const uint32_t *a) { func_8008B168(a[0]); }

/* func_80090C88: a0 is a voice; *(a0) is its script cursor (two u16 LE
 * offsets, 0 = none). */
static void setup_0c88(uint32_t *a)
{
    uint32_t cur = 0x80118000u + (rnd() % 0x400u) * 4u + (rnd() & 3u);
    a[0] = 0x800B8AC0u + (rnd() % 24u) * 0x11Cu;
    PE_StoreU32(a[0], cur);
    if ((rnd() % 3u) == 0) { PE_StoreU8(cur, 0); PE_StoreU8(cur + 1u, 0); }
    if ((rnd() % 3u) == 0) { PE_StoreU8(cur + 2u, 0); PE_StoreU8(cur + 3u, 0); }
    a[1] = a[2] = a[3] = 0;
}
static void native_0c88(const uint32_t *a) { func_80090C88(a[0]); }

int main(void)
{
    const char *dir = getenv("PE_LEAF_ORACLE_DIR");
    const char *nc = getenv("PE_LEAF_ORACLE_CASES");
    unsigned cases = nc ? (unsigned)atoi(nc) : 400u;
    int ok = 1;
    if (!dir) dir = "build/leaf-oracle";
    if (!load_image(dir)) {
        printf("LEAF_ORACLE SKIP: no image in %s (run pc_port/tools/build_leaf_oracle.py)\n", dir);
        return 77;
    }
    PE_RamInit();
    g_snap = (uint8_t *)malloc(PE_RAM_SIZE);
    g_ref = (uint8_t *)malloc(PE_RAM_SIZE);
    if (!g_snap || !g_ref) return 2;
    ok &= run_leaf("func_8008A750", sym("func_8008A750"), cases, setup_a750, native_a750);
    ok &= run_leaf("func_8008AC40", sym("func_8008AC40"), cases, setup_ac40, native_ac40);
    ok &= run_leaf("func_8008A92C", sym("func_8008A92C"), cases, setup_a92c, native_a92c);
    ok &= run_leaf("func_8008B084", sym("func_8008B084"), cases, setup_rec, native_b084);
    ok &= run_leaf("func_8008B124", sym("func_8008B124"), cases, setup_rec, native_b124);
    ok &= run_leaf("func_8008B0C8", sym("func_8008B0C8"), cases, setup_rec, native_b0c8);
    ok &= run_leaf("func_8008B168", sym("func_8008B168"), cases, setup_b168, native_b168);
    ok &= run_leaf("func_80090C88", sym("func_80090C88"), cases, setup_0c88, native_0c88);
    printf("LEAF_ORACLE %s\n", ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
