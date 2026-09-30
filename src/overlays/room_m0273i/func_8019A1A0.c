/* room_m0273i — func_8019A1A0, blob offset 0xB1B8, 0xF0 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl 2026-09-27).
 * camera shake/param preset selector by mode a1 (0/1/2), random sign via func_80052B2C */

extern unsigned char D_8019AEFA;
extern short D_8019AEEE, D_8019AEEA, D_8019AEF2, D_8019AEF6;
extern char D_8019AE9C[];
extern int func_80052B2C();

char *func_8019A1A0(int a0, int a1)
{
    short v;

    D_8019AEFA = a1;
    switch (a1) {
    case 0:
        D_8019AEEE = 0;
        D_8019AEEA = 2;
        break;
    case 1:
        v = (func_80052B2C() & 1) ? 0x2A : -0x2A;
        D_8019AEF2 = 2;
        D_8019AEEA = 2;
        D_8019AEEE = v;
        D_8019AEF6 = 0x61;
        break;
    case 2:
        v = (func_80052B2C() & 1) ? 0x80 : -0x80;
        D_8019AEF2 = 4;
        D_8019AEEA = 5;
        D_8019AEEE = v;
        D_8019AEF6 = 0xB0;
        break;
    }
    return D_8019AE9C;
}
