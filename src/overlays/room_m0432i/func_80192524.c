/* room_m0432i (PE.IMG room m0432i chunk 2, VRAM 0x8018EFE8)
 * func_80192524 — blob offset 0x353c, 0x1c bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0022i func_801924C4; C re-targeted by symbol address
 * (docs/evidence/room_m0432i-ports-2026-09-23/REPORT.md). */

typedef struct {
    unsigned char state;
    unsigned char pad[0xF];
    int *link;
} Obj;
int func_80192524(Obj *o)
{
    o->state = 4;
    if (o->link != 0) {
        *o->link = 0;
    }
    return 0;
}
