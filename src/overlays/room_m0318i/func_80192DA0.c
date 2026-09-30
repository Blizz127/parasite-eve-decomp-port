/* room_m0318i — func_80192DA0, blob offset 0x3DB8, 0x134 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl13 2026-09-28).
 * k==4 sprite + func_800CF3AC: a1[1] -= a1[3]; a1[3]--; (retail double load), k read between v.x and v.y, pinned four $3 (ovl park 34w -> 0). */

typedef struct { short x, y, z; } SV3;
typedef struct { short x, y, z, pad; } SV;
extern int D_800E27EC;
extern unsigned short D_800F336C;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern char D_8019948C[];
extern int func_80077AA4();
extern void func_800CF3AC();
extern void func_800CEE20();

int func_80192DA0(int a0, short *a1)
{
    SV w;
    SV3 v;
    int n;
    int u;
    register int four asm("$3");

    switch (a0) {
    case 1:
        a1[1] -= a1[3];
        a1[3]--;
        if (D_800E27EC >= 0x18) {
            return 1;
        }
        break;
    case 2:
        func_800CF3AC(D_8019948C, &w, D_800E27EC);
        v.x = a1[0];
        u = D_800F336C;
        v.y = a1[1];
        v.z = a1[2];
        four = 4;
        n = D_800E1204[u];
        if (u == four && D_800F3428 != 0) {
            n += 10;
        } else {
            n += 6;
        }
        func_800CEE20(&v, 0, 0x800, 0x800, 6, func_80077AA4(0, n) & 0xFFFF, 1, 0x80, &w);
        break;
    default:
        return 0;
    }
    return 0;
}
