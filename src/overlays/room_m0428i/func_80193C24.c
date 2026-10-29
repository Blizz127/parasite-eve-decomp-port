/* room_m0428i — func_80193C24, blob offset 0x4C3C, 0x2B8 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (focus lane f3 2026-10-01). inline GTE/asm: one
 * zero-instruction `__asm__ volatile("" ::: "memory")` scheduling barrier in case 2.
 * Room effect controller: cmd 0 clears the owner's counters and registers the
 * func_80193A58 particle pool (func_800CE560 0x18, 8); cmd 1 spawns up to 0x18
 * func_800CE610 particles at random D_800F32D0 anchor triples, sets +0x14 = 0x30000
 * once the owner's +0x16 count reaches +0xF, returns 2 (D_801941D0 = 1) when done;
 * cmd 2 sets the D_800F3368 draw state (0x40/4, palette D_800E2850[D_800E11FA]).
 * Levers: case 1 written `if (s[0] < 0x18) {..} else {D_801941D0 = 1; return 2;}`;
 * case 0 reads D_800F33E0 into its own local before the two zero stores; `n = s[0]`
 * read before the ob[8] store (gives retail's lh); D_800F3368 block as one struct;
 * case 2 index/palette in block locals with the palette load before the barrier
 * (without it sched1 regroups the stores: 25-26 mismatches). */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define H(o, x) (*(unsigned short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(char **)((char *)(o) + (x)))

extern char *D_800F32D0;
extern char *D_800F33E0;
extern unsigned char D_801941D0;
extern unsigned short D_800E11FA;
extern unsigned short D_800E2850[];
extern struct { short a68, a6A, a6C, a6E; unsigned short a70; short a72, a74, a76, a78; } D_800F3368;
extern void func_80193A58();
extern int func_800CE560();
extern short *func_800CE610();
extern int func_80071A54();

int func_80193C24(int mode, short *s)
{
    short *ob;
    char *t;
    char *q;
    char *p;
    int r;
    int idx;
    int hi;
    int n;

    switch (mode) {
    case 0:
        q = D_800F33E0;
        s[0] = 0;
        s[1] = 0;
        return func_800CE560(P(q, 8), 0x18, 8, func_80193A58);
    case 1:
        if (s[0] < 0x18) {
        if (--s[1] == -1) {
            ob = func_800CE610(P(D_800F33E0, 8));
            if (ob) {
                r = func_80071A54();
                t = D_800F32D0;
                idx = (r >> 2) % 38;
                ob[0] = W(P(P(t, 8), 0x238) + idx * 32, 0x14);
                ob[1] = W(P(P(t, 8), 0x238) + idx * 32, 0x18);
                ob[2] = W(P(P(t, 8), 0x238) + idx * 32, 0x1C);
                n = s[0];
                ob[8] = 0x80;
                ob[10] = (n << 11) + 0x800;
                s[1] = (r & 7) + 0xC;
                s[0]++;
                r = func_80071A54();
                ob[4] = (r & 0xFF) + 0x200;
                hi = (r >> 8) & 0xFF;
                if (r & 2)
                    ob[5] = hi + 0x300;
                else
                    ob[5] = hi - 0x400;
                ob[6] = 0;
                ob[7] = 0;
            }
        }
        p = P(D_800F32D0, 8);
        if (H(p, 0x16) >= B(p, 0xF))
            W(p, 0x14) = 0x30000;
        } else {
            D_801941D0 = 1;
            return 2;
        }
        break;
    case 2: {
        int index = D_800E11FA;
        int palette;
        D_800F3368.a68 = 0x40;
        D_800F3368.a6A = 4;
        D_800F3368.a76 = 0x40;
        D_800F3368.a78 = 0x40;
        palette = D_800E2850[index];
        __asm__ volatile("" : : : "memory");
        D_800F3368.a6C = 3;
        D_800F3368.a6E = 1;
        D_800F3368.a72 = 5;
        D_800F3368.a74 = 0;
        D_800F3368.a70 = palette;
        break;
    }
    }
    return 0;
}
