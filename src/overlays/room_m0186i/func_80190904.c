/* room_m0186i — func_80190904, blob offset 0x191C, 0x154 bytes. Profile era_o2_g0 (default).
 * LINK_EXACT at the target VMA (lane ovl3 2026-09-27). */

typedef struct { short x, y, z, pad; } V4;
extern short D_800942EC;
extern int func_80071A54();

void func_80190904(int a0, unsigned char *a1, unsigned char *a2)
{
    unsigned int i;
    V4 *v = (V4 *)a2;
    int dx, dy, dz;

    if (*(short *)(a2 + 0xD2) >= 9) {
        *(short *)(a2 + 0xD2) -= 8;
    }
    for (i = 0; i < 8; i++) {
        if ((a2 + i)[0xC0] == 1) {
            dx = v[i + 8].x >> 8;
            dy = v[i + 8].y >> 8;
            dz = v[i + 8].z >> 8;
            v[i].x += dx;
            v[i].y += dy;
            v[i].z += dz;
            v[i + 8].y += 0x190;
            *(short *)((char *)&v[i] + 0x84) += func_80071A54() % 512;
            if (v[i].y > D_800942EC) {
                a2[0xD4]--;
                (a2 + i)[0xC0] = 0;
            }
        }
    }
    if (a2[0xD4] == 0) {
        a1[1] = 2;
    }
}
