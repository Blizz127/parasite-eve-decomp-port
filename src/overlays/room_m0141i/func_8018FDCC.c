/* room_m0141i (PE.IMG room m0141i chunk 2, VRAM 0x8018EFE8)
 * func_8018FDCC — blob offset 0xDE4, 0xDC bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room-lane-2026-09-23/REPORT.md).
 * Particle burst step for 16 particles (spread by +0x106, rise +8, fade via +0x104/+0x105). */

typedef struct { short x, y, z, pad; } SV;
typedef struct {
    SV pos[16];
    SV dir[16];
    short f100, f102;
    unsigned char f104, f105;
    short f106, f108;
} Fx;
void func_8018FDCC(int a0, unsigned char *q, Fx *p)
{
    unsigned int i;
    short t;
    unsigned int k;

    p->f106 += p->f108;
    t = p->f102;
    if (t > 16) {
        p->f102 = t - 16;
    }
    for (i = 0; i < 16; i++) {
        p->pos[i].x += (p->dir[i].x * p->f106) >> 16;
        p->pos[i].y += 8;
        p->pos[i].z += (p->dir[i].z * p->f106) >> 16;
    }
    p->f105 += 6;
    k = p->f105 >> 4;
    p->f104 = k + 1;
    if (k == 7) {
        p->f104 = 0;
        q[1] = 2;
    }
}
