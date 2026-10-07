/*
 * pe-akao-tick-oracle-tests — whole-tick differential for the native AKAO
 * sequencer (func_8008DB7C and the graph it reaches), with no disc and no
 * retail bytes in git.
 *
 *   python3 pc_port/tools/build_leaf_oracle.py --out build/leaf-oracle-akao \
 *       --range 0x80085000 0x80092000 func_8008DB7C
 *
 * compiles every matched src/ function of the sound driver (plus callee
 * closure) with the era toolchain into one relocated image.  Each case:
 *   - randomises the driver's globals, voice banks, control records, a
 *     sequence-byte sandbox and the SPU register file;
 *   - points every voice's script cursor into the sandbox, the SPU base
 *     pointer (D_8009B3FC) at 0x1F801C00 and the control pointer
 *     (D_8009D2C8) into the record window;
 *   - fills the opcode tables D_8009C8F0 / D_8009CCF0 with random handlers
 *     from the image, and places a `j` trampoline at each image function's
 *     retail address so the matched code's indirect calls land in the image
 *     (the native side reaches the same handlers through the guest-code
 *     registry, PE_GuestCall);
 *   - runs the matched tick in the TEST-ONLY interpreter, restores, runs the
 *     native func_8008DB7C, and compares all of guest RAM and the 256 SPU
 *     registers.
 * Cases where the interpreter faults (random state left the modelled
 * address space) are counted and skipped; a case is only judged when the
 * oracle completed.
 *
 * Exit 0 = pass, 1 = mismatch (or too few judged cases), 77 = no image.
 * PE_TICK_ORACLE_DIR / PE_TICK_ORACLE_CASES / PE_TICK_ORACLE_SEED override.
 */
#include "pe_guest_ram.h"
#include "pe_audio_driver.h"
#include "pe_guest_decomp.h"
#include "pe_guestcode.h"
#include "pe_sdk.h"
#include "pe_spu_dma.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void func_8008DB7C(void);
void pe_plat_audio_commit_registers(void);
const char *PE_AkaoOracle_LastFault(uint32_t *pc, uint32_t *addr);
void PE_AkaoTick_Counters(unsigned long out[4]);
void Trace_Direct(const char *event) { (void)event; }   /* host trace hook (port_main.c) */

#define VOICE_SIZE 0x11Cu
#define SANDBOX_LO 0x80150000u
#define SANDBOX_SZ 0x00020000u

static uint32_t g_rng = 0x9E3779B9u;
static uint32_t rnd(void)
{
    g_rng ^= g_rng << 13; g_rng ^= g_rng >> 17; g_rng ^= g_rng << 5;
    return g_rng;
}

#define MAX_SYM 1024
static struct { uint32_t retail, reloc; } g_sym[MAX_SYM];
static unsigned g_nsym;
static uint32_t g_img_base, g_img_size, g_entry;
static uint8_t g_img[0x40000];
static uint32_t g_handlers[MAX_SYM];
static unsigned g_nhandlers;

static int load_image(const char *dir)
{
    char path[512], name[64];
    unsigned addr, size, ret;
    FILE *f;
    snprintf(path, sizeof path, "%s/manifest.txt", dir);
    if (!(f = fopen(path, "r"))) return 0;
    if (fscanf(f, "image %x %u\n", &addr, &size) != 2 || size > sizeof g_img) { fclose(f); return 0; }
    g_img_base = addr; g_img_size = size;
    while (g_nsym < MAX_SYM && fscanf(f, "%63s %x\n", name, &addr) == 2) {
        if (sscanf(name, "func_%8x", &ret) != 1) continue;
        if (ret == 0x8008DB7Cu) g_entry = addr;
        g_sym[g_nsym].retail = ret;
        g_sym[g_nsym++].reloc = addr;
    }
    fclose(f);
    snprintf(path, sizeof path, "%s/image.bin", dir);
    if (!(f = fopen(path, "rb"))) return 0;
    if (fread(g_img, 1, size, f) != size) { fclose(f); return 0; }
    fclose(f);
    return g_entry != 0;
}

/* Opcode-handler candidates: image functions of the sequencer's op range
 * that the native registry can also dispatch.  Overridable with
 * PE_TICK_ORACLE_HANDLERS=lo,hi (hex retail range). */
