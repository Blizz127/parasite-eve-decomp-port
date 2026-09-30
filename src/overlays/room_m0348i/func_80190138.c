/* room_m0348i (PE.IMG room m0348i chunk 2, VRAM 0x8018EFE8)
 * func_80190138 — blob offset 0x1150, 0x94 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0174i func_80191250; C re-targeted by symbol address
 * (docs/evidence/room_m0348i-ports-2026-09-23/REPORT.md). */

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
void func_80190138(int a0, int a1, Fx *p)
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
