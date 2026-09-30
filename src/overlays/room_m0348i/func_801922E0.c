/* room_m0348i (PE.IMG room m0348i chunk 2, VRAM 0x8018EFE8)
 * func_801922E0 — blob offset 0x32f8, 0x90 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0174i func_801933F8; C re-targeted by symbol address
 * (docs/evidence/room_m0348i-ports-2026-09-23/REPORT.md). */

typedef struct { short x, y, z, pad; } SV;
typedef struct {
    unsigned char pad0[0x30];
    SV pos[4];
    SV vel[4];
    unsigned char pad70[8];
    short fade[4];
} Fx;
void func_801922E0(int a0, unsigned char *q, Fx *p)
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
