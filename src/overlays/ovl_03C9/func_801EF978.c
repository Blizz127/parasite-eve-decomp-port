/* ovl_03C9 (PE.IMG handler-module overlay, VRAM 0x801ED7F8)
 * func_801EF978 — blob offset 0x2180, 0x6F0 bytes. inline GTE/asm (gte_SetRotMatrix/SetTransMatrix).
 * Effect handler (mode, obj), keyed on the D_800E27EC tick: mode 0 copies the anchor position
 * from D_800F32D0->f8 (+0x268) and spawns func_801EF560; mode 1 bursts six children at tick 3
 * (rand % 50 - 25 spins) and four jittered children per tick before tick 5 (rand % 190 - 95
 * spins), done at tick 8; mode 2 sets the camera matrix, flashes via func_800D1AE0 on ticks 1/2
 * and draws three sprite layers (func_800CEE20 / func_800D004C / func_800D0728) up to tick 32.
 * Profile era_o2_g0 (default), LINK_EXACT. Levers: the GTE macros take `*slot` (a memory
 * operand; cc1's reload picks retail's $t1, no register pin); D_800F3368/336A/3376/3378 as
 * array elements. */
#define gte_SetRotMatrix(r0) __asm__ volatile ( \
    "lw $12,0(%0)\n\tlw $13,4(%0)\n\tctc2 $12,$0\n\tctc2 $13,$1\n\t" \
    "lw $12,8(%0)\n\tlw $13,12(%0)\n\tlw $14,16(%0)\n\t" \
    "ctc2 $12,$2\n\tctc2 $13,$3\n\tctc2 $14,$4" : : "r"(r0) : "$12", "$13", "$14")
#define gte_SetTransMatrix(r0) __asm__ volatile ( \
    "lw $12,20(%0)\n\tlw $13,24(%0)\n\tctc2 $12,$5\n\tlw $14,28(%0)\n\t" \
    "ctc2 $13,$6\n\tctc2 $14,$7" : : "r"(r0) : "$12", "$13", "$14")
typedef struct { unsigned char r, g, b, cd; } CVECTOR;
typedef struct { short m[3][3]; int t[3]; } MATRIX;
typedef struct {
    short x, y, z, w;     /* 0x0 */
    short rx, ry, rz;     /* 0x8 */
    short e;      /* 0xE */
    short wait;   /* 0x10 */
    short step;   /* 0x12 */
} Obj;
typedef struct { unsigned char pad[0x268]; short x, y, z; } Anchor;
typedef struct { unsigned char pad[8]; Anchor *f8; } Owner;
typedef struct { unsigned char pad[8]; int f8; } Ctx;
extern CVECTOR D_801ED844;
extern Owner *D_800F32D0;
extern Ctx *D_800F33E0;
extern int D_800E27EC;
extern MATRIX *D_800BCFA4;
extern unsigned char D_801F1CB0;
extern int D_800F3428;
extern unsigned short D_800E11E6;
extern unsigned short D_800E11F6;
extern unsigned short D_800E2850[];
extern unsigned short D_800E1204[];
extern short D_800F3368[];
extern short D_800F336A[];
extern unsigned short D_800F336C;
extern unsigned short D_800F336E;
extern unsigned short D_800F3370;
extern unsigned short D_800F3372;
extern unsigned short D_800F3374;
extern short D_800F3376[];
extern short D_800F3378[];
extern int func_800CE560();
extern Obj *func_800CE610();
extern int func_80071A54();
extern int func_80077CF4();
extern int func_80077DC4();
extern unsigned short func_80077AA4();
extern void func_800CEDA8();
extern void func_800D1AE0();
extern void func_800CEE20();
extern void func_800CF3AC();
extern void func_800D004C();
extern void func_800D0728();
extern int func_801EF560();

