/* room_m0156i - func_80190F94, blob offset 0x1FAC, 0xC00 bytes. Profile
 * era_o2_g0_expand_div (as room_m0159i func_8018F3A4).
 * Variant of room_m0159i func_8018F3A4 (the 3-mode burst effect): the arm
 * step also creates a 3D sprite object (func_8006E498 -> D_80192C30) and
 * plays sound 0x58A; the draw step keeps the GTE line but draws the glow
 * through the sprite object (RotMatrix/ScaleMatrix, func_800C6EC0..71E4)
 * instead of the two func_800CEE20 sprites. Was the Day 4 m0156i battle
 * stop CPU_BOUNDARY/REFUSED pc=0x80190F94 ra=@800D413C:effect. */

#define gte_SetRotMatrix(r0) __asm__ volatile(                       \
    "lw $12, 0(%0);" "lw $13, 4(%0);" "ctc2 $12, $0;" "ctc2 $13, $1;"   \
    "lw $12, 8(%0);" "lw $13, 12(%0);" "lw $14, 16(%0);"                \
    "ctc2 $12, $2;" "ctc2 $13, $3;" "ctc2 $14, $4;"                     \
    "lw $12, 20(%0);" "lw $13, 24(%0);" "ctc2 $12, $5;"                 \
    "lw $14, 28(%0);" "ctc2 $13, $6;" "ctc2 $14, $7"                    \
    : : "r"(r0) : "$12", "$13", "$14")
#define gte_ldv3(r0, r1, r2) __asm__ volatile(                          \
    "lwc2 $0, 0(%0);" "lwc2 $1, 4(%0);" "lwc2 $2, 0(%1);"               \
    "lwc2 $3, 4(%1);" "lwc2 $4, 0(%2);" "lwc2 $5, 4(%2)"                \
    : : "r"(r0), "r"(r1), "r"(r2))
#define gte_rtpt() __asm__ volatile("nop; nop; .word 0x4A280030")
#define gte_stszotz(r0) __asm__ volatile(                               \
    "mfc2 $12, $19; nop; sra $12, $12, 2; sw $12, 0(%0)"                \
    : : "r"(r0) : "$12", "memory")
#define gte_stsxy3(r0, r1, r2) __asm__ volatile(                        \
    "swc2 $12, 0(%0);" "swc2 $13, 0(%1);" "swc2 $14, 0(%2)"             \
    : : "r"(r0), "r"(r1), "r"(r2) : "memory")

typedef struct { short vx, vy, vz, pad; } SVECTOR;
typedef struct { short x, y, z, pad; int dur; int gap; } BurstDef;
typedef struct { char *ordering[2]; char *other_buffers[6]; char *packets[2]; } RenderBufferPrefix;
extern SVECTOR D_8018F048;
extern SVECTOR D_8018EFFC;
typedef struct { short m[3][3]; int t[3]; } MATRIX;
typedef struct { int vx, vy, vz, pad; } VECTOR;
extern void *D_80192C30;
extern void *D_800B0E64[];
extern unsigned short D_800E11EA;
extern unsigned short D_800E1204[];
extern void *func_8006E498(void *, int);
extern void func_800C6D5C(void *, int, int);
extern int func_80077A64(int, int, int, int);
extern void func_800C6EC0(int, int);
extern void func_800C6ED8(int);
extern void func_80079754(SVECTOR *, MATRIX *);
extern void func_80078CC4(MATRIX *, VECTOR *);
extern void func_800C6EF8(void *);
extern void func_800C6FA0(void *, int);
extern void func_800C71E4(void *, MATRIX *);
extern void func_800C6F4C(void *);
extern unsigned short D_800942EC[];
extern int D_8009CDD8;
extern int D_8009CDDC;
extern unsigned short D_8009D1CC;
extern int D_8009D248;
extern unsigned char **D_8009D254;
extern RenderBufferPrefix D_800B0E38;
extern int *D_800BCFA4;
extern unsigned short D_800E11E8;
extern unsigned short D_800E11EC;
extern unsigned short D_800E120C;
extern unsigned char *D_800E2368;
extern unsigned short D_800E2850[];
extern unsigned char *D_800F32D0;
extern unsigned char *D_800F33E0;
extern short D_800F3368;
extern short D_800F336A;
extern unsigned short D_800F336C;
extern short D_800F336E;
extern unsigned short D_800F3370;
extern short D_800F3372;
extern short D_800F3374;
extern short D_800F3376;
extern short D_800F3378;
extern int D_800F3428;
extern void func_80190BF8();
extern int func_8001CAB0(int, int, int, int);
extern int func_80071A54();
extern int func_80077AA4();
extern int func_80077CF4(int);
extern int func_80077DC4(int);
extern void func_800783E4(short *, short *, int, int, short *);
extern int func_800C6B90(void *, int);
extern int func_800CE560();
extern short *func_800CE610();
extern void func_800CE8F0();
extern void func_800CE9D4();
extern void func_800CEE20();
extern void func_800CF6F8(void *, void *, int);
extern void func_800D3F64(int, int);
extern int func_800D3FD8();

