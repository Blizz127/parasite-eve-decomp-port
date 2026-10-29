/* room_m0318i — func_80194AAC, blob offset 0x5AC4, 0x6E4 bytes. NON_MATCHING
 * (behaviour C, proven by the e21 differential; not a byte match).
 * Effect callback, a0 = mode, a1 = effect record (state +8, frame +0xA,
 * strength +0xC), a2 = key pair for mode 0.
 *   mode 0: clear a1 +8/+0xA/+0xC, find the live object in the D_8009D20C
 *           list whose +0xD / +0xC bytes equal a2 words 0 / 4 (func_800CE870
 *           attach, else func_80071A74 on D_8018F1CC), func_800D3F64(0x5F5,
 *           func_800D3FD8()), func_800D3F64(0x5F6, 0x80), then
 *           return func_800CE560(D_800F33E0->+8, 0x14, 0x28, func_801944E8).
 *   mode 1: state 0 (16 frames): frame++, strength = func_80077CF4(frame<<6)/32,
 *           then state 1. State 1 (frame++, 0x20 frames): while frame < 0x19
 *           spawn a +-0x20 burst particle; odd frames a +-0x80 / +-16 spray
 *           particle; even frames a +-0x100 ring particle (state 2); then
 *           state 2. State 2 (frame++): strength = func_80077DC4(
 *           (frame<<10)/24)/32, returns 1 from frame 0x18 on.
 *   mode 2: load the D_800BCFA4 rotation/translation into the GTE, set the
 *           D_800F3368 draw block, and when strength != 0 draw func_800D004C
 *           (0x190 ring, sp+0x30 / sp+0x38 colour by D_800E27EC & 1) and
 *           func_800D0728 (0x1F4 / 0x2BC ring, strength / 2); then
 *           D_800F3372 = 0, D_800F3374 = 4. Returns 0.
 * Unknown modes / states return 0. The two colour seeds (lwl/lwr of
 * D_8018F200 / D_8018F204) live at sp+0x30 / sp+0x38 and are passed by
 * address to the ring draws.
 * inline GTE/asm: Psy-Q inline_c.h-shaped GTE (COP2) SetRot/SetTrans macros. */
#define gte_SetRotMatrix(r0) __asm__ volatile ( \
    "lw $12,0(%0)\n\tlw $13,4(%0)\n\tctc2 $12,$0\n\tctc2 $13,$1\n\t" \
    "lw $12,8(%0)\n\tlw $13,12(%0)\n\tlw $14,16(%0)\n\t" \
    "ctc2 $12,$2\n\tctc2 $13,$3\n\tctc2 $14,$4" : : "r"(r0) : "$12", "$13", "$14")
#define gte_SetTransMatrix(r0) __asm__ volatile ( \
    "lw $12,20(%0)\n\tlw $13,24(%0)\n\tctc2 $12,$5\n\tlw $14,28(%0)\n\t" \
    "ctc2 $13,$6\n\tctc2 $14,$7" : : "r"(r0) : "$12", "$13", "$14")

typedef struct { unsigned char c[4]; } B4;
typedef struct { short m[3][3]; int t[3]; } MATRIX;

#define H(o, x) (*(unsigned short *)((char *)(o) + (x)))
#define SH(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))

extern B4 D_8018F200;
extern B4 D_8018F204;
extern char D_8018F1CC[];
extern unsigned char *D_8009D20C;
extern unsigned char *D_8009D254;
extern unsigned char *D_800F33E0;
extern MATRIX *D_800BCFA4[];
extern int D_800E27EC;
extern unsigned short D_800E11EA;
extern unsigned short D_800E2850[];
extern short D_800F3368;
extern short D_800F336A;
extern short D_800F336C;
extern short D_800F336E;
extern short D_800F3370;
extern short D_800F3372;
extern short D_800F3374;
extern short D_800F3376;
extern short D_800F3378;
extern unsigned char func_801944E8[];

extern int func_800D3FD8();
extern void func_800D3F64();
extern int func_800CE560();
extern void func_800CE870();
extern unsigned char *func_800CE610();
extern int func_80077CF4();
extern int func_80077DC4();
extern int func_80071A54();
extern int func_80071A74();
extern void func_800D004C();
extern void func_800D0728();

