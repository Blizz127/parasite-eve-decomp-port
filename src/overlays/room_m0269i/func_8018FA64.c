/* room_m0269i — func_8018FA64, blob offset 0xA7C, 0xCA8 bytes.
 * Profile: -O2 -G0 + MASPSX_THREE_WORD_SYMBOL_STORE=1,
 * MASPSX_DISPATCH_FOLD=jtbl_8018F01C,jtbl_8018F034 (both switch tables sit in the room header pool).
 * Room effect controller. cmd 0 toggles D_80192224 between 1 and 0, places the
 * effect beside the owner (func_800CE8F0 slot 4), seeds a trail (func_800D1384),
 * sets the owner flag byte, and arms func_8018F800 (0xC, 0x10).
 * cmd 1: a 5-state machine on +0xB0 (state 0 plays sound 0x598 at frame 2;
 * states 1/2 swing along a1+8 and land on func_800C6B90 contact with owner
 * flag poke 0x3D000000, counting +0xB4 bounces; state 3 stops the sound; state
 * 4 ends after 0x39 frames).
 * cmd 2: GTE rot+trans from D_800BCFA4[0], then per state a func_800D2B58 /
 * func_800D1384 / func_800CEE20 draw with an alternating palette (odd ticks
 * D_8018F014, even ticks D_8018F018).
 * inline GTE/asm: Psy-Q inline_c.h-shaped GTE (COP2) SetRot/SetTrans macros; count
 * separately from plain C.
 * Levers: H/SH as array-element macros (MEM_IN_STRUCT, so the D_800F32D0/D_800F33E0
 * loads hoist above the a1 stores) except the three w-adds in cmd-1 states 1/2, which
 * use the scalar HS() form so D_800942EC is not hoisted above them; ternary for the
 * +0xB6 sound handle; `s2 = t*h/4096; s2 += h;`; `goto end` tail in state 4; palette
 * select written `== 0 ? pb : pa`; block-local divide result in cmd-2 state 3. */
#define gte_SetRotMatrix(r0) __asm__ volatile ( \
    "lw $12,0(%0)\n\tlw $13,4(%0)\n\tctc2 $12,$0\n\tctc2 $13,$1\n\t" \
    "lw $12,8(%0)\n\tlw $13,12(%0)\n\tlw $14,16(%0)\n\t" \
    "ctc2 $12,$2\n\tctc2 $13,$3\n\tctc2 $14,$4" : : "r"(r0) : "$12", "$13", "$14")
#define gte_SetTransMatrix(r0) __asm__ volatile ( \
    "lw $12,20(%0)\n\tlw $13,24(%0)\n\tctc2 $12,$5\n\tlw $14,28(%0)\n\t" \
    "ctc2 $13,$6\n\tctc2 $14,$7" : : "r"(r0) : "$12", "$13", "$14")

typedef struct { short x, y, z, w; } GteShortVector;
typedef struct { unsigned char c[8]; } B8;
typedef struct { unsigned char c[4]; } B4;
typedef struct { short m[3][3]; int t[3]; } MATRIX;

#define H(o, x) (((unsigned short *)(o))[(x) / 2])
#define SH(o, x) (((short *)(o))[(x) / 2])
#define HS(o, x) (*(unsigned short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))

extern B8 D_8018F00C;
extern B4 D_8018F014;
extern B4 D_8018F018;
extern int D_80192224;
extern unsigned char *D_800E2368;
extern void *D_800F32D0;
extern void *D_800F33E0;
extern void **D_8009D254;
extern int D_800B0E64;
extern short D_800942EC;
extern int D_800E27EC;
extern unsigned short D_800E11EA;
extern unsigned short D_800E2850[];
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern MATRIX *D_800BCFA4[];
extern struct { short a68, a6A; unsigned short a6C; short a6E; unsigned short a70; short a72, a74, a76, a78; } D_800F3368;

extern int func_8018F800();
extern int func_80071A54();
extern void func_800CE8F0();
extern void func_800CE870();
extern void func_800D1384();
extern int func_800CE560();
extern unsigned short *func_800CE610();
extern int func_800C6B90();
extern int func_800D3FD8();
extern int func_8006DF50();
extern void func_800866A4();
extern int func_80077CF4();
extern int func_80077DC4();
extern int func_80077AA4();
extern void func_800CFB7C(void *, short, void *);
extern void func_800CFAA8();
extern void func_800CFD50(void *, void *, unsigned short);
extern void func_800783E4();
extern void func_800D2B58();
extern void func_800CEE20();

