/* room_m0437i (PE.IMG room m0437i chunk 2, VRAM 0x8018EFE8)
 * func_80192500 — blob offset 0x3518, 0x874 bytes. Default profile
 * (era_o2_g0, -O2 -G0); LINK_EXACT at the room VMA (2026-10-08, written from
 * the retail asm). Was the m0437i story 0x140/0x300 CPU_BOUNDARY stop.
 *
 * Matching notes (gcc 2.7.2):
 *  - retail's rotation-matrix macro loads the matrix pointer into $8 in the
 *    asm text without telling cc1 (gte_SetRotMatrix8_ld), and the next macro
 *    (gte_SetRotMatrix8) reuses that $8. Because cc1 never sees $8 in use,
 *    it hands $8 to the mfhi/mflo reloads. The PC lowering keeps $8 as a
 *    function-scope carried register (guest_lower.py, carried GPRs);
 *  - the empty tied asms with constant inputs (`"0"(16)`, `"1"(&D_800F3368.f8)`)
 *    make the constants reloads emitted in operand order, ahead of the s
 *    address, and hide 2 from CSE's jump equivalence with `mode`;
 *    gen_decomp_ports rewrites them to plain assignments;
 *  - do {} while (0) loop notes are scheduling barriers. */
#define gte_SetRotMatrix8_ld(r0) __asm__ volatile(                   \
    "lw $8, 0(%0);" "nop;"                                              \
    "lw $12, 0($8);" "lw $13, 4($8);" "ctc2 $12, $0;" "ctc2 $13, $1;"   \
    "lw $12, 8($8);" "lw $13, 12($8);" "lw $14, 16($8);"                \
    "ctc2 $12, $2;" "ctc2 $13, $3;" "ctc2 $14, $4;"                     \
    "lw $12, 20($8);" "lw $13, 24($8);" "ctc2 $12, $5;"                 \
    "lw $14, 28($8);" "ctc2 $13, $6;" "ctc2 $14, $7"                    \
    : : "r"(r0) : "$12", "$13", "$14")
#define gte_SetRotMatrix8() __asm__ volatile(                           \
    "lw $12, 0($8);" "lw $13, 4($8);" "ctc2 $12, $0;" "ctc2 $13, $1;"   \
    "lw $12, 8($8);" "lw $13, 12($8);" "lw $14, 16($8);"                \
    "ctc2 $12, $2;" "ctc2 $13, $3;" "ctc2 $14, $4;"                     \
    "lw $12, 20($8);" "lw $13, 24($8);" "ctc2 $12, $5;"                 \
    "lw $14, 28($8);" "ctc2 $13, $6;" "ctc2 $14, $7"                    \
    : : : "$12", "$13", "$14")
#define gte_SetRotMatrix(r0) __asm__ volatile(                       \
    "lw $12, 0(%0);" "lw $13, 4(%0);" "ctc2 $12, $0;" "ctc2 $13, $1;"   \
    "lw $12, 8(%0);" "lw $13, 12(%0);" "lw $14, 16(%0);"                \
    "ctc2 $12, $2;" "ctc2 $13, $3;" "ctc2 $14, $4;"                     \
    "lw $12, 20(%0);" "lw $13, 24(%0);" "ctc2 $12, $5;"                 \
    "lw $14, 28(%0);" "ctc2 $13, $6;" "ctc2 $14, $7"                    \
    : : "r"(r0) : "$12", "$13", "$14")

typedef struct { short vx, vy, vz, pad; } SVECTOR;
typedef struct { unsigned char r, g, b, cd; } CVECTOR;
extern CVECTOR D_8018F1CC;
extern CVECTOR D_8018F1D0;
extern short D_800942EC[];
typedef struct { short f0; short f2; unsigned short f4; short f6; unsigned short f8; short fa; short fc; short fe; short f10; } SPRP;
extern SPRP D_800F3368;
extern short D_800F336A;
extern unsigned short D_800F336C;
extern short D_800F336E;
extern unsigned short D_800F3370;
extern short D_800F3376;
extern short D_800F3378;
extern unsigned short D_800E11E4[];
extern unsigned short D_800E1204[];
extern unsigned short D_800E2850[];
extern int D_800E27EC;
extern int D_800F3428;
extern int *D_800BCFA4;
extern unsigned char *D_800E2368;
extern unsigned char *D_800F32D0;
extern unsigned char *D_800F33E0;
extern unsigned char **D_8009D254;
extern int D_8009D248;
extern unsigned short D_8009D1CC;
extern int func_80071A54();
extern int func_80077CF4(int);
extern int func_80077DC4(int);
extern unsigned short func_80077AA4(int, int);
extern short *func_800CE610();
extern void func_800CFB7C(void *, int, SVECTOR *);
extern int func_800C6B90(void *, int);
extern int func_8001CAB0(int, int, int, int);
extern void func_800CEE20(void *, void *, int, int, int, int, int, int, void *);
extern void func_800D004C(void *, int, int, int, void *, int, int, void *, void *, int, int);
extern void func_800D2370(void *, void *, int, int, int, int, int, int, int, void *, void *, short, int);

