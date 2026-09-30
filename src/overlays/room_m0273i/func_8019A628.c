/* room_m0273i — func_8019A628, blob offset 0xB640, 0xE4 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl 2026-09-27).
 * event state handler (spawn / periodic vector copy); levers: if-tree, SVECTOR-typed source and destination */

extern unsigned char *D_800F33E0;
extern int D_800E27EC;
extern unsigned char D_8019AF6B;
typedef struct { short x, y, z, pad; } SV;
extern SV D_8019AEFC;
extern int func_800CE560();
extern SV *func_800CE610();
extern void func_8019A720();

int func_8019A628(int a0)
{
    SV *p;

    if (a0 != 1) {
        if (a0 < 2) {
            if (a0 == 0) {
                return func_800CE560(*(void **)(D_800F33E0 + 8), 8, 5, func_8019A720);
            }
            return 0;
        }
    } else {
        if (D_8019AF6B != 0) {
            return 2;
        }
        if (D_800E27EC & 3) {
            return 0;
        }
        p = func_800CE610(*(void **)(D_800F33E0 + 8));
        if (p == 0) {
            return 0;
        }
        p->x = D_8019AEFC.x;
        p->y = D_8019AEFC.y;
        p->z = D_8019AEFC.z;
        p->pad = 0;
    }
    return 0;
}
