/* room_m0428i — func_801935F0, blob offset 0x4608, 0x468 bytes. Flags -O2 -G0 +
 * MASPSX_EXPAND_DIV=1 (profile era_o2_g0_expand_div); LINK_EXACT at the target VMA (focus lane
 * f10 2026-10-01). inline GTE/asm: Psy-Q SetRotMatrix/SetTransMatrix/ldv0/rtv0/stsv macros
 * (same text as room_m0141i func_80191330); no scheduling barriers.
 * Spark controller (sibling of func_80193C24): mode 0 plays sound 0x546, seeds the counters
 * and registers the func_80193488 pool (func_800CE560 0xC, 0xA); mode 1 returns 2 once
 * D_801941D0 is set, otherwise every 4 ticks spawns a particle at a random D_800F32D0 anchor
 * (% 38), offsets it by a random-yaw rotated vector and plays sound 0x5FD; mode 2 sets the
 * D_800F3368 draw state (0x20/2, palette D_800E2850[D_800E11FA]).
 * Levers: `% d` with a block-local `int d = 0x60` (retail divides by register, no magic
 * multiply) plus MASPSX_EXPAND_DIV for the break guards (134 mismatches without the knob);
 * `(r & 0x7F) * ob[4]` operand order; `int n = s[0]` (lh); ob[3] = 0 after both adds;
 * per-case pointer locals (t, q, a in case 0; u for the case-1 sound call); the case-0 pool
 * argument read into `a` before the D_801941D0 store. */

#define gte_SetRotMatrix(r0) __asm__ volatile ( \
    "lw $12,0(%0)\n\tlw $13,4(%0)\n\tctc2 $12,$0\n\tctc2 $13,$1\n\t" \
    "lw $12,8(%0)\n\tlw $13,12(%0)\n\tlw $14,16(%0)\n\t" \
    "ctc2 $12,$2\n\tctc2 $13,$3\n\tctc2 $14,$4" : : "r"(r0) : "$12", "$13", "$14")
#define gte_SetTransMatrix(r0) __asm__ volatile ( \
    "lw $12,20(%0)\n\tlw $13,24(%0)\n\tctc2 $12,$5\n\tlw $14,28(%0)\n\t" \
    "ctc2 $13,$6\n\tctc2 $14,$7" : : "r"(r0) : "$12", "$13", "$14")
#define gte_ldv0(r0) __asm__ volatile ( \
    "lwc2 $0,0(%0)\n\tlwc2 $1,4(%0)" : : "r"(r0))
#define gte_rtv0() __asm__ volatile ("nop\n\tnop\n\t.word 0x4A480012")
#define gte_stsv(r0) __asm__ volatile ( \
    "mfc2 $12,$9\n\tmfc2 $13,$10\n\tmfc2 $14,$11\n\t" \
    "sh $12,0(%0)\n\tsh $13,2(%0)\n\tsh $14,4(%0)" : : "r"(r0) : "$12", "$13", "$14", "memory")

#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(char **)((char *)(o) + (x)))

typedef struct { short m[3][3]; int t[3]; } MATRIX;
typedef struct { short x, y, z, w; } SV;

extern char *D_800F32D0;
extern char *D_800F33E0;
extern unsigned char D_801941D0;
extern unsigned short D_800E11FA;
extern unsigned short D_800E2850[];
extern struct { short a68, a6A, a6C, a6E; unsigned short a70; short a72, a74, a76, a78; } D_800F3368;
extern void func_80193488();
extern void func_8006DCE4();
extern int func_800CE560();
extern short *func_800CE610();
extern int func_80052B2C();
extern int func_80071A54();
extern int func_80077DC4();
extern int func_80077CF4();

int func_801935F0(int mode, short *s)
{
    SV v;
    MATRIX m;
    short *ob;
    char *t;
    char *q;
    char *u;
    char *a;
    int r;
    int h;
    int c, sn;
    int n;

    switch (mode) {
    case 0:
        t = D_800F32D0;
        func_8006DCE4(0x546, P(P(P(t, 8), 0), 8), H(P(P(t, 8), 0x238), 0x14),
                      H(P(P(t, 8), 0x238), 0x18), H(P(P(t, 8), 0x238), 0x1C));
        q = D_800F33E0;
        s[0] = 0;
        s[1] = 0x3C;
        a = P(q, 8);
        D_801941D0 = 0;
        return func_800CE560(a, 0xC, 0xA, func_80193488);
    case 1:
        if (D_801941D0 != 0)
            return 2;
        if (s[1]-- > 0)
            break;
        ob = func_800CE610(P(D_800F33E0, 8));
        if (ob == 0)
            break;
        n = s[0];
        ob[4] = (n << 5) + 0x800;
        ob[0] = W(P(P(D_800F32D0, 8), 0x238) + (func_80052B2C() % 38) * 32, 0x14);
        ob[1] = W(P(P(D_800F32D0, 8), 0x238) + (func_80052B2C() % 38) * 32, 0x18);
        ob[2] = W(P(P(D_800F32D0, 8), 0x238) + (func_80052B2C() % 38) * 32, 0x1C);
        r = func_80052B2C();
        ob[1] -= ((r & 0x7F) * ob[4]) >> 13;
        h = ob[4] >> 1;
        v.x = 0;
        {
            int d = 0x60;
            v.z = (((func_80071A54() % d) + 0x40) * h) >> 12;
        }
        c = func_80077DC4(func_80071A54());
        sn = func_80077CF4(func_80071A54());
        m.m[0][2] = sn;
        m.m[2][0] = -sn;
        m.m[1][1] = 0x1000;
        m.m[0][0] = c;
        m.m[2][2] = c;
        m.t[0] = m.t[1] = m.t[2] = 0;
        m.m[0][1] = m.m[1][0] = m.m[1][2] = m.m[2][1] = 0;
        gte_SetRotMatrix(&m);
        gte_SetTransMatrix(&m);
        gte_ldv0(&v);
        gte_rtv0();
        gte_stsv(&v);
        ob[0] += v.x;
        ob[2] += v.z;
        ob[3] = 0;
        s[1] = 4;
        u = D_800F32D0;
        s[0]++;
        func_8006DCE4(0x5FD, P(P(P(u, 8), 0), 8), ob[0], ob[1], ob[2]);
        break;
    case 2: {
        int index = D_800E11FA;
        int palette;
        D_800F3368.a68 = 0x20;
        D_800F3368.a6A = 2;
        D_800F3368.a76 = 0x20;
        D_800F3368.a78 = 0x20;
        palette = D_800E2850[index];
        D_800F3368.a6C = 3;
        D_800F3368.a6E = 1;
        D_800F3368.a72 = 0;
        D_800F3368.a74 = 0x20;
        D_800F3368.a70 = palette;
        break;
    }
    }
    return 0;
}
