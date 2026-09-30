/* room_m0116i (PE.IMG room m0116i chunk 2, VRAM 0x8018EFE8)
 * func_801932EC — blob offset 0x4304, 0x44 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0104i func_8018FDF8; C re-targeted by symbol address
 * (docs/evidence/room_m0116i-ports-2026-09-23/REPORT.md). */

typedef struct Slot {
    volatile short h0;
    short h2, h4, h6;
    int w8, wC;
} Slot;
extern Slot D_80193430[];
extern volatile unsigned short D_800942EC;
Slot *func_801932EC(int a0, int a1, int a2, int a3)
{
    Slot *p = &D_80193430[a1];

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
