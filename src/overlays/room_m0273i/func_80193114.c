/* room_m0273i — func_80193114, blob offset 0x412C, 0xF8 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl 2026-09-27).
 * weighted random pick over D_8019A8C8[a0][band(y)][(x>>10)&3][6]; lever: reuse y for band index, for(i=0,s=0;...) */

extern unsigned char D_8019A8C8[][3][4][7];
extern int func_80052B2C();

int func_80193114(int a0, int **a1)
{
    int b = (*a1[0] >> 10) & 3;
    int y = *a1[1];
    unsigned char *p;
    int t;
    int i;
    int s;

    if (y < 0x280) {
        y = 0;
    } else if (y < 0x600) {
        y = 1;
    } else {
        y = 2;
    }
    p = D_8019A8C8[a0][y][b];
    t = (func_80052B2C() & 0xFF) * 100 / 256;
    for (i = 0, s = 0; i < 6; i++) {
        s += p[i];
        if (t < s) {
            return i;
        }
    }
    return i;
}