int func_80190F94(int mode, short *p, BurstDef *d)
{
    SVECTOR off0;
    SVECTOR off1;
    SVECTOR rot;
    int otz;
    int t;
    short *e;
    int ang;
    int i;
    int r;
    int pal;
    int clut;
    int **pp;
    unsigned char *flag;
    unsigned char *chan;
    unsigned char *prim;
    int **matrix_slot;

    off0 = D_8018F048;
    off1 = D_8018EFFC;
    switch (mode) {
    case 0:
        p[0xC] = 0;
        p[0xD] = 0;
        p[0xF] = 0;
        p[0x10] = 0;
        p[0x11] = 0;
        p[8] = d->x;
        p[9] = d->y;
        p[10] = d->z;
        p[9] = D_800942EC[0];
        D_80192C30 = func_8006E498(D_800B0E64[0], 0xC5485704);
        func_800C6D5C(D_80192C30, 0, 0);
        func_800D3F64(0x58A, func_800D3FD8());
        if (D_800E2368[0xD] != 0) {
            pp = *(int ***)(D_800F32D0 + 8);
            if (pp != 0 && *pp != 0) {
                flag = *(unsigned char **)((unsigned char *)*pp + 0x18);
                if (*flag == 1) {
                    *flag = 2;
                }
            }
        }
        return func_800CE560(*(void **)(D_800F33E0 + 8), 0x14, 0x20, func_80190BF8);
    case 1:
        func_800CE8F0(*(void **)(D_800F32D0 + 8), 0x1F, &off0, p);
        func_800CE9D4(*(void **)(D_800F32D0 + 8), 0, &rot);
        ang = -rot.vy + 0x400;
        switch (p[0xC]) {
        case 0:
            p[0xD]++;
            if (p[0xD] == 1) {
                for (i = 0; i < 2; i++) {
                    e = func_800CE610(*(void **)(D_800F33E0 + 8));
                    if (e) {
                        e[0] = p[0];
                        e[1] = p[1];
                        e[2] = p[2];
                        e[4] = func_80077DC4(ang) / 128;
                        e[6] = func_80077CF4(ang) / 128;
                        e[5] = 0;
                        e[8] = i;
                        e[9] = 0;
                    }
                }
            }
            t = 0x1000 - func_80077DC4((p[0xD] << 10) / 6);
            func_800783E4(p, p + 8, 0x1000 - t, t, p + 4);
            p[0xE] = 0x80 - (p[0xD] << 5) / 6;
            if (p[0xD] < 6) {
                break;
            }
            p[0xC] = 1;
            p[0xD] = 0;
            for (i = 0; i < 8; i++, ang += 0x200) {
                e = func_800CE610(*(void **)(D_800F33E0 + 8));
                if (e) {
                    e[0] = p[8];
                    e[1] = p[9];
                    e[2] = p[10];
                    r = (func_80071A54() & 0x1F) + 0x20;
                    e[4] = func_80077DC4(ang) * r / 4096;
                    e[6] = func_80077CF4(ang) * r / 4096;
                    e[5] = 0;
                    e[8] = 0;
                    e[9] = 0;
                }
            }
            if (!func_8001CAB0(p[8] << 16, p[10] << 16, D_8009D248, D_8009D1CC)) {
                return 1;
            }
            break;
        case 1:
            p[0xD]++;
            p[0xE] = func_80077DC4((p[0xD] << 10) / 6) / 128 + 0x40;
            p[4] = p[8];
            p[5] = p[9];
            p[6] = p[10];
            p[0xF] = func_80077CF4((p[0xD] << 10) / 6) + 0x800;
            p[0x10] = 0x80;
            if (p[0xD] < 6) {
                break;
            }
            p[0xC] = 2;
            p[0xD] = 0;
            p[0xE] = 0;
            for (i = 0; i < 22; i++) {
                e = func_800CE610(*(void **)(D_800F33E0 + 8));
                if (e) {
                    t = func_80071A54() & 0xFFF;
                    func_800783E4(p, p + 8, 0x1000 - t, t, e);
                    e[5] = 0;
                    e[7] = (func_80071A54() & 3) + 1;
                    e[8] = 1;
                    e[9] = 0;
                }
            }
            return 0;
        case 2:
            p[0xD]++;
            if (p[0xF] > 0x1000) {
                p[0xF] -= 0x400;
            }
            p[0x10] = func_80077CF4((p[0xD] << 14) / d->dur) / 128 + 0x80;
            if (p[0x11] != 0) {
                p[0x11]--;
            }
            if (func_800C6B90(p + 8, 600) && p[0x11] == 0) {
                if (D_800E2368[0xD] != 0) {
                    chan = D_800F32D0;
                    if ((***(int ***)(chan + 8) & 0x3F000000) == 0x01000000) {
                        *(int *)(*D_8009D254 + 0x4C) |= 0x4000;
                        **(int **)*(int ***)(chan + 8) = (**(int **)*(int ***)(chan + 8) & 0xC0FFFFFF) | 0x19000000;
                        **(int **)*(int ***)(chan + 8) |= 0x80000000;
                    }
                }
                p[0x11] = d->gap;
            }
            if (p[0xD] >= d->dur) {
                p[0xC] = 3;
                p[0xD] = 0;
            }
            return 0;
        case 3:
            p[0xD]++;
            p[0xF] = func_80077DC4(p[0xD] << 5);
            p[0x10] = func_80077DC4(p[0xD] << 5) / 32;
            if (p[0xD] >= 0x20) {
                return 1;
            }
            break;
        }
        break;
    case 2:
        matrix_slot = &D_800BCFA4;
        asm volatile("" : "=r"(matrix_slot) : "0"(matrix_slot));
        gte_SetRotMatrix(*matrix_slot);
        asm volatile("" : : "r"(matrix_slot));
        if (p[0xE] != 0) {
            prim = D_800B0E38.packets[D_8009CDDC] + D_8009CDD8;
            D_8009CDD8 += 0x10;
            gte_ldv3(p, p + 4, p);
            gte_rtpt();
            prim[3] = 3;
            prim[7] = 0x40;
            prim[4] = p[0xE];
            prim[5] = p[0xE];
            prim[6] = p[0xE];
            gte_stszotz(&otz);
            if (otz > 0 && otz < 0x1000) {
                {
                    unsigned char *s8 = prim + 8;
                    unsigned char *s12 = prim + 0xC;
                    gte_stsxy3(s8, s12, &t);
                }
                func_800CF6F8(D_800B0E38.ordering[D_8009CDDC] + otz * 4, prim, 1);
            }
        }
        {
            unsigned short *idx = &D_800E11EA;
            unsigned short *tab = D_800E2850;
            unsigned short *q;
            MATRIX m;
            VECTOR sc;
            int w;

            D_800F3368 = 0x40;
            D_800F336A = 4;
            D_800F3376 = 0x40;
            D_800F3378 = 0x40;
            q = &tab[*idx];
            pal = *q;
            D_800F336C = 3;
            D_800F336E = 0;
            D_800F3372 = 5;
            D_800F3374 = 0;
            D_800F3370 = pal;
            asm volatile("" : : : "memory");
            if (p[0x10] != 0) {
                matrix_slot = &D_800BCFA4;
                asm volatile("" : "=r"(matrix_slot) : "0"(matrix_slot));
                gte_SetRotMatrix(*matrix_slot);
                asm volatile("" : : "r"(matrix_slot));
                {
                    int pal2 = *q;
                    D_800F336C = 3;
                    D_800F336E = 0;
                    D_800F3370 = pal2;
                }
                w = func_80077A64(0, 1, 0, 0);
                w = (tab[*idx] | w) & 0xFFFF;
                clut = D_800E1204[D_800F336C];
                if (D_800F336C == 4 && D_800F3428) {
                    clut += 4;
                }
                func_800C6EC0(w, func_80077AA4(0x40, clut) & 0xFFFF);
                func_800C6ED8(1);
                func_80079754(&off1, &m);
                m.t[0] = p[8];
                m.t[1] = p[9];
                m.t[2] = p[10];
                sc.vx = p[0xF] * 3 / 2;
                sc.vy = p[0xF] * 3 / 2;
                sc.vz = p[0xF] * 3 / 2;
                func_80078CC4(&m, &sc);
                func_800C6EF8(D_80192C30);
                func_800C6FA0(D_80192C30, (p[0x10] / 2) & 0xFFFF);
                func_800C71E4(D_80192C30, &m);
                func_800C6F4C(D_80192C30);
            }
        }
        D_800F3368 = 0x20;
        D_800F336A = 2;
        D_800F3376 = 0x20;
        D_800F3378 = 0x20;
        pal = D_800E2850[D_800E11EA];
        asm volatile("" : : : "memory");
        D_800F336C = 3;
        D_800F336E = 0;
        D_800F3372 = 0;
        D_800F3374 = 8;
        D_800F3370 = pal;
        break;
    }
    return 0;
}