static void pick_handlers(void)
{
    uint32_t lo = 0x8008F1B0u, hi = 0x80091100u;
    const char *e = getenv("PE_TICK_ORACLE_HANDLERS");
    if (e) sscanf(e, "%x,%x", &lo, &hi);
    for (unsigned i = 0; i < g_nsym; i++) {
        uint32_t r = g_sym[i].retail;
        if (r < lo || r >= hi) continue;
        if (!PE_GuestCode_Resolve(r)) continue;
        g_handlers[g_nhandlers++] = r;
    }
}

static void install_image(void)
{
    memcpy(PE_Translate(g_img_base, g_img_size), g_img, g_img_size);
    for (unsigned i = 0; i < g_nsym; i++) {
        uint32_t r = g_sym[i].retail;
        if (r >= g_img_base) continue;
        PE_StoreU32(r, 0x08000000u | ((g_sym[i].reloc >> 2) & 0x03FFFFFFu));  /* j reloc */
        PE_StoreU32(r + 4u, 0);                                               /* nop */
    }
}

/* Driver state words are built from halfwords that are zero, small
 * (counters, voice indices, sparse voice masks) or, g_mix_rand percent of
 * the time, fully random.  A pointer-typed word then lands in low KUSEG RAM
 * (valid) unless its high half drew a random value, which is what makes
 * the interpreter leave RAM -- those cases are skipped, so the random share
 * trades coverage of big values against judged-case yield. */
static unsigned g_mix_rand = 6;
static uint32_t state_half(void)
{
    uint32_t r = rnd() % 100u;
    if (r < g_mix_rand) return rnd() & 0xFFFFu;
    if (r < 50u) return 0;
    return rnd() & 0x1Fu;
}
static uint32_t state_word(void) { return state_half() | state_half() << 16; }
static void fill(uint32_t lo, uint32_t hi)
{
    uint8_t *ram = (uint8_t *)PE_Translate(0x80000000u, PE_RAM_SIZE);
    for (uint32_t a = lo & 0x1FFFFFu; a < (hi & 0x1FFFFFu); a += 4) {
        uint32_t r = state_word();
        memcpy(ram + a, &r, 4);
    }
}
static void fill_bytes(uint32_t lo, uint32_t hi)
{
    uint8_t *ram = (uint8_t *)PE_Translate(0x80000000u, PE_RAM_SIZE);
    for (uint32_t a = lo & 0x1FFFFFu; a < (hi & 0x1FFFFFu); a += 4) {
        uint32_t r = rnd();
        memcpy(ram + a, &r, 4);
    }
}

static uint32_t sandbox_ptr(void) { return SANDBOX_LO + rnd() % (SANDBOX_SZ - 0x400u); }

