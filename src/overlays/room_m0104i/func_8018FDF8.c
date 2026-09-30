/* room_m0104i (PE.IMG room m0104i chunk 2, VRAM 0x8018EFE8)
 * func_8018FDF8 — blob offset 0xE10, 0x44 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room-lane-2026-09-23/REPORT.md).
 * 16-byte slot setter; volatile h0 + volatile D_800942EC keep retail's store-before-load order. */

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
