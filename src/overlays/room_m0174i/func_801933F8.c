/* room_m0174i (PE.IMG room m0174i chunk 2, VRAM 0x8018EFE8)
 * func_801933F8 — blob offset 0x4410, 0x90 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room-lane-2026-09-23/REPORT.md).
 * Advance 4 particles by velocity, fade -8 (floor 0); done after frame 61. */

typedef struct { short x, y, z, pad; } SV;
typedef struct {
    unsigned char pad0[0x30];
    SV pos[4];
    SV vel[4];
    unsigned char pad70[8];
    short fade[4];
} Fx;
void func_801933F8(int a0, unsigned char *q, Fx *p)
{
    unsigned int i;

    for (i = 0; i < 4; i++) {
        p->pos[i].x += p->vel[i].x;
        p->pos[i].y += p->vel[i].y;
        p->pos[i].z += p->vel[i].z;
        p->fade[i] -= 8;
        if (p->fade[i] < 0) {
            p->fade[i] = 0;
        }
    }
    if (*(short *)(q + 2) >= 61) {
        q[1] = 2;
    }
}