static void randomise(void)
{
    unsigned i;
    fill(0x8009B000u, 0x800C2000u);          /* driver globals + voice banks */
    fill(0x80100000u, 0x80120000u);          /* control records */
    fill_bytes(SANDBOX_LO, SANDBOX_LO + SANDBOX_SZ);
    /* Bias the sandbox toward short notes (op < 0xA0 ends a voice's fetch
     * loop) so a tick does a bounded amount of work. */
    for (i = 0; i < SANDBOX_SZ; i += 1) {
        if ((rnd() & 3u) == 0)
            PE_StoreU8(SANDBOX_LO + i, (uint8_t)(rnd() % 0xA0u));
    }
    PE_StoreU32(0x8009B3FCu, 0x1F801C00u);  /* SPU base */
    PE_StoreU32(0x8009D2C8u, 0x80100000u + (rnd() % 0x200u) * 4u);
    PE_StoreU32(0x8009D240u, 0x80104000u + (rnd() % 0x100u) * 2u);
    PE_StoreU32(0x8009D260u, 0x80108000u + (rnd() % 0x100u));
    /* Fast-forward repeat counter: the tick's outer loop only ends when
     * the measure counters wrap, so keep it 0 most of the time. */
    PE_StoreU32(0x8009D22Cu, (rnd() % 8u) ? 0u : 1u);
    /* Make the tick actually advance: the tempo accumulator (+0x28) of
     * each control record usually carries this tick, and voices' note
     * timers (+0x56 / +0x58) often expire, so func_8008E8D0 fetches and
     * dispatches opcodes. */
    for (uint32_t r = 0x80100000u; r < 0x80102000u; r += 0x68u) {
        if (rnd() % 4u) PE_StoreU32(r + 0x28u, 0xFFFFu - (rnd() & 0xFFu));
        PE_StoreU16(r + 0x22u, (uint16_t)rnd());
    }
    if (rnd() & 1u) PE_StoreU32(0x8009D2DCu, PE_LoadU32(0x8009D2DCu) | 4u);
    for (uint32_t v = 0x800B8AC0u; v < 0x800BCD50u; v += VOICE_SIZE) {
        if (rnd() & 1u) PE_StoreU16(v + 0x56u, 1u);
        if (rnd() & 1u) PE_StoreU16(v + 0x58u, 1u);
    }
    /* Voice masks: the music banks use bits 0..23, the SFX bank
     * (D_800BC000, masks D_800BCD50..D_800BCD60) bits 12..23 -- the driver's
     * bit-walk loops never terminate on bits outside those ranges. */
    for (uint32_t r = 0x80100000u; r < 0x80102000u; r += 0x68u)
        for (i = 0; i < 0x20u; i += 4)
            PE_StoreU32(r + i, PE_LoadU32(r + i) & 0x00FFFFFFu);
    for (i = 0x800BCD50u; i <= 0x800BCD60u; i += 4)
        PE_StoreU32(i, (rnd() & 1u) ? 0u : (rnd() & rnd() & 0x00FFF000u));
    /* Documented pointer fields of every voice slot (src/func_80087AA8.c:
     * script cursor +0x00; vibrato / tremolo / pan wave tables +0x1C/+0x20/
     * +0x24) point into the random sandbox.  Banks: 24 voices at
     * 0x800B8AC0, 24 at 0x800BA560, 12 SFX voices at 0x800BC000 (the SFX
     * globals start at 0x800BCD50). */
    for (uint32_t v = 0x800B8AC0u; v < 0x800BCD50u; v += VOICE_SIZE) {   /* 24+24+12 */
        PE_StoreU32(v + 0x00u, sandbox_ptr());
        PE_StoreU32(v + 0x1Cu, sandbox_ptr() & ~1u);
        PE_StoreU32(v + 0x20u, sandbox_ptr() & ~1u);
        PE_StoreU32(v + 0x24u, sandbox_ptr() & ~1u);
        PE_StoreU32(v + 0xF0u, rnd() % 24u);   /* SPU channel (func_800878F0 a0) */
        PE_StoreU32(v + 0x14u, sandbox_ptr() & ~1u);  /* key-split table (func_8008E8D0) */
        PE_StoreU16(v + 0x5Au, (uint16_t)(rnd() % 32u)); /* instrument (func_8008E840 row) */
        if (rnd() % 16u) PE_StoreU16(v + 0x7Eu, (uint16_t)(1u + rnd() % 0x100u)); /* portamento len */
    }
    /* opcode tables */
    for (i = 0xA0u; i < 0x100u; i++)
        PE_StoreU32(0x8009C8F0u + i * 4u, g_handlers[rnd() % g_nhandlers]);
    for (i = 0; i < 0x100u; i++)
        PE_StoreU32(0x8009CCF0u + i * 4u, g_handlers[rnd() % g_nhandlers]);
    for (i = 0; i < 256u; i++)
        PE_SpuRegister_StoreU16(i * 2u, (uint16_t)rnd());
}

static uint8_t *g_snap, *g_ref;
static uint16_t g_spu_snap[256], g_spu_ref[256];

static unsigned compare(unsigned c, unsigned *shown)
{
    uint8_t *ram = (uint8_t *)PE_Translate(0x80000000u, PE_RAM_SIZE);
    unsigned nd = 0;
    /* 0x801FF000..0x801FFFFF: native host scratch windows (the interpreter
     * runs on a private stack), excluded exactly as PE_AKAO_VERIFY did. */
    for (uint32_t i = 0; i < 0x1FF000u; i += 4) {
        uint32_t a, b;
        memcpy(&a, g_ref + i, 4); memcpy(&b, ram + i, 4);
        if (a != b) {
            if ((*shown)++ < 24)
                fprintf(stderr, "  case %u: RAM %08X oracle=%08X native=%08X\n",
                        c, 0x80000000u + i, a, b);
            nd++;
        }
    }
    for (unsigned i = 0; i < 256u; i++) {
        uint16_t b = PE_SpuRegister_LoadU16(i * 2u);
        if (b != g_spu_ref[i]) {
            if ((*shown)++ < 24)
                fprintf(stderr, "  case %u: SPU %03X oracle=%04X native=%04X\n",
                        c, i * 2u, g_spu_ref[i], b);
            nd++;
        }
    }
    return nd;
}