#define TEX(w) \
    w = D_800E1204[D_800F3368.a6C]; \
    if (D_800F3368.a6C == 4 && D_800F3428 != 0) \
        w += 4;

#define OWNER_POKE() \
    if (D_800E2368[0xD] != 0) { \
        o = D_800F32D0; \
        if ((**(int **)P(o, 8) & 0x3F000000) == 0x01000000) { \
            W(*D_8009D254, 0x4C) |= 0x4000; \
            **(int **)P(o, 8) = (**(int **)P(o, 8) & 0xC0FFFFFF) | 0x3D000000; \
            **(int **)P(o, 8) |= 0x80000000; \
        } \
    }

#define SPAWN() \
    e = func_800CE610(P(D_800F33E0, 8)); \
    if (e != 0) { \
        e[0] = H(a1, 0); \
        e[1] = H(a1, 2); \
        e[2] = H(a1, 4); \
        e[4] = 0; \
        e[5] = 0; \
    }

int func_8018FA64(int mode, unsigned char *a1, int *a2)
{
    B8 t8;
    GteShortVector v;
    GteShortVector w;
    GteShortVector r;
    GteShortVector x;
    B4 b50;
    B4 b58;
    B8 *pt;
    B4 *pa;
    B4 *pb;
    B4 *sel;
    unsigned short *e;
    void *o;
    unsigned char *st;
    int s2;
    int k;

    t8 = D_8018F00C;
    b50 = D_8018F014;
    b58 = D_8018F018;
    pt = &t8;
    pa = &b50;
    pb = &b58;
    switch (mode) {
    case 0:
        D_80192224 = 2 - D_80192224;
        r.y = 0;
        r.x = (D_80192224 - 1) * 350;
        r.z = -(func_80071A54() & 0xFF) - 400;
        func_800CE8F0(P(D_800F32D0, 8), 4, &r, a1);
        H(a1, 2) = D_800942EC;
        {
            int q = func_80071A54();
            int y = H(a1, 0);

            H(a1, 0) = (y - 16) + (q & 0x1F);
        }
        {
            int q = func_80071A54();
            int y = H(a1, 4);

            SH(a1, 8) = 0;
            H(a1, 4) = (y - 16) + (q & 0x1F);
        }
        H(a1, 0xA) = func_80071A54() & 0xFFF;
        SH(a1, 0xC) = 0;
        func_800CE8F0(P(D_800F32D0, 8), 4, pt, &v);
        func_800D1384(&v, a1, 0x3F0, pa, pa, 0x8C, a1 + 0x10, 1);
        if (D_800E2368[13] != 0) {
            void *obj = P(D_800F32D0, 8);

            if (obj != 0) {
                void *inner = *(void **)obj;

                if (inner != 0) {
                    st = P(inner, 0x18);
                    if (*st == 1)
                        *st = 2;
                }
            }
        }
        SH(a1, 0xB0) = 0;
        SH(a1, 0xB2) = 0;
        SH(a1, 0xB4) = 8;
        return func_800CE560(P(D_800F33E0, 8), 0xC, 0x10, func_8018F800);
    case 1:
        func_800CE870(D_8009D254, 1, &v);
        v.y = D_800942EC;
        switch (SH(a1, 0xB0)) {
        case 0:
            {
                short n = H(a1, 0xB2) + 1;

                SH(a1, 0xB2) = n;
                if (n == 2) {
                    int *pbz = &D_800B0E64;

                    SH(a1, 0xB6) = (*pbz != 0) ? func_8006DF50(*pbz, 0x598, func_800D3FD8(), 0x80, 0x7F) : 0;
                }
            }
            if (SH(a1, 0xB2) < 4)
                return 0;
            SH(a1, 0xB0) = 1;
            SH(a1, 0xB2) = 0;
            break;
        case 1:
            {
                short n = H(a1, 0xB2) + 1;
                int t, h;

                SH(a1, 0xB2) = n;
                t = func_80077CF4((n << 11) / 12);
                h = a2[0] / 2;
                s2 = t * h / 4096;
                s2 += h;
            }
            func_800CFB7C(a1 + 8, s2, &w);
            HS(a1, 0) += w.x;
            HS(a1, 2) += w.y;
            HS(a1, 4) += w.z;
            H(a1, 2) = D_800942EC;
            SPAWN();
            if (func_800C6B90(a1, 0x32) != 0) {
                OWNER_POKE();
            }
            if (SH(a1, 0xB2) < 0xC)
                return 0;
            SH(a1, 0xB0) = 2;
            if (--SH(a1, 0xB4) <= 0)
                SH(a1, 0xB0) = 3;
            SH(a1, 0xB2) = 0;
            break;
        case 2:
            {
                short n = H(a1, 0xB2) + 1;

                s2 = n * 650;
                SH(a1, 0xB2) = n;
            }
            func_800CFAA8(a1, &v, &x);
            x.x = 0;
            func_800CFD50(&x, a1 + 8, s2);
            func_800CFB7C(a1 + 8, a2[0] / 2, &w);
            HS(a1, 0) += w.x;
            HS(a1, 2) += w.y;
            HS(a1, 4) += w.z;
            H(a1, 2) = D_800942EC;
            SPAWN();
            if (func_800C6B90(a1, 0x32) != 0) {
                OWNER_POKE();
            }
            if (SH(a1, 0xB2) < 4)
                return 0;
            SH(a1, 0xB0) = 1;
            if (--SH(a1, 0xB4) <= 0)
                SH(a1, 0xB0) = 3;
            SH(a1, 0xB2) = 0;
            break;
        case 3:
            {
                short n = H(a1, 0xB2) + 1;

                SH(a1, 0xB2) = n;
                if (n < 8)
                    return 0;
            }
            SH(a1, 0xB0) = 4;
            SH(a1, 0xB2) = 0;
            if (SH(a1, 0xB6) != -1)
                func_800866A4(SH(a1, 0xB6), 0);
            break;
        case 4:
            {
                short n = H(a1, 0xB2) + 1;

                SH(a1, 0xB2) = n;
                if (n < 0x39)
                    goto end;
            }
            return 2;
        }
        break;
    case 2:
        func_800CE8F0(P(D_800F32D0, 8), 4, pt, &v);
        if ((D_800E27EC & 1) == 0)
            sel = pb;
        else
            sel = pa;
        D_800F3368.a68 = 0x20;
        D_800F3368.a6A = 2;
        D_800F3368.a76 = 0x20;
        D_800F3368.a78 = 0x20;
        D_800F3368.a70 = D_800E2850[D_800E11EA];
        D_800F3368.a6C = 3;
        D_800F3368.a6E = 0;
        D_800F3368.a72 = 0;
        D_800F3368.a74 = 0xC;
        gte_SetRotMatrix(D_800BCFA4[0]);
        gte_SetTransMatrix(D_800BCFA4[0]);
        switch (SH(a1, 0xB0)) {
        case 0:
            s2 = SH(a1, 0xB2) << 10;
            func_800783E4(&v, a1, 0x1000 - s2, s2, &r);
            func_800D2B58(&v, &r, sel, sel, 0x80, 0, 1);
            TEX(k);
            func_800CEE20(&v, 0, 0x1000, 0x1000, D_800F3368.a6A * (H(a1, 0xB2) & 1) + 0x40,
                          func_80077AA4(0x20, k) & 0xFFFF, 1, 0x80, 0);
            break;
        case 1:
            func_800D2B58(&v, a1, sel, sel, 0x8C, 0x8C, 1);
            func_800D1384(&v, a1, 8, sel, &b50, 0x8C, a1 + 0x10, 1);
            TEX(k);
            func_800CEE20(&v, 0, 0x1000, 0x1000, D_800F3368.a6A * (H(a1, 0xB2) & 1) + 0x40,
                          func_80077AA4(0x20, k) & 0xFFFF, 1, 0x80, 0);
            break;
        case 2:
            func_800D2B58(&v, a1, sel, sel, 0x60, 0x60, 1);
            func_800D1384(&v, a1, 8, sel, &b50, 0x8C, a1 + 0x10, 1);
            TEX(k);
            func_800CEE20(&v, 0, 0x1000, 0x1000, D_800F3368.a6A * (H(a1, 0xB2) & 1) + 0x40,
                          func_80077AA4(0x20, k) & 0xFFFF, 1, 0x80, 0);
            break;
        case 3: {
            int z = func_80077DC4(SH(a1, 0xB2) << 7) / 32;

            func_800D2B58(&v, a1, sel, sel, z, z, 1);
            func_800D1384(&v, a1, 8, sel, &b50, 0x8C, a1 + 0x10, 1);
            TEX(k);
            func_800CEE20(&v, 0, 0x1000, 0x1000, D_800F3368.a6A * (H(a1, 0xB2) & 1) + 0x40,
                          func_80077AA4(0x20, k) & 0xFFFF, 1, z, 0);
            break;
        }
        case 4:
            break;
        }
        break;
    }
end:
    return 0;
}