int func_801EF978(int mode, Obj *o)
{
    unsigned char buf[8];
    CVECTOR c;
    MATRIX **slot;
    Owner *p;
    Obj *q;
    int i;
    int u;
    int k;
    int j;
    int cy;
    int cl;
    int sz;

    c = D_801ED844;
    switch (mode) {
    case 0:
        p = D_800F32D0;
        o->x = p->f8->x;
        o->y = p->f8->y;
        o->z = p->f8->z;
        return func_800CE560(D_800F33E0->f8, 0x14, 0x16, func_801EF560);
    case 1:
        if (D_800E27EC == 3) {
            for (i = 0; i < 6; i++) {
                q = func_800CE610(D_800F33E0->f8);
                if (q != 0) {
                    q->x = o->x;
                    q->y = o->y;
                    q->z = o->z;
                    q->rx = func_80071A54() % 50 - 25;
                    q->ry = func_80071A54() % 50 - 25;
                    q->rz = func_80071A54() % 50 - 25;
                    q->wait = 1;
                    q->step = 0;
                }
            }
        }
        if (D_800E27EC < 5) {
            for (i = 0; i < 4; i++) {
                q = func_800CE610(D_800F33E0->f8);
                if (q != 0) {
                    q->x = o->x;
                    q->y = o->y;
                    q->z = o->z;
                    q->w = (func_80071A54() & 0x7FF) + 0x800;
                    q->x += (func_80071A54() & 0x7F) - 0x40;
                    q->y += (func_80071A54() & 0x7F) - 0x40;
                    q->z += (func_80071A54() & 0x7F) - 0x40;
                    q->rx = func_80071A54() % 190 - 95;
                    q->ry = func_80071A54() % 190 - 95;
                    q->rz = func_80071A54() % 190 - 95;
                    q->wait = 0;
                    q->step = 0;
                }
            }
        }
        if (D_800E27EC < 8) break;
        return 2;
    case 2:
        slot = &D_800BCFA4;
        __asm__("" : "=r"(slot) : "0"(slot));
        gte_SetRotMatrix(*slot);
        gte_SetTransMatrix(*slot);
        D_800F3368[0] = 0x20;
        D_800F336A[0] = 2;
        D_800F3376[0] = 0x20;
        D_800F3378[0] = 0x20;
        D_800F336C = 1;
        D_800F3370 = D_800E2850[D_800E11E6];
        func_800CEDA8(1);
        D_800F336E = 0;
        D_800F3372 = 0;
        D_800F3374 = 0x20;
        if (D_800E27EC == 1) {
            func_800D1AE0(&c, 0x80, 2, 8);
        } else if (D_800E27EC == 2) {
            func_800D1AE0(&c, 0x80, 1, 8);
        }
        if (D_800E27EC < 0x21) {
            u = D_800E27EC << 5;
            k = func_80077DC4(u) / 32;
            cy = D_800E1204[D_800F336C];
            if (D_800F336C == 4 && D_800F3428 != 0) {
                cy += 4;
            }
            cl = func_80077AA4(0xC0, cy);
            func_800CEE20(o, 0, 0x2000, 0x2000, D_800F336A[0] * (D_800E27EC / 4) + 0xC0, cl, 1, k, 0);
            func_800CF3AC(&D_801F1CB0, buf, D_800E27EC);
            sz = func_80077CF4(u) + 0x1000;
            if (D_800E27EC & 1) {
                k = k * 2 / 3;
            }
            func_800D004C(o, 0x1F4, 0x1F4, 0xE, 0, sz, sz, buf, 0, k, 1);
            j = func_80077DC4(u);
            k = j / 32;
            sz = func_80077CF4(u) + 0x800;
            func_800D0728(o, 0x44C, 0x6A4, 0x12, 0, sz, sz, 0, buf, k / 2, 1);
        }
        D_800F336C = 1;
        D_800F3370 = D_800E2850[D_800E11F6];
        func_800CEDA8(1);
        D_800F336E = 1;
        D_800F3372 = 0;
        D_800F3374 = 0x10;
        break;
    }
    return 0;
}
