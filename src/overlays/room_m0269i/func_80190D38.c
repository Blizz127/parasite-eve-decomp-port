/* room_m0269i — func_80190D38, blob offset 0x1D50, 0x67C bytes. Profile era_o2_g0.
 * Emitter callback registered by func_801913B4. cmd 0 zeroes a1+8/+0xA, attaches
 * a1 to D_8009D254, plays sound 0x596 and arms func_8019070C (0x14, 0x28).
 * cmd 1 (ticks < 4) spawns 3 state-1 particles with +/-35 velocity, 5 state-0
 * particles and 4 state-2 particles; done at tick 0x1C. cmd 2 draws a
 * func_800CEE20 / func_800D004C / func_800D0728 flash for ticks < 0x15, then
 * stores a1's position into D_80192334 and seeds the D_800F3368 block.
 * inline GTE/asm: Psy-Q inline_c.h-shaped GTE (COP2) SetRot/SetTrans macros; count
 * separately from plain C.
 * Levers: cmd 0 uses `void *d = D_8009D254;` and a `register unsigned char *p = a1;`
 * block copy (unpinned `register` survives CSE and is tied to $a2, so the two zero
 * stores go through the argument register as in retail); loop-1 writes e[6] before
 * the e[8]/e[9] state words. */
#define gte_SetRotMatrix(r0) __asm__ volatile ( \
    "lw $12,0(%0)\n\tlw $13,4(%0)\n\tctc2 $12,$0\n\tctc2 $13,$1\n\t" \
    "lw $12,8(%0)\n\tlw $13,12(%0)\n\tlw $14,16(%0)\n\t" \
    "ctc2 $12,$2\n\tctc2 $13,$3\n\tctc2 $14,$4" : : "r"(r0) : "$12", "$13", "$14")
#define gte_SetTransMatrix(r0) __asm__ volatile ( \
    "lw $12,20(%0)\n\tlw $13,24(%0)\n\tctc2 $12,$5\n\tlw $14,28(%0)\n\t" \
    "ctc2 $13,$6\n\tctc2 $14,$7" : : "r"(r0) : "$12", "$13", "$14")

typedef struct { short x, y, z, w; } GteShortVector;
typedef struct { unsigned char c[4]; } B4;
typedef struct { short m[3][3]; int t[3]; } MATRIX;

#define H(o, x) (*(unsigned short *)((char *)(o) + (x)))
#define SH(o, x) (*(short *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))

extern B4 D_8018F04C;
extern GteShortVector D_80192334;
extern void *D_8009D254;
extern void *D_800F33E0;
extern int D_800E27EC;
extern unsigned short D_800E11EA;
extern unsigned short D_800E2850[];
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern MATRIX *D_800BCFA4[];
extern struct { short a68, a6A; unsigned short a6C; short a6E; unsigned short a70; short a72, a74, a76, a78; } D_800F3368;

extern int func_8019070C();
extern void func_800CE870();
extern int func_800D3FD8();
extern int func_800D3F64();
extern int func_800CE560();
extern unsigned short *func_800CE610();
extern int func_80071A54();
extern int func_80077CF4();
extern int func_80077DC4();
extern int func_80077AA4();
extern void func_800CEE20();
extern void func_800D004C();
extern void func_800D0728();

int func_80190D38(int mode, unsigned char *a1)
{
    B4 buf;
    unsigned short *e;
    int i;
    int s;
    int w;

    buf = D_8018F04C;
    switch (mode) {
    case 0:
        {
            void *d = D_8009D254;
            register unsigned char *p = a1;

            SH(p, 8) = SH(p, 0xA) = 0;
            func_800CE870(d, 0, p);
        }
        func_800D3F64(0x596, func_800D3FD8());
        return func_800CE560(P(D_800F33E0, 8), 0x14, 0x28, func_8019070C);
    case 1:
        if (D_800E27EC < 4) {
            for (i = 0; i < 3; i++) {
                e = func_800CE610(P(D_800F33E0, 8));
                if (e != 0) {
                    e[0] = H(a1, 0);
                    e[1] = H(a1, 2);
                    e[2] = H(a1, 4);
                    e[4] = func_80071A54() % 70 - 35;
                    e[5] = func_80071A54() % 70 - 35;
                    e[6] = func_80071A54() % 70 - 35;
                    e[8] = 1;
                    e[9] = 0;
                }
            }
            for (i = 0; i < 5; i++) {
                e = func_800CE610(P(D_800F33E0, 8));
                if (e != 0) {
                    e[0] = H(a1, 0);
                    e[1] = H(a1, 2);
                    e[2] = H(a1, 4);
                    e[4] = func_80071A54();
                    e[5] = func_80071A54();
                    e[6] = func_80071A54();
                    e[3] = func_80071A54() % 800 + 0x200;
                    e[8] = 0;
                    e[9] = 0;
                }
            }
            for (i = 0; i < 4; i++) {
                e = func_800CE610(P(D_800F33E0, 8));
                if (e != 0) {
                    e[0] = H(a1, 0);
                    e[1] = H(a1, 2);
                    e[2] = H(a1, 4);
                    e[4] = func_80071A54();
                    e[5] = func_80071A54();
                    e[6] = func_80071A54();
                    e[3] = (func_80071A54() & 0x1FF) + 0x100;
                    e[8] = 2;
                    e[9] = 0;
                }
            }
        }
        if (D_800E27EC < 0x1C)
            goto end;
        return 1;
    case 2:
        gte_SetRotMatrix(D_800BCFA4[0]);
        gte_SetTransMatrix(D_800BCFA4[0]);
        D_800F3368.a68 = 0x20;
        D_800F3368.a6A = 2;
        D_800F3368.a76 = 0x20;
        D_800F3368.a78 = 0x20;
        D_800F3368.a70 = D_800E2850[D_800E11EA];
        D_800F3368.a6C = 3;
        D_800F3368.a6E = 0;
        D_800F3368.a72 = 0;
        D_800F3368.a74 = 0x20;
        if (D_800E27EC < 0x15) {
            s = func_80077DC4((D_800E27EC << 10) / 20) / 32;
            w = D_800E1204[D_800F3368.a6C];
            if (D_800F3368.a6C == 4 && D_800F3428 != 0)
                w += 4;
            func_800CEE20(a1, 0, 0x2000, 0x2000, D_800F3368.a6A * (D_800E27EC / 3) + 0x20,
                          func_80077AA4(0x10, w) & 0xFFFF, 1, s, 0);
            func_800D004C(a1, 0x1F4, 0x1F4, 0xC, 0, 0x1000, 0x1000, &buf, 0, s, 1);
            {
                int t = func_80077CF4((D_800E27EC << 10) / 20) / 4 + 0x1000;

                func_800D0728(a1, 0x15E, 0x190, 0x18, 0, t, t, 0, &buf, s / 2, 1);
            }
        }
        D_80192334.x = H(a1, 0);
        D_80192334.y = H(a1, 2);
        D_80192334.z = H(a1, 4);
        D_800F3368.a68 = 0x20;
        D_800F3368.a6A = 2;
        D_800F3368.a76 = 0x20;
        D_800F3368.a78 = 0x20;
        D_800F3368.a70 = D_800E2850[D_800E11EA];
        D_800F3368.a6C = 3;
        D_800F3368.a6E = 0;
        D_800F3368.a72 = 0;
        D_800F3368.a74 = 8;
        break;
    }
end:
    return 0;
}
