/* room_m0231i (PE.IMG room m0231i chunk 2, VRAM 0x8018EFE8)
 * func_80192F6C — blob offset 0x3F84, 0x6E4 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0203i func_8019281C (symbols re-targeted).
 * Shared room effect callback (cmd 0/1/2 shape, as room_m0104i func_8018F3A4).
 * Twin (masked-body port): room_m0231i func_80192F6C (D_8018F1E0/D_8018F1E8/
 * D_80194844/func_80192C8C), also LINK_EXACT.
 * Levers (lane s441):
 *   - cmd 2 state switch needs explicit empty `case 1:` and `case 2:` (retail
 *     keeps the beq-1/slti/bnez tree; without case 2 cc1 collapses it);
 *   - cmd 1 state 2 is `return 2` (retail jumps to the epilogue with v0 = 2);
 *   - `SH(ob,0) -= 0x80 - (rand() & 0xFF)` gives retail (ob - 0x80) + r order
 *     (`ob[0] += (r & 0xFF) - 0x80` reassociates; unsigned form does too);
 *   - the state/timer header accessed as a struct (Hdr) in cmd 0 and in the
 *     state-1 timer++, so the stores schedule past fixed-address loads
 *     (gcc 2.7 MEM_IN_STRUCT alias rule);
 *   - tail block: `a70 = pal` first, so cse picks &D_800F3370 as the base.
 * inline GTE/asm: Psy-Q inline_c.h-shaped GTE (COP2) SetRot/SetTrans macros.
 * Hand-written in this repo by lane-s441 (khasinski's vendored tree has this body as asm only). */
#define gte_SetRotMatrix(r0) __asm__ volatile ( \
    "lw $12,0(%0)\n\tlw $13,4(%0)\n\tctc2 $12,$0\n\tctc2 $13,$1\n\t" \
    "lw $12,8(%0)\n\tlw $13,12(%0)\n\tlw $14,16(%0)\n\t" \
    "ctc2 $12,$2\n\tctc2 $13,$3\n\tctc2 $14,$4" : : "r"(r0) : "$12", "$13", "$14")
#define gte_SetTransMatrix(r0) __asm__ volatile ( \
    "lw $12,20(%0)\n\tlw $13,24(%0)\n\tctc2 $12,$5\n\tlw $14,28(%0)\n\t" \
    "ctc2 $13,$6\n\tctc2 $14,$7" : : "r"(r0) : "$12", "$13", "$14")

typedef struct { short x, y, z, w; } GteShortVector;
typedef struct { short m[3][3]; int t[3]; } MATRIX;
typedef struct { unsigned char a, b, c, d; } S4;
typedef struct { unsigned char b[8]; } Seed8;
typedef struct { short state; unsigned short timer; } Hdr;

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define H(o, x) (*(unsigned short *)((char *)(o) + (x)))
#define SH(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))

extern GteShortVector D_8018F1E0;
extern S4 D_8018F1E8;
extern unsigned char D_80194844[];
extern void func_80192C8C();
extern unsigned char *D_800E2368;
extern void *D_800F32D0;
extern void *D_800F33E0;
extern void **D_8009D254;
extern unsigned short D_800E11EC;
extern unsigned short D_800E2850[];
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern MATRIX *D_800BCFA4[];
extern void func_800CE870();
extern int func_800D3FD8();
extern void func_800D3F64();
extern int func_800CE560();
extern void func_800CE8F0();
extern unsigned short *func_800CE610();
extern int func_80077DC4();
extern int func_80077CF4();
extern int func_80071A54();
extern int func_800C6B90();
extern int func_80077AA4();
extern void func_800CEE20();
extern void func_800CF3AC();
extern void func_800D004C();
extern struct { short a68, a6A; unsigned short a6C; short a6E; unsigned short a70; short a72, a74, a76, a78; } D_800F3368;

