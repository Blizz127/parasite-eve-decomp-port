/* room_m0137i — func_801900B0, blob offset 0x10C8, 0xDC bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl 2026-09-27).
 * six-particle integrator (+0x20 pos, +0x50 vel, +0x98 alive, +0xA4 count); lever: explicit i/one/lim/two locals reproduce loop-hoist order */

typedef struct { short x, y, z, pad; } SV;
typedef struct {
    unsigned char pad0[0x20];
    SV p[6];
    SV v[6];
    unsigned char pad80[0x18];
    short f[6];
    unsigned char padA4[0xC - 0xC];
    short cnt;
} W;
extern short D_800942EC;

void func_801900B0(void *a0, unsigned char *a1, W *a2)
{
    unsigned int i;
    int dx, dy, dz;
    short *lim;
    int two;
    int one;

    i = 0;
    one = 1;
    lim = &D_800942EC;
    two = 2;
    for (; i < 6; i++) {
        if (a2->f[i] == one) {
            dx = a2->v[i].x >> 4;
            dy = a2->v[i].y >> 4;
            dz = a2->v[i].z >> 4;
            a2->p[i].x += dx;
            a2->p[i].y += dy;
            a2->p[i].z += dz;
            a2->v[i].y += 4;
            if (a2->p[i].y >= *lim) {
                a2->f[i] = 0;
                if (--a2->cnt == 0) {
                    a1[1] = two;
                }
            }
        }
    }
}
