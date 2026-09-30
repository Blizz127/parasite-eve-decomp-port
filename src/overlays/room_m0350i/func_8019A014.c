/* room_m0350i — func_8019A014, blob offset 0xB02C, 0x10C bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl 2026-09-27).
 * event callback: spawn func_800CEE20 effect with per-area pitch table D_800E1204; lever: int n (no ushort mask) */

extern int D_800E27EC;
extern unsigned short D_800F336C;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern char D_8019A630[];
extern int func_80077AA4();
extern void func_800CEE20();

int func_8019A014(int a0, void *a1)
{
    int n;
    unsigned short u;

    if (a0 == 1) {
        if (D_800E27EC >= 5) {
            return 1;
        }
        goto out;
    }
    if (a0 != 2) {
        return 0;
    }
    if (D_800E27EC < 2) {
        goto out;
    }
    u = D_800F336C;
    n = D_800E1204[u];
    if (u == 4 && D_800F3428 != 0) {
        n += 4;
    }
    func_800CEE20(a1, 0, 0x1000, 0x1000, 0xDC, func_80077AA4(0x30, n) & 0xFFFF, 1,
                  (6 - D_800E27EC) << 5, D_8019A630);
out:
    return 0;
}