static const char *reloc_name(uint32_t pc, uint32_t *off)
{
    static char buf[32];
    uint32_t best = 0, bret = 0;
    for (unsigned i = 0; i < g_nsym; i++)
        if (g_sym[i].reloc <= pc && g_sym[i].reloc > best) { best = g_sym[i].reloc; bret = g_sym[i].retail; }
    *off = pc - best;
    snprintf(buf, sizeof buf, "func_%08X", bret);
    return buf;
}

int main(void)
{
    const char *dir = getenv("PE_TICK_ORACLE_DIR");
    const char *nc = getenv("PE_TICK_ORACLE_CASES");
    const char *sd = getenv("PE_TICK_ORACLE_SEED");
    unsigned cases = nc ? (unsigned)atoi(nc) : 300u;
    unsigned judged = 0, bad = 0, faults = 0, shown = 0, c;
    uint8_t *ram;
    if (getenv("PE_TICK_ORACLE_RANDPCT")) g_mix_rand = (unsigned)atoi(getenv("PE_TICK_ORACLE_RANDPCT"));
    if (sd) g_rng = (uint32_t)strtoul(sd, 0, 0) | 1u;
    if (!dir) dir = "build/leaf-oracle-akao";
    if (!load_image(dir)) {
        printf("TICK_ORACLE SKIP: no image in %s (run build_leaf_oracle.py --out %s "
               "--range 0x80085000 0x80092000 func_8008DB7C)\n", dir, dir);
        return 77;
    }
    PE_RamInit();
    PE_GuestCode_InstallGenerated();
    PE_Decomp_SetBoundaryStop(0);
    pick_handlers();
    if (!g_nhandlers) { fprintf(stderr, "TICK_ORACLE: no handler candidates\n"); return 1; }
    printf("TICK_ORACLE image %u bytes, %u functions, %u handler candidates\n",
           g_img_size, g_nsym, g_nhandlers);
    g_snap = (uint8_t *)malloc(PE_RAM_SIZE);
    g_ref = (uint8_t *)malloc(PE_RAM_SIZE);
    if (!g_snap || !g_ref) return 2;
    ram = (uint8_t *)PE_Translate(0x80000000u, PE_RAM_SIZE);
    for (c = 0; c < cases; c++) {
        int ok = 0;
        unsigned i, nd;
        randomise();
        install_image();
        memcpy(g_snap, ram, PE_RAM_SIZE);
        for (i = 0; i < 256u; i++) g_spu_snap[i] = PE_SpuRegister_LoadU16(i * 2u);
        (void)PE_AudioDriver_CallGuest(g_entry, 0, 0, 0, 0, &ok);
        if (!ok) {
            uint32_t pc, addr, off;
            const char *what = PE_AkaoOracle_LastFault(&pc, &addr);
            if (faults++ < (getenv("PE_TICK_ORACLE_SHOWFAULTS") ? 100000u : 12u)) {
                const char *fn = reloc_name(pc, &off);
                fprintf(stderr, "  case %u: oracle fault %s addr=%08X in %s+0x%X (skipped)\n",
                        c, what ? what : "?", addr, fn, off);
                if (getenv("PE_TICK_ORACLE_DEBUG"))
                    fprintf(stderr, "    snap CD50=%08X now CD50=%08X\n",
                            *(uint32_t *)(g_snap + 0xBCD50u), PE_LoadU32(0x800BCD50u));
            }
            continue;
        }
        memcpy(g_ref, ram, PE_RAM_SIZE);
        for (i = 0; i < 256u; i++) g_spu_ref[i] = PE_SpuRegister_LoadU16(i * 2u);
        memcpy(ram, g_snap, PE_RAM_SIZE);
        for (i = 0; i < 256u; i++) PE_SpuRegister_StoreU16(i * 2u, g_spu_snap[i]);
        func_8008DB7C();
        pe_plat_audio_commit_registers();
        judged++;
        nd = compare(c, &shown);
        if (nd) bad++;
    }
    {
        unsigned long k[4];
        PE_AkaoTick_Counters(k);
        printf("TICK_ORACLE native coverage over judged cases: func_8008E8D0=%lu func_80087AA8=%lu "
               "func_80088980=%lu key-flag writes=%lu\n", k[0], k[1], k[2], k[3]);
    }
    printf("TICK_ORACLE func_8008DB7C: %u cases, %u judged, %u mismatched, %u oracle faults (skipped) -> %s\n",
           cases, judged, bad, faults, (bad || judged < cases / 4u) ? "FAIL" : "PASS");
    return (bad || judged < cases / 4u) ? 1 : 0;
}
