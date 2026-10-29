/* room_m0159i (PE.IMG room m0159i chunk 2, VRAM 0x8018EFE8)
 * func_8018FDF8 — blob offset 0xe10, 0x44 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0104i func_8018FDF8; C re-targeted by symbol address
 * (docs/evidence/room_m0159i-ports-2026-09-23/REPORT.md). */

typedef struct Slot {
    volatile short h0;
    short h2, h4, h6;
    int w8, wC;
} Slot;
extern Slot D_8018FE94[];
extern volatile unsigned short D_800942EC;
Slot *func_8018FDF8(int a0, int a1, int a2, int a3)
{
    Slot *p = &D_8018FE94[a1];

    if (a0 == 1) {
        p->w8 = a2;
        p->wC = a3;
    } else {
        p->h0 = a2;
        p->h2 = D_800942EC;
        p->h4 = a3;
    }
    return p;
}
