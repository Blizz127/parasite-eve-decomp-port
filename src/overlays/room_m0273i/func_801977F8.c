/* room_m0273i — func_801977F8, blob offset 0x8810, 0x250 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl13 2026-09-28).
 * CEE20 + two D004C: int-array sin index D_800966EC[(t << 6) & 0xFC0] (keeps t << 8 un-CSEd, 121->108), explicit pc/pv/one/tb locals for the callee-saved args, w pinned $3 (108->0). */

typedef struct { short x, y, z, w; } SV;
extern int D_800E27EC;
extern unsigned short D_800F336C;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern int D_800966EC[];
extern unsigned short D_800942EC;
extern char D_8019AB70[];
extern char D_8019AD74[];
extern char D_8019AD78[];
extern unsigned short func_80077AA4();
extern void func_800CEE20();
extern void func_800D004C();

int func_801977F8(int a0, SV *a1)
{
    short v[4];
    SV c;
    int t;
    register int w asm("$3");
    int s4;
    int s5;
    int k;
    int u;
    SV *pc;
    short *pv;
    int one;
    char *tb;

    if (a0 == 1) {
        if (D_800E27EC >= 0x10) {
            return 1;
        }
    } else if (a0 == 2) {
        t = D_800E27EC - 1;
        k = D_800F336C;
        w = D_800966EC[(t << 6) & 0xFC0];
        v[0] = 0;
        v[1] = 0;
        v[2] = t << 8;
        v[3] = 0;
        c = *a1;
        u = D_800E1204[k];
        s5 = w >> 21;
        s4 = (short)w * 3 + 0x800;
        if (k == 4 && D_800F3428 != 0) {
            u += 4;
        }
        pc = &c;
        one = 1;
        func_800CEE20(pc, v, s4, s4, 0x4A, func_80077AA4(0x50, u), one, s5, 0);
        pv = v;
        tb = D_8019AB70;
        v[2] = v[2] >> 1;
        w = D_800966EC[(t << 6) & 0xFC0];
        s5 = w >> 21;
        s4 = (short)w + 0x400;
        func_800D004C(pc, 0x200, 0x200, 0x10, pv, s4, s4, tb, D_8019AD74, s5, one);
        v[0] = 0x400;
        v[2] = t << 8;
        v[3] = 1;
        v[1] = 0;
        c.y = D_800942EC;
        func_800D004C(pc, 0x300, 0x300, 8, pv, 0x1000, 0x1000, D_8019AD78, tb, s5, one);
    }
    return 0;
}
