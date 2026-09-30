/* room_m0153i (PE.IMG room m0153i chunk 2, VRAM 0x8018EFE8)
 * func_801939E0 — blob offset 0x49f8, 0x1c bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0022i func_801924C4; C re-targeted by symbol address
 * (docs/evidence/room_m0153i-ports-2026-09-23/REPORT.md). */

typedef struct {
    unsigned char state;
    unsigned char pad[0xF];
    int *link;
} Obj;
int func_801939E0(Obj *o)
{
    o->state = 4;
    if (o->link != 0) {
        *o->link = 0;
    }
    return 0;
}
