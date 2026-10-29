/* room_m0428i — func_80192B8C, blob offset 0x3BA4, 0x2D4 bytes. Flags -O2 -G0 (default
 * era_o2_g0, no knobs); LINK_EXACT at the target VMA (focus lane f10 2026-10-01). Plain C.
 * Particle emitter: mode 0 allocates the D_801941C4 sprite and registers the
 * func_801928CC pool (func_800CE560 0x14, 4); mode 1 spawns four particles six ticks apart
 * on the owner's +0x1FC position (alternating +/-0x80 drift), plays sounds 0x5C4/0x5C5 on
 * the first, then returns 1 once D_801941C8 reaches 0; mode 2 selects the texture page.
 * Levers: `++p[0] == mode` (retail reuses $s1 = mode as the constant 1); separate case-0 /
 * case-1 pointer locals; drift value in a local `v` stored after the 0x200 pair; chained
 * `ob[5] = ob[4] = 0x200` (reverse store order); `ob[2] = 0` after the pointer store;
 * `int u` palette word (keeps the andi 0xFFFF). */

#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(char **)((char *)(o) + (x)))
extern int D_800B0E64;
extern short D_801941C8;
extern int D_801941C4;
extern char *D_800F33E0;
extern char *D_800F32D0;
extern unsigned short D_800E11FA;
extern unsigned short D_800E2850[];
extern unsigned short D_800F336C;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern int func_8006E498();
extern void func_800C6D5C();
extern int func_800CE560();
extern short *func_800CE610();
extern void func_8006DCE4();
extern int func_80077A64();
extern int func_80077AA4();
extern void func_800C6EC0(unsigned short, unsigned short);
extern void func_800C6ED8();
extern int func_801928CC();

int func_80192B8C(int mode, short *p)
{
    short *ob;
    char *t;
    char *q;
    int u;
    int v;
    unsigned short k;
    int w;

    switch (mode) {
    case 0:
        D_801941C8 = 4;
        D_801941C4 = func_8006E498(D_800B0E64, 0xC58C7704);
        func_800C6D5C(D_801941C4, 0, 0);
        q = D_800F33E0;
        p[0] = 0;
        p[1] = 0;
        return func_800CE560(P(q, 8), 0x14, 4, func_801928CC);
    case 1:
        if (p[0] < 4) {
            if (++p[1] < 6)
                break;
            ob = func_800CE610(P(D_800F33E0, 8));
            if (ob == 0)
                break;
            t = D_800F32D0;
            *(char **)ob = P(t, 8) + 0x1FC;
            ob[2] = 0;
            v = (p[0] & 1) ? 0x80 : -0x80;
            ob[5] = ob[4] = 0x200;
            ob[3] = v;
            ob[6] = 0;
            ob[7] = 0;
            ob[8] = 0xC0;
            ob[9] = p[0] == 0;
            p[1] = 0;
            if (++p[0] == mode) {
                func_8006DCE4(0x5C4, P(P(P(t, 8), 0), 8), H(P(P(t, 8), 0x238), 0x14),
                              H(P(P(t, 8), 0x238), 0x18), H(P(P(t, 8), 0x238), 0x1C));
                func_8006DCE4(0x5C5, P(P(P(D_800F32D0, 8), 0), 8), H(P(P(D_800F32D0, 8), 0x238), 0x14),
                              H(P(P(D_800F32D0, 8), 0x238), 0x18), H(P(P(D_800F32D0, 8), 0x238), 0x1C));
            }
        } else {
            if (D_801941C8 != 0)
                break;
            return 1;
        }
        break;
    case 2:
        u = (D_800E2850[D_800E11FA] | func_80077A64(0, 1, 0, 0)) & 0xFFFF;
        k = D_800F336C;
        w = D_800E1204[k] + ((k == 4 && D_800F3428 != 0) ? 10 : 6);
        func_800C6EC0(u, func_80077AA4(0, w));
        func_800C6ED8(1);
        break;
    }
    return 0;
}
