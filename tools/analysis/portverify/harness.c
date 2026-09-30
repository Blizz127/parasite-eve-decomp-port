/* Differential harness: generated (matched-C) TU vs hand port, forked per trial. */
#include "harness.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/resource.h>
static void die(int sig){ _exit(100+sig); }
void PE_RamInit(void); void PE_RamReset(void); void *PE_Translate(pe_addr_t, size_t);
void PE_Sdk_ResetState(void);
#define RAM 0x200000u
#define SP 0x400u
typedef struct { int status; int64_t ret; uint8_t ram[RAM]; uint8_t sp[SP]; } Slot;
static uint64_t rs;
static uint32_t rnd(void) { rs ^= rs << 13; rs ^= rs >> 7; rs ^= rs << 17; return (uint32_t)(rs >> 11); }
static uint32_t rptr(void) { return 0x80000000u | ((0x10000u + rnd() % 0x1E0000u) & ~3u); }
static void seed_ram(int mode) {
    uint8_t *ram = PE_Translate(0x80000000u, RAM), *sp = PE_Translate(0x1F800000u, SP);
    uint32_t *w = (uint32_t *)ram; unsigned i;
    switch (mode) {
    case 0: memset(ram, 0, RAM); memset(sp, 0, SP); break;
    case 1: for (i = 0; i < RAM / 4; i++) w[i] = rnd(); for (i = 0; i < SP; i++) sp[i] = rnd(); break;
    case 2: for (i = 0; i < RAM / 4; i++) { uint32_t r = rnd() % 8; w[i] = r < 4 ? rptr() : r < 6 ? rnd() % 16 : rnd(); }
            for (i = 0; i < SP / 4; i++) ((uint32_t *)sp)[i] = rptr(); break;
    case 3: for (i = 0; i < RAM; i++) ram[i] = rnd() % 4; for (i = 0; i < SP; i++) sp[i] = rnd() % 4; break;
    case 4: memset(ram, 0xFF, RAM); memset(sp, 0xFF, SP); break;
    case 6: for (i = 0; i < RAM / 4; i++) w[i] = rptr(); for (i = 0; i < SP / 4; i++) ((uint32_t *)sp)[i] = rptr(); break;
    case 7: for (i = 0; i < RAM / 8; i++) w[i] = 0x80100000u | ((rnd() % 0xFF000u) & ~3u);
            for (i = RAM / 2; i < RAM; i++) ram[i] = rnd() % 4; for (i = 0; i < SP / 4; i++) ((uint32_t *)sp)[i] = 0x80100000u | ((rnd() % 0xFF000u) & ~3u); break;
    case 8: for (i = 0; i < RAM; i++) ram[i] = rnd() % 21; for (i = 0; i < SP; i++) sp[i] = rnd() % 21; break;
    case 9: for (i = 0; i < RAM / 4; i++) w[i] = rnd() % 0x41; for (i = 0; i < SP / 4; i++) ((uint32_t *)sp)[i] = rnd() % 0x41; break;
    case 10: for (i = 0; i < RAM / 4; i++) { uint32_t r = rnd() % 3; w[i] = r == 0 ? rptr() : r == 1 ? (rnd() % 16) * 0x01010101u : (rnd() & 0x0F0F0F0Fu); }
            for (i = 0; i < SP / 4; i++) ((uint32_t *)sp)[i] = rnd() % 16; break;
    case 11: for (i = 0; i < RAM; i++) ram[i] = (uint8_t)(0xF8 + rnd() % 8); for (i = 0; i < SP; i++) sp[i] = (uint8_t)(0xF8 + rnd() % 8); break;
    default: for (i = 0; i < RAM / 4; i++) { uint32_t r = rnd() % 4; w[i] = r == 0 ? rptr() : r == 1 ? 0 : r == 2 ? 1 : rnd() % 0x100; }
            for (i = 0; i < SP / 4; i++) ((uint32_t *)sp)[i] = rnd() % 0x100; break;
    }
}
static uint32_t rarg(char k) {
    static const uint32_t pool[] = {0,1,2,3,4,5,7,8,0xF,0x10,0x1F,0x20,0x3F,0x40,0x7F,0x80,0xFF,0x100,0x7FFF,0x8000,0xFFFF,0x10000,0xFFFFFFFFu,0xFFFFFFFEu,0x80000000u,0x7FFFFFFFu};
    uint32_t r = rnd() % 10;
    if (k == 'p') return (rnd() % 16 == 0) ? 0u : r < 4 ? (0x80010000u | ((rnd() % 0xE0000u) & ~3u)) : r < 8 ? rptr() : (r == 8 ? 0x1F800000u + (rnd() % 0x3F0 & ~3u) : rptr() + 1);
    if (r < 2) return rnd() % 21;
    if (r < 6) return pool[rnd() % (sizeof pool / sizeof pool[0])];
    if (r < 8) return rnd() % 64;
    if (r < 9) return rptr();
    return rnd();
}
/* decomp_hand guest scratch windows (hand ports materialise retail stack
 * objects there, e.g. PE_HAND_HI_STACK): retail writes its own stack at those
 * addresses, so differences inside are harness artefacts (portabsent). */
