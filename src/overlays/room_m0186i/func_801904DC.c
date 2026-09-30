/* room_m0186i (PE.IMG room m0186i chunk 2, VRAM 0x8018EFE8)
 * func_801904DC — blob offset 0x14F4, 0xF8 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room-lane-2026-09-23/REPORT.md).
 * Particle burst step: spread 8 particles along their directions, gravity 600, fade-out via +0x84/+0x85. */

typedef struct { short x, y, z, pad; } SV;
typedef struct {
    SV pos[8];
    SV dir[8];
    short f80, f82;
    unsigned char f84, f85;
    short f86, f88;
} Fx;
void func_801904DC(int a0, unsigned char *q, Fx *p)
{

    unsigned int i;
    short t;
    unsigned int k;

    p->f86 += p->f88;
    t = p->f82;
    if (t > 8) {
        p->f82 = t - 8;
    }
    for (i = 0; i < 8; i++) {
        p->pos[i].x += (p->dir[i].x * p->f86) >> 16;
        p->pos[i].y += p->dir[i].y >> 8;
        p->pos[i].z += (p->dir[i].z * p->f86) >> 16;
        p->dir[i].y += 600;
    }
    p->f80 += 70;
    p->f85 += 6;
    k = p->f85 >> 4;
    p->f84 = k + 1;
    if (k == 7) {
        p->f84 = 0;
        q[1] = 2;
    }
}
