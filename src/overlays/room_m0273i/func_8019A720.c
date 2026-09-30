/* room_m0273i — func_8019A720, blob offset 0xB738, 0xF0 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl13 2026-09-28).
 * func_800D004C drawer: case-1 a1[1] -= a1[3]; a1[3]++; idiom (retail double lhu) (ovl park 4w -> 0). */

typedef struct { short x, y, z, pad; } SV;
extern int D_800E27EC;
extern int D_800966EC[];
extern char D_8019AB70[];
extern char D_8019AD7C[];
extern void func_800D004C();

int func_8019A720(int a0, unsigned char *a1)
{
    SV s;
    int k;

    if (a0 == 1) {
        if (D_800E27EC >= 0x10) {
            return 1;
        }
        ((short *)a1)[1] -= ((short *)a1)[3];
        ((short *)a1)[3]++;
        goto out;
    }
    if (a0 != 2) {
        return 0;
    }
    s.x = 0;
    s.y = 0;
    s.pad = 0;
    s.z = (D_800E27EC - 1) << 7;
    k = D_800966EC[((D_800E27EC - 1) << 6) & 0xFC0];
    func_800D004C(a1, 0x100, 0x100, 0x10, &s, (short)k + 0x400, (short)k + 0x400,
                  D_8019AB70, D_8019AD7C, k >> 21, 1);
out:
    return 0;
}