int func_80192F6C(int mode, unsigned char *a1)
{
    GteShortVector eff = D_8018F1E0;
    GteShortVector pos;
    Seed8 seed;
    S4 t = D_8018F1E8;
    unsigned short *ob;
    void **pp, *r;
    unsigned char *c;
    void *q;
    int *p;
    int s2, s0, s1, w, k;
    MATRIX **pm;
    register GteShortVector *e asm("$6");

    e = &eff;
    switch (mode) {
    case 0:
        func_800CE870(D_8009D254, 0, a1 + 4);
        func_800D3F64(0x59E, func_800D3FD8());
        ((Hdr *)a1)->state = 0;
        ((Hdr *)a1)->timer = 0;
        if (D_800E2368[0xD] != 0) {
            pp = P(D_800F32D0, 8);
            if (pp != 0) {
                r = *pp;
                if (r != 0) {
                    c = P(r, 0x18);
                    if (*c == 1) *c = 2;
                }
            }
        }
        return func_800CE560(P(D_800F33E0, 8), 0x14, 0x18, func_80192C8C);
    case 1:
        switch (SH(a1, 0)) {
        case 0:
            H(a1, 2)++;
            if (SH(a1, 2) < 0x14) return 0;
            SH(a1, 0) = 1;
            SH(a1, 2) = 0;
            break;
        case 1:
            ((Hdr *)a1)->timer++;
            ob = func_800CE610(P(D_800F33E0, 8));
            if (ob) {
                ob[0] = H(a1, 4);
                ob[1] = H(a1, 6);
                ob[2] = H(a1, 8);
                SH(ob, 0) -= 0x80 - (func_80071A54() & 0xFF);
                SH(ob, 4) -= 0x80 - (func_80071A54() & 0xFF);
                SH(ob, 2) -= 0x100 - (func_80071A54() & 0x1FF);
                ob[4] = func_80071A54() % 32 - 0x10;
                ob[5] = func_80071A54() % 32 - 0x10;
                ob[6] = func_80071A54() % 32 - 0x10;
                ob[8] = 0;
                ob[9] = 0;
            }
            if (func_800C6B90(a1 + 4, 0xB4) && D_800E2368[0xD] != 0
                && (**(int **)P(q = D_800F32D0, 8) & 0x3F000000) == 0x1000000) {
                W(*D_8009D254, 0x4C) |= 0x4000;
                p = *(int **)P(q, 8);
                *p = (*p & 0xC0FFFFFF) | 0x21000000;
                p = *(int **)P(q, 8);
                *p |= 0x80000000;
            }
            if (SH(a1, 2) < 0x20) return 0;
            SH(a1, 0) = 2;
            break;
        case 2:
            return 2;
        }
        break;
    case 2:
        D_800F3368.a68 = 0x20;
        D_800F3368.a6A = 2;
        D_800F3368.a76 = 0x20;
        D_800F3368.a78 = 0x20;
        D_800F3368.a76 = 0x100;
        D_800F3368.a78 = 0x20;
        D_800F3368.a70 = D_800E2850[D_800E11EC];
        D_800F3368.a6C = 4;
        D_800F3368.a6E = 0;
        D_800F3368.a72 = 0;
        D_800F3368.a74 = 0x20;
        switch (SH(a1, 0)) {
        case 0:
            func_800CE8F0(P(D_800F32D0, 8), 0x10, e, &pos);
            s2 = func_80077CF4((SH(a1, 2) << 10) / 20);
            s0 = func_80077DC4((SH(a1, 2) << 10) / 20) / 2;
            func_800CF3AC(D_80194844, &seed, SH(a1, 2));
            k = D_800F3368.a6C;
            w = D_800E1204[k];
            if (k == 4 && D_800F3428 != 0) w += 4;
            func_800CEE20(&pos, 0, s2, s0, 0, func_80077AA4(0, w) & 0xFFFF, 1, 0x80, &seed);
            pm = D_800BCFA4;
            gte_SetRotMatrix(pm[0]);
            gte_SetTransMatrix(pm[0]);
            s1 = func_80077DC4((SH(a1, 2) << 10) / 20) / 64;
            func_800D004C(&pos, 0x44C, 0x44C, 0x10, 0, 0x1000, 0x1000, &t, 0, s1, 1);
            func_800D004C(&pos, 0x1F4, 0x1F4, 8, 0, 0x1000, 0x1000, &t, 0, s1, 1);
            break;
        case 1:
            break;
        case 2:
            break;
        }
        {
            unsigned short pal = D_800E2850[D_800E11EC];
            D_800F3368.a70 = pal;
            D_800F3368.a6C = 4;
            D_800F3368.a6E = 0;
            D_800F3368.a76 = D_800F3368.a68;
            D_800F3368.a78 = D_800F3368.a68;
            D_800F3368.a72 = 0;
            D_800F3368.a74 = 0x10;
        }
        break;
    }
    return 0;
}
