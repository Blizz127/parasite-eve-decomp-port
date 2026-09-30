/* room_m0273i (PE.IMG room m0273i chunk 2, VRAM 0x8018EFE8)
 * func_8019A70C — blob offset 0xB724, 0x14 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room-lane-2026-09-23/REPORT.md).
 * Store a1 into D_8019AF6C.f2E through a field pointer; return the record. */

typedef struct {
    unsigned char pad[0x2E];
    short f2E;
} Rec;
extern Rec D_8019AF6C;
Rec *func_8019A70C(int a0, int a1)
{
    short *q = &D_8019AF6C.f2E;

    *q = a1;
    return &D_8019AF6C;
}
