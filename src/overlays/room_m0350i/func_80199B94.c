/* room_m0350i — func_80199B94, blob offset 0xABAC, 0x110 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl 2026-09-27).
 * event callback twin with sine-scaled parameter */

extern int D_800E27EC;
extern unsigned short D_800F336C;
extern short D_800F336A;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern int D_800966EC[];
extern char D_8019A410[];
extern int func_80077AA4();
extern void func_800CEE20();

int func_80199B94(int a0, void *a1)
{
    int n;
    unsigned short u;

    if (a0 == 1) {
        if (D_800E27EC >= 8) {
            return 1;
        }
        goto out;
    }
    if (a0 != 2) {
        return 0;
    }
    u = D_800F336C;
    n = D_800E1204[u];
    if (u == 4 && D_800F3428 != 0) {
        n += 4;
    }
    func_800CEE20(a1, 0, 0x1800, 0x1800, D_800F336A + 0xDC, func_80077AA4(0x30, n) & 0xFFFF, 1,
                  (short)D_800966EC[((D_800E27EC - 1) << 8) & 0xF00] >> 5, D_8019A410);
out:
    return 0;
}