int func_80194AAC(int mode, unsigned char *a1, unsigned char *a2)
{
    /* retail frame: colour seeds at sp+0x30 and sp+0x38 (4 bytes each) */
    struct { B4 b0; unsigned short pad0[2]; B4 b1; unsigned short pad1[2]; } f;
    unsigned char *p, *s, *sentinel;
    int key0, key1, v, c, again;
    short fr;

    f.b0 = D_8018F200;
    f.b1 = D_8018F204;
    switch (mode) {
    case 0:
        SH(a1, 8) = 0;
        SH(a1, 0xA) = 0;
        SH(a1, 0xC) = 0;
        p = D_8009D20C;
        key0 = W(a2, 0);
        key1 = W(a2, 4);
        if (p != 0) {
            sentinel = D_8009D254;
            do {
                if (p != sentinel && W(p, 0) != 0 && W(*(unsigned char **)p, 0x10) > 0
                    && p[0xD] == key0 && p[0xC] == key1) {
                    func_800CE870(p, 0, a1);
                    goto found;
                }
                p = *(unsigned char **)(p + 4);
            } while (p != 0);
        }
        func_80071A74(D_8018F1CC, key0, key1);
    found:
        v = func_800D3FD8();
        func_800D3F64(0x5F5, v);
        func_800D3F64(0x5F6, 0x80);
        return func_800CE560(W(D_800F33E0, 8), 0x14, 0x28, func_801944E8);
    case 1:
        switch (SH(a1, 8)) {
        case 0:
            H(a1, 0xA) = H(a1, 0xA) + 1;
            v = func_80077CF4(SH(a1, 0xA) << 6);
            SH(a1, 0xC) = v / 32;
            if (SH(a1, 0xA) >= 0x10) {
                SH(a1, 8) = 1;
                SH(a1, 0xA) = 0;
            }
            return 0;
        case 1:
            H(a1, 0xA) = H(a1, 0xA) + 1;
            if (SH(a1, 0xA) < 0x19) {
                s = func_800CE610(W(D_800F33E0, 8));
                if (s != 0) {
                    H(s, 0) = H(a1, 0);
                    H(s, 2) = H(a1, 2);
                    H(s, 4) = H(a1, 4);
                    H(s, 0) = H(s, 0) - 0x20 + (func_80071A54() & 0x3F);
                    H(s, 2) = H(s, 2) - 0x20 + (func_80071A54() & 0x3F);
                    H(s, 4) = H(s, 4) - 0x20 + (func_80071A54() & 0x3F);
                    SH(s, 6) = (func_80071A54() & 0x1FF) + 0x200;
                    SH(s, 8) = func_80071A54();
                    SH(s, 0xA) = func_80071A54();
                    SH(s, 0xC) = func_80071A54();
                    SH(s, 0x10) = 1;
                    SH(s, 0x12) = 0;
                }
            }
            again = (H(a1, 0xA) & 1) == 0;
            if (!again) {
                s = func_800CE610(W(D_800F33E0, 8));
                if (s != 0) {
                    H(s, 0) = H(a1, 0);
                    H(s, 2) = H(a1, 2);
                    H(s, 4) = H(a1, 4);
                    H(s, 2) = H(s, 2) + 0x100 - (func_80071A54() & 0x1FF);
                    H(s, 0) = H(s, 0) - 0x80 + (func_80071A54() & 0xFF);
                    H(s, 4) = H(s, 4) - 0x80 + (func_80071A54() & 0xFF);
                    c = func_80071A54();
                    SH(s, 8) = c % 32 - 0x10;
                    c = func_80071A54();
                    SH(s, 0xA) = c % 8 - 4;
                    c = func_80071A54();
                    SH(s, 0xC) = c % 32 - 0x10;
                    SH(s, 0x10) = 0;
                    SH(s, 0x12) = 0;
                }
                again = (H(a1, 0xA) & 1) == 0;
            }
            if (again) {
                s = func_800CE610(W(D_800F33E0, 8));
                if (s != 0) {
                    H(s, 0) = H(a1, 0);
                    H(s, 2) = H(a1, 2);
                    H(s, 4) = H(a1, 4);
                    H(s, 0) = H(s, 0) - 0x100 + (func_80071A54() & 0x1FF);
                    H(s, 2) = H(s, 2) - 0x100 + (func_80071A54() & 0x1FF);
                    c = func_80071A54() & 0x1FF;
                    SH(s, 0x10) = 2;
                    SH(s, 0x12) = 0;
                    H(s, 4) = H(s, 4) - 0x100 + c;
                }
            }
            if (SH(a1, 0xA) >= 0x20) {
                SH(a1, 8) = 2;
                SH(a1, 0xA) = 0;
            }
            return 0;
        case 2:
            H(a1, 0xA) = H(a1, 0xA) + 1;
            fr = SH(a1, 0xA);
            v = func_80077DC4((fr << 10) / 24);
            SH(a1, 0xC) = v / 32;
            if (SH(a1, 0xA) >= 0x18)
                return 1;
            return 0;
        }
        return 0;
    case 2:
        gte_SetRotMatrix(D_800BCFA4[0]);
        gte_SetTransMatrix(D_800BCFA4[0]);
        D_800F3368 = 0x20;
        D_800F336A = 2;
        D_800F3376 = 0x20;
        D_800F3378 = 0x20;
        D_800F336C = 3;
        D_800F336E = 0;
        D_800F3372 = 0;
        D_800F3370 = D_800E2850[D_800E11EA];
        c = SH(a1, 0xC);
        if (c != 0) {
            if (D_800E27EC & 1)
                p = (unsigned char *)&f.b0;
            else
                p = (unsigned char *)&f.b1;
            D_800F3374 = 0x28;
            func_800D004C(a1, 0x190, 0x190, 0x10, 0, 0x1000, 0x1000, p, 0, c, 1);
            func_800D0728(a1, 0x1F4, 0x2BC, 0x18, 0, 0x1000, 0x1000, 0, &f.b0, c / 2, 3);
        }
        D_800F3372 = 0;
        D_800F3374 = 4;
        return 0;
    }
    return 0;
}
