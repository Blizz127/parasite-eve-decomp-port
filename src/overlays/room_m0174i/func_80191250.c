/* room_m0174i (PE.IMG room m0174i chunk 2, VRAM 0x8018EFE8)
 * func_80191250 — blob offset 0x2268, 0x94 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room-lane-2026-09-23/REPORT.md).
 * Init 8 particle slots (flags 1, alpha 128, height from +2) and the fall vector. */

typedef struct { int w0, w4, w8, wC; } Q4;
typedef struct {
    Q4 q[8];
    unsigned char pad80[8];
    short h88[8];
    unsigned char pad98[0x10];
    int wA8[8];
    unsigned char bC8[8];
    short hD0[8];
    int wE0;
    int wE4;
    short padE8;
    short hEA;
} Fx;
extern int *func_800C2B50();
void func_80191250(int a0, int a1, Fx *p)
{
    int *r = func_800C2B50();
    unsigned int i;

    p->hEA = 128;
    p->wE0 = 0xFFEC0000;
    p->wE4 = r[22] << 8;
    for (i = 0; i < 8; i++) {
        p->h88[i] = 0;
        p->wA8[i] = 0;
        p->q[i].wC = ((short *)p)[1] << 16;
        p->bC8[i] = 1;
        p->hD0[i] = 128;
    }
}
