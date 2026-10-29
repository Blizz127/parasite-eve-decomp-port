/* ovl_03C9 (PE.IMG handler-module overlay, VRAM 0x801ED7F8)
 * func_801F0D30 — blob offset 0x3538, 0x340 bytes. inline GTE/asm (gte_SetRotMatrix/SetTransMatrix).
 * Effect handler (mode, obj): mode 0 seeds the object and spawns func_801F0BA0; mode 1 ticks
 * and fades; mode 2 draws through func_800D1384 under the D_800BCFA4 camera matrix.
 * Profile era_o2_g0 (default), LINK_EXACT. Levers: camera matrix through a barriered slot
 * (slot = &D_800BCFA4; asm("" : "=r"(slot) : "0"(slot))) into a `register asm("$8")` pointer;
 * D_801F1D86/D_801F1D88 read as array elements so their loads stay after the struct-field stores;
 * mode 1/2 end in `break` to the shared return 0. */
#define gte_SetRotMatrix(r0) __asm__ volatile ( \
    "lw $12,0(%0)\n\tlw $13,4(%0)\n\tctc2 $12,$0\n\tctc2 $13,$1\n\t" \
    "lw $12,8(%0)\n\tlw $13,12(%0)\n\tlw $14,16(%0)\n\t" \
    "ctc2 $12,$2\n\tctc2 $13,$3\n\tctc2 $14,$4" : : "r"(r0) : "$12", "$13", "$14")
#define gte_SetTransMatrix(r0) __asm__ volatile ( \
    "lw $12,20(%0)\n\tlw $13,24(%0)\n\tctc2 $12,$5\n\tlw $14,28(%0)\n\t" \
    "ctc2 $13,$6\n\tctc2 $14,$7" : : "r"(r0) : "$12", "$13", "$14")
typedef struct { short vx, vy, vz, pad; } SVECTOR;
typedef struct { unsigned char r, g, b, cd; } CVECTOR;
typedef struct { short m[3][3]; int t[3]; } MATRIX;
typedef struct {
    short f0;     /* 0x0 */
    short step;   /* 0x2 */
    short alpha;  /* 0x4 */
    short pad6;
    unsigned char rest[8]; /* 0x8 */
} Fx;
typedef struct { unsigned char pad[8]; int f8; } Ctx;
extern SVECTOR D_801ED818;
extern CVECTOR D_801ED850;
extern CVECTOR D_801ED854;
extern int D_8009D254;
extern short D_801F1D86[];
extern short D_801F1D88[];
extern Ctx *D_800F33E0;
extern MATRIX *D_800BCFA4;
extern unsigned char D_801F1D5C;
extern unsigned short D_800E11E6;
extern unsigned short D_800E2850[];
extern unsigned short D_800F336C;
extern unsigned short D_800F336E;
extern unsigned short D_800F3370;
extern unsigned short D_800F3372;
extern unsigned short D_800F3374;
extern void func_800CE8F0();
extern void func_800D1384();
extern int func_800CE560();
extern void func_800CF3AC();
extern void func_800CEDA8();
extern int func_801F0BA0();

int func_801F0D30(int mode, Fx *o)
{
    SVECTOR v0;
    SVECTOR v1;
    SVECTOR v2;
    SVECTOR a;
    unsigned char buf[8];
    CVECTOR b;
    CVECTOR c;
    register MATRIX *m asm("$8");
    short t;
    MATRIX **slot;

    a = D_801ED818;
    b = D_801ED850;
    c = D_801ED854;
    switch (mode) {
    case 0:
        o->alpha = 0x80;
        o->f0 = 0;
        o->step = 0;
        func_800CE8F0(D_8009D254, D_801F1D86[0], &a, &v1);
        func_800CE8F0(D_8009D254, D_801F1D88[0], &a, &v2);
        func_800D1384(&v1, &v2, 0x3F8, &c, &b, 0x80, o->rest, 1);
        return func_800CE560(D_800F33E0->f8, 0x14, 0x16, func_801F0BA0);
    case 1:
        o->step++;
        func_800CE8F0(D_8009D254, D_801F1D88[0], &a, &v0);
        if (o->step >= 0x17) {
            t = o->alpha;
            if (t > 0) o->alpha = t - 8;
        }
        if (o->step < 0x16) break;
        return 2;
    case 2:
        if (o->alpha >= 0) {
            func_800CF3AC(&D_801F1D5C, buf, o->step);
            func_800CE8F0(D_8009D254, D_801F1D86[0], &a, &v1);
            func_800CE8F0(D_8009D254, D_801F1D88[0], &a, &v2);
            slot = &D_800BCFA4;
            __asm__("" : "=r"(slot) : "0"(slot));
            m = *slot;
            gte_SetRotMatrix(m);
            gte_SetTransMatrix(m);
            func_800D1384(&v1, &v2, 0x10, &c, &b, o->alpha, o->rest, 1);
        }
        D_800F336C = 1;
        D_800F3370 = D_800E2850[D_800E11E6];
        func_800CEDA8(1);
        D_800F336E = 0;
        D_800F3372 = 0;
        D_800F3374 = 0x10;
        break;
    }
    return 0;
}