#define PV_MASK_LO 0x1FF500u
#define PV_MASK_HI 0x1FFD80u
static void mask_scratch(Slot *s) { memset(s->ram + PV_MASK_LO, 0, PV_MASK_HI - PV_MASK_LO); }
static void run(Slot *s, int64_t (*f)(const uint32_t *), const uint32_t *a, const char *tag) {
    fflush(NULL);
    pid_t p = fork();
    if (p == 0) {
        if (!getenv("PV_DEBUG")) { int fd = open("/dev/null", 1); dup2(fd, 2); dup2(fd, 1); } else fprintf(stderr, "@%s ", tag);
        alarm(3); { struct rlimit z={0,0}; setrlimit(RLIMIT_CORE,&z); } signal(SIGABRT,die); signal(SIGSEGV,die); signal(SIGBUS,die); signal(SIGFPE,die); signal(SIGILL,die);
        int64_t r = f(a);
        s->ret = r;
        memcpy(s->ram, PE_Translate(0x80000000u, RAM), RAM);
        memcpy(s->sp, PE_Translate(0x1F800000u, SP), SP);
        s->status = 0; _exit(0);
    }
    int st = 0; waitpid(p, &st, 0);
    if (!(WIFEXITED(st) && WEXITSTATUS(st) == 0)) s->status = WIFSIGNALED(st) ? 1000 + WTERMSIG(st) : 2000 + WEXITSTATUS(st);
}

int main(int argc, char **argv) {
    int trials = argc > 1 ? atoi(argv[1]) : 48;
    const char *only = argc > 2 ? argv[2] : NULL;
    Slot *g = mmap(0, sizeof(Slot), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    Slot *h = mmap(0, sizeof(Slot), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    PE_RamInit(); PE_Sdk_ResetState();
    for (const PvEntry *e = pv_table; e->name; e++) {
        if (only && !strstr(only, e->name)) continue;
        unsigned dmask = 0; int ok = 0, both_abort = 0, g_only = 0, h_only = 0, ramd = 0, retd = 0; char ex[512] = "", exa[512] = "";
        for (int t = 0; t < trials; t++) {
            rs = 0x9E3779B97F4A7C15ull ^ ((uint64_t)t * 0x100000001B3ull) ^ (uint64_t)(uintptr_t)e->name[7];
            uint32_t a[6] = {0}; int nk = strlen(e->kinds);
            PE_RamReset(); seed_ram(t % 12);
            for (int i = 0; i < 6; i++) a[i] = i < nk ? rarg(e->kinds[i]) : rarg('i');
            /* pointer args: make the pointee region hold pointer-ish words too */
            g->status = h->status = -1;
            run(g, e->gen, a, "gen"); run(h, e->hand, a, "hand");
            if (g->status && h->status) { both_abort++; continue; }
            if (g->status || h->status) {
                if (g->status) g_only++; else h_only++;
                if (!exa[0]) snprintf(exa, sizeof exa, "trial %d mode %d args %08X %08X %08X %08X: %s aborted (st %d)", t, t % 12, a[0], a[1], a[2], a[3], g->status ? "decomp" : "hand", g->status ? g->status : h->status);
                continue;
            }
            ok++;
            mask_scratch(g); mask_scratch(h);
            int bad = 0;
            if (e->rk == 1 && g->ret != h->ret) { retd++; bad = 1; dmask |= 1u << (t % 12);
                if (!ex[0]) snprintf(ex, sizeof ex, "trial %d mode %d args %08X %08X %08X %08X: ret decomp=%lld hand=%lld", t, t % 12, a[0], a[1], a[2], a[3], (long long)g->ret, (long long)h->ret); }
            if (memcmp(g->ram, h->ram, RAM) || memcmp(g->sp, h->sp, SP)) { ramd++; dmask |= 1u << (t % 12);
                if (!bad || !ex[0]) { char buf[400]; int n = 0; unsigned i, shown = 0;
                    n += snprintf(buf + n, sizeof buf - n, "trial %d mode %d args %08X %08X %08X %08X: ram", t, t % 12, a[0], a[1], a[2], a[3]);
                    for (i = 0; i < RAM && shown < 4; i++) if (g->ram[i] != h->ram[i]) { n += snprintf(buf + n, sizeof buf - n, " [%08X] decomp=%02X hand=%02X", 0x80000000u + i, g->ram[i], h->ram[i]); shown++; }
                    for (i = 0; i < SP && shown < 4; i++) if (g->sp[i] != h->sp[i]) { n += snprintf(buf + n, sizeof buf - n, " [SP+%03X] decomp=%02X hand=%02X", i, g->sp[i], h->sp[i]); shown++; }
                    if (!ex[0]) snprintf(ex, sizeof ex, "%s", buf); } }
        }
        const char *v = (g_only && (ok==0) && 0) ? "" : (ramd || retd) ? "DIVERGE" : (g_only || h_only) ? "ABORTDIFF" : ok >= 4 ? "EQUIV" : "INCONCL";
        printf("%s %s ok=%d bothabort=%d decomp_abort=%d hand_abort=%d ramdiff=%d retdiff=%d rk=%d modes=%03X | %s\n", v, e->name, ok, both_abort, g_only, h_only, ramd, retd, e->rk, dmask, ex[0] ? ex : exa);
        fflush(stdout);
    }
    return 0;
}

/* Host trace sink every pc_port executable provides (see
 * pc_port/tests/test_route_boot_day2.c); the harness ignores traces. */
void Trace_Direct(const char *event) { (void)event; }
