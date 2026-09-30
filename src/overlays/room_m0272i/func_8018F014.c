/* room_m0272i — func_8018F014, blob offset 0x2C, 0x164 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl7 2026-09-27).
 * Display/VRAM setup by D_801991C8 state: three RECT templates, clear/move image, draw-env push (func_800755F0 on D_800BCE80[D_8009CDDC]). */

typedef struct { short x, y, w, h; } RECT;
extern RECT D_8018EFF4, D_8018EFFC, D_8018F004;
extern int D_801991C8;
extern int D_8009CDDC;
extern char D_800BCE80[];
extern char D_8018F1C8[];
extern void func_800750CC(), func_8007506C(), func_8007512C(), func_80074DC0(), func_80073A44(), func_800755F0(), func_80074D28();

int func_8018F014(void)
{
    RECT r0;
    RECT r1;
    RECT r2;

    r0 = D_8018EFF4;
    r1 = D_8018EFFC;
    r2 = D_8018F004;
    if (D_801991C8 == 0) {
        func_800750CC(&r0, D_8018F1C8);
        func_8007512C(&r1, 0x300, 0x100);
        func_80074DC0(0);
    }
    if (D_801991C8 == 1) {
        func_8007506C(&r0, D_8018F1C8);
        func_8007512C(&r2, 0x140, 0x100);
        func_80074DC0(0);
        func_80073A44(2);
        func_800755F0(D_800BCE80 + D_8009CDDC * 20);
        func_80074D28(1);
    }
    return 0;
}
