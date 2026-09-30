/* room_m0141i — func_801911F0, blob offset 0x2208, 0x140 bytes. Profile era_o2_g0_narrow_shifted_word_load
 * (local; MASPSX_NARROW_SHIFTED_WORD_LOAD=1). LINK_EXACT at the target VMA (lane ovl3 2026-09-27). */

extern int D_800E27EC;
extern unsigned short D_800F336C;
extern short D_800F336A;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern int D_800966EC[];
extern char D_80192118[];
extern char D_80192120[];
extern unsigned short func_80077AA4();
extern void func_800CEE20();

int func_801911F0(int a0, void *a1)
{
    int i;
    int t;
    int c;
    int k;
    int v;

    if (a0 == 1) {
        if (D_800E27EC >= 9) {
            return 1;
        }
        goto out;
    }
    if (a0 != 2) {
        return 0;
    }
    t = D_800E27EC - 1;
    k = D_800F336C;
    c = D_800966EC[(t << 7) & 0xF80] + 0x200;
    v = D_800E1204[k];
    i = t;
    if (k == 4 && D_800F3428 != 0) {
        v += 4;
    }
    func_800CEE20(a1, D_80192118, (short)c, (short)c, D_800F336A * 2 + 0xFD,
                  func_80077AA4(0x80, v), 1,
                  D_800966EC[(i << 7) & 0xF80] >> 21, D_80192120);
out:
    return 0;
}