int func_80192500(int mode, short *p, short *arg)
{
    int two;
    int k16, k48;
    SVECTOR v;
    CVECTOR col0;
    CVECTOR col1;
    short *e;
    unsigned char *chan;
    int *w;
    int **matrix_slot;
    int **slot2;
    int palette;
    int value;
    int pal;
    unsigned short *s;
    int a;
    int b;
    int clut;
    int val0;

    col0 = D_8018F1CC;
    col1 = D_8018F1D0;
    switch (mode) {
    case 1:
        switch (p[8]) {
        case 0:
            p[9]++;
            func_800CFB7C(&p[4], arg[0], &v);
            p[0] += v.vx;
            p[1] += v.vy;
            p[2] += v.vz;
            if (!(p[9] & 3)) {
                e = func_800CE610(*(void **)(D_800F33E0 + 8));
                if (e) {
                    e[0] = p[0];
                    e[1] = p[1];
                    e[2] = p[2];
                    e[4] = (func_80071A54() & 0x1F) - 0x10;
                    e[5] = 0;
                    e[6] = (func_80071A54() & 0x1F) - 0x10;
                    e[8] = 1;
                    e[9] = 0;
                }
            }
            func_800CFB7C(&p[4], 180, &v);
            v.vx += p[0];
            v.vy += p[1];
            v.vz += p[2];
            if (func_800C6B90(&v, 50)) {
                if (D_800E2368[0xD]) {
                    chan = D_800F32D0;
                    if ((***(int ***)(chan + 8) & 0x3F000000) == 0x01000000) {
                        *(int *)(*D_8009D254 + 0x4C) |= 0x4000;
                        w = **(int ***)(chan + 8);
                        *w = (*w & 0xC0FFFFFF) | 0x19000000;
                        w = **(int ***)(chan + 8);
                        *w |= 0x80000000;
                    }
                }
                p[0] = v.vx;
                p[1] = v.vy;
                p[2] = v.vz;
                p[8] = 2;
                p[9] = 0;
            }
            if (!func_8001CAB0(p[0] << 16, p[2] << 16, D_8009D248, D_8009D1CC)) return 1;
            break;
        case 1:
            p[9]++;
            p[0] += p[4];
            p[1] += p[5];
            p[2] += p[6];
            p[4] = p[4] * 63 / 64;
            p[6] = p[6] * 63 / 64;
            p[5] -= 2;
            if (p[1] >= D_800942EC[0]) p[5] *= -1;
            if (p[9] >= 8) return 1;
            break;
        case 2:
            p[9]++;
            if (p[9] >= 8) return 1;
            break;
        }
        break;
    case 2:
        matrix_slot = &D_800BCFA4;
        asm volatile("" : "=r"(matrix_slot) : "0"(matrix_slot) : "$2");
        do { gte_SetRotMatrix8_ld(matrix_slot); } while (0);
        switch (p[8]) {
        case 0:
            a = ((D_800E27EC & 1) << 5) + 96;
            gte_SetRotMatrix8();
            val0 = D_800E2850[D_800E11E4[4]];
            palette = D_800E1204[4];
            D_800F336C = 4;
            D_800F336E = 0;
            D_800F3370 = val0;
            if (D_800F336C == 4 && D_800F3428 != 0) palette += 4;
            clut = func_80077AA4(0, palette);
            func_800D2370(p, &p[4], 320, 120, 0, (D_800F336C == 4 && D_800F3428 != 0) ? 96 : 0,
                          64, 16, clut, &col0, &col1, a, 1);
            if (D_800E27EC & 3) break;
            slot2 = &D_800BCFA4;
            asm volatile("" : "=r"(slot2) : "0"(slot2));
            gte_SetRotMatrix8_ld(slot2);
            func_800CFB7C(&p[4], 240, &v);
            v.vx += p[0];
            v.vy += p[1];
            v.vz += p[2];
            func_800D004C(&v, 200, 200, 5, 0, 4096, 4096, &col1, 0, a / 2, 1);
            break;
        case 1:
            b = func_80077DC4(p[9] << 7) * 2 / 3;
            a = func_80077CF4(p[9] << 7) / 64 + 64;
            value = D_800E2850[D_800E11E4[2]];
            pal = D_800E1204[2];
            asm("" : "=r"(k16), "=r"(s) : "0"(16), "1"(&D_800F3368.f8));
            D_800F3368.f4 = 2;
            D_800F3368.f6 = 0;
            D_800F3368.f0 = k16;
            D_800F3368.f2 = 1;
            D_800F3368.fe = k16;
            D_800F3368.f10 = k16;
            *s = value;
            func_800CEE20(p, 0, b, b, ((short *)s)[-3] * (p[9] & 1) + 220,
                          func_80077AA4(48, pal), 1, a, &col0);
            break;
        case 2:
            do { } while (0);
            asm("" : "=r"(two), "=r"(s) : "0"(2), "1"(&D_800F3368.f8));
            value = D_800E2850[D_800E11E4[2]];
            pal = D_800E1204[2];
            D_800F3368.f4 = two;
            D_800F3368.f6 = 0;
            D_800F3368.f0 = 32;
            D_800F3368.f2 = two;
            D_800F3368.fe = 32;
            D_800F3368.f10 = 32;
            *s = value;
            func_800CEE20(p, 0, 4096, 4096, ((short *)s)[-3] * (p[9] / 2) + 192,
                          func_80077AA4(0, pal), 1, 128, &col1);
            break;
        }
        break;
    }
    return 0;
}
