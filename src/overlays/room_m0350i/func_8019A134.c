/* room_m0350i — func_8019A134, blob offset 0xB14C, 0xE8 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl 2026-09-27).
 * event callback: frame>=4 query / spawn effect via func_800D004C with sine amplitude; levers: goto shared return 0 (no store-flag), int k + (short) casts keep lh */

typedef struct { short x, y, z; } SV3;
extern int D_800E27EC;
extern int D_800966EC[];
extern int D_8019A614[];
extern void func_800D004C();

int func_8019A134(int a0, int **a1)
{
    SV3 s;
    int k;

    if (a0 == 1) {
        if (D_800E27EC >= 4) {
            return 1;
        }
        goto out;
    }
    if (a0 != 2) {
        return 0;
    }
    s.x = (*a1)[0];
    s.y = (*a1)[1];
    s.z = (*a1)[2];
    k = ((short *)&D_800966EC[((D_800E27EC - 1) << 8) & 0xF00])[1];
    func_800D004C(&s, 0x200, 0x200, 0x10, 0, (short)k, (short)k, D_8019A614, D_8019A614 + 1, (short)k >> 5, 1);
out:
    return 0;
}
