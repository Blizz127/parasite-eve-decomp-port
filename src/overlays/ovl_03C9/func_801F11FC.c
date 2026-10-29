/* ovl_03C9 (PE.IMG handler-module overlay, VRAM 0x801ED7F8)
 * func_801F11FC — blob offset 0x3A04, 0x3E0 bytes. inline GTE/asm (gte_SetRotMatrix/SetTransMatrix).
 * Twin of func_801F1790: mode 0 seeds the object and spawns func_801F1070; mode 1 ticks,
 * placing a child (func_800CE610) every odd step up to 32; mode 2 sets the D_800BCFA4 camera
 * matrix and draws the sprite twice (0x40 and 0x46 modes) through func_800CEE20.
 * Profile era_o2_g0 (default), LINK_EXACT. Same levers as func_801F1790 (array-element draw
 * globals, `do {} while (0);` after the size line, $3 `four` pin, barriered $8 matrix pointer). */
#define gte_SetRotMatrix(r0) __asm__ volatile ( \
    "lw $12,0(%0)\n\tlw $13,4(%0)\n\tctc2 $12,$0\n\tctc2 $13,$1\n\t" \
    "lw $12,8(%0)\n\tlw $13,12(%0)\n\tlw $14,16(%0)\n\t" \
    "ctc2 $12,$2\n\tctc2 $13,$3\n\tctc2 $14,$4" : : "r"(r0) : "$12", "$13", "$14")
#define gte_SetTransMatrix(r0) __asm__ volatile ( \
    "lw $12,20(%0)\n\tlw $13,24(%0)\n\tctc2 $12,$5\n\tlw $14,28(%0)\n\t" \
    "ctc2 $13,$6\n\tctc2 $14,$7" : : "r"(r0) : "$12", "$13", "$14")
typedef struct { short vx, vy, vz, pad; } SVECTOR;
typedef struct { short m[3][3]; int t[3]; } MATRIX;
typedef struct {
    short f0;     /* 0x0 */
    short step;   /* 0x2 */
    short id;     /* 0x4 */
} Fx;
typedef struct {
    short x, y, z, pad;   /* 0x0 */
    unsigned char rest[8]; /* 0x8 */
    short wait;   /* 0x10 */
    short step;   /* 0x12 */
} Obj;
typedef struct { unsigned char pad[8]; int f8; } Ctx;
extern SVECTOR D_801ED818;
extern int D_8009D254;
extern Ctx *D_800F33E0;
extern MATRIX *D_800BCFA4;
extern unsigned char D_801F1DE8;
extern unsigned char D_801F1F30;
extern unsigned char D_801F1F31;
extern unsigned char D_801F1F32;
extern unsigned short D_800E11F6;
extern unsigned short D_800E2850[];
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern short D_800F3368[];
extern short D_800F336A[];
extern unsigned short D_800F336C;
extern unsigned short D_800F336E[];
extern unsigned short D_800F3370;
extern unsigned short D_800F3372[];
extern unsigned short D_800F3374[];
extern short D_800F3376[];
extern short D_800F3378[];
extern void func_800CE8F0();
extern int func_800CE560();
extern Obj *func_800CE610();
extern void func_800CF3AC();
extern void func_800CEDA8();
extern void func_800CEE20();
extern int func_80077CF4();
extern unsigned short func_80077AA4();
extern int func_801F1070();

int func_801F11FC(int mode, Fx *o)
{
    SVECTOR v;
    SVECTOR a;
    unsigned char buf[8];
    register MATRIX *m asm("$8");
    MATRIX **slot;
    Obj *q;
    int size;
    int cy;
    int k;
    register int four asm("$3");

    a = D_801ED818;
    switch (mode) {
    case 0:
        o->id = 0x13;
        o->f0 = 0;
        o->step = 0;
        return func_800CE560(D_800F33E0->f8, 0x14, 0x18, func_801F1070);
    case 1:
        o->step++;
        func_800CE8F0(D_8009D254, o->id, &a, &v);
        if (o->step < 0x21 && (o->step & 1)) {
            q = func_800CE610(D_800F33E0->f8);
            if (q != 0) {
                q->x = v.vx;
                q->y = v.vy;
                q->z = v.vz;
                q->wait = 0;
                q->step = 0;
            }
        }
        if (o->step < 0x20) break;
        return 2;
    case 2:
        slot = &D_800BCFA4;
        __asm__("" : "=r"(slot) : "0"(slot));
        m = *slot;
        gte_SetRotMatrix(m);
        gte_SetTransMatrix(m);
        func_800CE8F0(D_8009D254, o->id, &a, &v);
        D_800F3368[0] = 0x20;
        D_800F336A[0] = 2;
        D_800F3376[0] = 0x20;
        D_800F3378[0] = 0x20;
        D_800F336C = 1;
        D_800F3370 = D_800E2850[D_800E11F6];
        func_800CEDA8(1);
        D_800F336E[0] = 1;
        D_800F3372[0] = 5;
        D_800F3374[0] = 0x20;
        if (o->step < 0x21) {
            func_800CF3AC(&D_801F1DE8, buf, o->step);
            size = func_80077CF4(o->step << 6) + 0x1000;
            do {} while (0);
            k = D_800F336C;
            four = 4;
            cy = D_800E1204[k];
            if (k == four && D_800F3428 != 0) {
                cy += 8;
            } else {
                cy += 4;
            }
            func_800CEE20(&v, 0, size, size, 0x40, func_80077AA4(0, cy), 1, 0x80, buf);
            k = D_800F336C;
            four = 4;
            cy = D_800E1204[k];
            if (k == four && D_800F3428 != 0) {
                cy += 7;
            } else {
                cy += 3;
            }
            func_800CEE20(&v, 0, size, size, 0x46, func_80077AA4(0, cy), 1, 0x80, buf);
            D_801F1F30 = buf[0];
            D_801F1F31 = buf[1];
            D_801F1F32 = buf[2];
        } else {
            *(int *)&D_801F1F30 = 0;
        }
        D_800F3372[0] = 0;
        D_800F3374[0] = 0x10;
        break;
    }
    return 0;
}
