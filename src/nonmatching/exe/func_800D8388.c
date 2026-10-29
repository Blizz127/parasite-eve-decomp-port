/* func_800D8388 — EXE, VRAM 0x800D8388, file offset 0xC8B88, 0x304 bytes. NON_MATCHING
 * (behaviour C, proven by the e21 differential; not a byte match).
 * Orbiting-effect task handler (the effect-descriptor callback with the
 * func_800D7FBC / func_800D7E78 sub-handlers), a0 = mode, a1 = effect state:
 *   mode 0 (init):  a1[0] = rand; allocates 12 slots stride 0x10 from the
 *                   task list (func_800CE560, callback func_800D7FBC) and
 *                   registers the 8-entry list a1+4 (func_800CE5AC, callback
 *                   func_800D7E78); returns the two counts summed.
 *   mode 1 (step):  every third frame below D_800E27EC == 0x2C spawns one
 *                   glint (func_800CE610 slot: pos 0, speed 0x2BC, x = a1[0],
 *                   y from the object position + random(0x1FF) - 0x28F),
 *                   a1[0] -= 0x555; below 0x46 retires the a1[1] list and
 *                   saves it in D_800E21F4; returns 1 at/after 0x46.
 *   mode 2 (draw):  loads the first D_800BCFA4 matrix into the GTE (rotation
 *                   and translation) and sets the two D_800F3368 draw blocks
 *                   (0x10 sprite id 1, 0x20 sprite id 2) around func_800CEDA8 /
 *                   func_800CE78C / func_800CE870 calls.
 * inline GTE/asm: Psy-Q inline_c.h-shaped GTE (COP2) SetRot/SetTrans macros. */
#define gte_SetRotMatrix(r0) __asm__ volatile ( \
    "lw $12,0(%0)\n\tlw $13,4(%0)\n\tctc2 $12,$0\n\tctc2 $13,$1\n\t" \
    "lw $12,8(%0)\n\tlw $13,12(%0)\n\tlw $14,16(%0)\n\t" \
    "ctc2 $12,$2\n\tctc2 $13,$3\n\tctc2 $14,$4" : : "r"(r0) : "$12", "$13", "$14")
#define gte_SetTransMatrix(r0) __asm__ volatile ( \
    "lw $12,20(%0)\n\tlw $13,24(%0)\n\tctc2 $12,$5\n\tlw $14,28(%0)\n\t" \
    "ctc2 $13,$6\n\tctc2 $14,$7" : : "r"(r0) : "$12", "$13", "$14")

typedef struct { short m[3][3]; int t[3]; } MATRIX;

#define W(o, x) (*(unsigned int *)((unsigned char *)(o) + (x)))
#define H(o, x) (*(unsigned short *)((unsigned char *)(o) + (x)))
#define SH(o, x) (*(short *)((unsigned char *)(o) + (x)))

extern unsigned char *D_800F33E0;
extern unsigned char *D_8009D254;
extern int D_800E27EC;
extern unsigned char *D_800E21F4;
extern unsigned char D_800E21EC[];
extern unsigned short D_800E11E6;
extern unsigned short D_800E11F6;
extern unsigned short D_800E2850[];
extern MATRIX *D_800BCFA4[];
extern struct { short a68, a6A, a6C, a6E, a70, a72, a74, a76, a78; } D_800F3368;

extern unsigned char func_800D7FBC[];
extern unsigned char func_800D7E78[];
extern int func_80071A54();
extern int func_800CE560();
extern int func_800CE5AC();
extern unsigned char *func_800CE610();
extern void func_800CE688();
extern void func_800CE78C();
extern void func_800CE870();
extern void func_800CEDA8();

int func_800D8388(int mode, unsigned char *a1)
{
    unsigned char *p;
    int r, v;
    short buf[4];

    switch (mode) {
    case 0:
        W(a1, 0) = func_80071A54();
        r = func_800CE560(W(D_800F33E0, 8), 0x10, 0xC, func_800D7FBC);
        v = func_800CE5AC(a1 + 4, r, 8, 0x20, func_800D7E78);
        return r + v;
    case 1:
        if (D_800E27EC < 0x2C && D_800E27EC == D_800E27EC / 3 * 3) {
            p = func_800CE610(W(D_800F33E0, 8));
            if (p) {
                SH(p, 0) = 0;
                SH(p, 4) = 0;
                SH(p, 0xC) = 0x2BC;
                SH(p, 2) = W(a1, 0);
                func_800CE870(D_8009D254, 1, buf);
                SH(p, 8) = (unsigned short)buf[1] + (func_80071A54() & 0x1FF) - 0x28F;
                W(a1, 0) = W(a1, 0) - 0x555;
            }
        }
        if (D_800E27EC < 0x46) {
            func_800CE688(W(a1, 4));
            D_800E21F4 = (unsigned char *)W(a1, 4);
            return 0;
        }
        return 1;
    case 2:
        gte_SetRotMatrix(D_800BCFA4[0]);
        gte_SetTransMatrix(D_800BCFA4[0]);
        D_800F3368.a68 = 0x10;
        D_800F3368.a6A = 1;
        D_800F3368.a76 = 0x10;
        D_800F3368.a78 = 0x10;
        D_800F3368.a6C = 1;
        D_800F3368.a70 = D_800E2850[D_800E11E6];
        func_800CEDA8(1);
        D_800F3368.a6E = 0;
        D_800F3368.a72 = 0;
        D_800F3368.a74 = 8;
        func_800CE78C(W(a1, 4));
        func_800CE870(D_8009D254, 1, D_800E21EC);
        D_800F3368.a68 = 0x20;
        D_800F3368.a6A = 2;
        D_800F3368.a76 = 0x20;
        D_800F3368.a78 = 0x20;
        D_800F3368.a6C = 1;
        D_800F3368.a70 = D_800E2850[D_800E11F6];
        func_800CEDA8(1);
        D_800F3368.a6E = 1;
        D_800F3368.a72 = 0;
        return 0;
    }
    return 0;
}
