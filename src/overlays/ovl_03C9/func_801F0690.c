/* ovl_03C9 (PE.IMG handler-module overlay, VRAM 0x801ED7F8)
 * func_801F0690 — blob offset 0x2E98, 0x510 bytes. inline GTE/asm (gte_SetRotMatrix/SetTransMatrix).
 * Effect handler (mode, obj): mode 0 copies the anchor position from D_800F32D0->f8 (+0x268),
 * seeds the rotation, and spawns func_801F0068; mode 1 is keyed on the D_800E27EC tick (3: sound
 * func_8006DDCC(0x4B1); 2 and 8: a centre child plus six random children via func_800CE610 /
 * func_80071A54; 2 again: a +0x800-rotated child), done at tick 8; mode 2 sets the camera matrix
 * and publishes the position to D_801F1F28.
 * Profile era_o2_g0 (default), LINK_EXACT. Levers: child rotation copied rx, ry, rz and only
 * then `+=` (the intervening store kills cse's equivalence, giving retail's store/reload pair);
 * D_801F1F28 typed as a 3-short struct (keeps each o-> load after the previous store). */
#define gte_SetRotMatrix(r0) __asm__ volatile ( \
    "lw $12,0(%0)\n\tlw $13,4(%0)\n\tctc2 $12,$0\n\tctc2 $13,$1\n\t" \
    "lw $12,8(%0)\n\tlw $13,12(%0)\n\tlw $14,16(%0)\n\t" \
    "ctc2 $12,$2\n\tctc2 $13,$3\n\tctc2 $14,$4" : : "r"(r0) : "$12", "$13", "$14")
#define gte_SetTransMatrix(r0) __asm__ volatile ( \
    "lw $12,20(%0)\n\tlw $13,24(%0)\n\tctc2 $12,$5\n\tlw $14,28(%0)\n\t" \
    "ctc2 $13,$6\n\tctc2 $14,$7" : : "r"(r0) : "$12", "$13", "$14")
typedef struct { short m[3][3]; int t[3]; } MATRIX;
typedef struct {
    short x, y, z, pad;   /* 0x0 */
    short rx, ry, rz;     /* 0x8 */
    short e;      /* 0xE */
    short wait;   /* 0x10 */
    short step;   /* 0x12 */
} Obj;
typedef struct { unsigned char pad[0x268]; short x, y, z; } Anchor;
typedef struct { unsigned char pad[8]; Anchor *f8; } Owner;
typedef struct { unsigned char pad[8]; int f8; } Ctx;
extern Owner *D_800F32D0;
extern int D_8009D254;
extern int D_800E22D4;
extern Ctx *D_800F33E0;
extern int D_800E27EC;
extern MATRIX *D_800BCFA4;
typedef struct { short x, y, z; } SV3;
extern SV3 D_801F1F28;
extern unsigned short D_800E11E6;
extern unsigned short D_800E2850[];
extern unsigned short D_800F336C;
extern unsigned short D_800F336E;
extern unsigned short D_800F3370;
extern unsigned short D_800F3372;
extern unsigned short D_800F3374;
extern void func_800CE9D4();
extern void func_800C6D5C();
extern int func_800CE560();
extern Obj *func_800CE610();
extern void func_8006DDCC();
extern int func_80071A54();
extern void func_800CEDA8();
extern int func_801F0068();

int func_801F0690(int mode, Obj *o)
{
    register MATRIX *m asm("$8");
    MATRIX **slot;
    Owner *p;
    Obj *q;
    int i;

    switch (mode) {
    case 0:
        p = D_800F32D0;
        o->x = p->f8->x;
        o->y = p->f8->y;
        o->z = p->f8->z;
        func_800CE9D4(D_8009D254, 0, &o->rx);
        o->rx = 0;
        func_800C6D5C(D_800E22D4, 0, 0);
        return func_800CE560(D_800F33E0->f8, 0x14, 0x28, func_801F0068);
    case 1:
        if (D_800E27EC == 3) {
            func_8006DDCC(0x4B1, 0, o->x, o->y, o->z);
        }
        if (D_800E27EC == 2) {
            q = func_800CE610(D_800F33E0->f8);
            if (q != 0) {
                q->x = o->x;
                q->y = o->y;
                q->z = o->z;
                q->rx = o->rx;
                q->ry = o->ry;
                q->rz = o->rz;
                q->e = 1;
                q->wait = 0;
                q->step = 0;
            }
            for (i = 0; i < 6; i++) {
                q = func_800CE610(D_800F33E0->f8);
                if (q != 0) {
                    q->x = (func_80071A54() & 0x7F) - 0x40;
                    q->y = 0;
                    q->z = (func_80071A54() & 0x1F) + 0x10;
                    q->rx = o->rx;
                    q->ry = o->ry;
                    q->rz = o->rz;
                    q->ry += 0x400;
                    q->rx += func_80071A54();
                    q->e = 0;
                    q->wait = 1;
                    q->step = 0;
                }
            }
        }
        if (D_800E27EC == 8) {
            q = func_800CE610(D_800F33E0->f8);
            if (q != 0) {
                q->x = o->x;
                q->y = o->y;
                q->z = o->z;
                q->rx = o->rx;
                q->ry = o->ry;
                q->rz = o->rz;
                q->rx += 0x400;
                q->e = 1;
                q->wait = 0;
                q->step = 0;
            }
            for (i = 0; i < 6; i++) {
                q = func_800CE610(D_800F33E0->f8);
                if (q != 0) {
                    q->x = 0;
                    q->y = (func_80071A54() & 0x7F) - 0x40;
                    q->z = (func_80071A54() & 0x1F) + 0x10;
                    q->rx = o->rx;
                    q->ry = o->ry;
                    q->rz = o->rz;
                    q->ry += func_80071A54();
                    q->e = 0;
                    q->wait = 1;
                    q->step = 0;
                }
            }
        }
        if (D_800E27EC == 2) {
            q = func_800CE610(D_800F33E0->f8);
            if (q != 0) {
                q->x = o->x;
                q->y = o->y;
                q->z = o->z;
                q->rx = o->rx;
                q->ry = o->ry;
                q->rz = o->rz;
                q->ry += 0x800;
                q->wait = 2;
                q->step = 0;
            }
        }
        if (D_800E27EC < 8) break;
        return 2;
    case 2:
        slot = &D_800BCFA4;
        __asm__("" : "=r"(slot) : "0"(slot));
        m = *slot;
        gte_SetRotMatrix(m);
        gte_SetTransMatrix(m);
        D_801F1F28.x = o->x;
        D_801F1F28.y = o->y;
        D_801F1F28.z = o->z;
        D_800F336C = 1;
        D_800F3370 = D_800E2850[D_800E11E6];
        func_800CEDA8(1);
        D_800F336E = 0;
        D_800F3372 = 0;
        D_800F3374 = 0x20;
        break;
    }
    return 0;
}
