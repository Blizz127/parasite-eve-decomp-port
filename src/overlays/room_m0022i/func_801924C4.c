/* room_m0022i (PE.IMG room m0022i chunk 2, VRAM 0x8018EFE8)
 * func_801924C4 — blob offset 0x34DC, 0x1C bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room_m0022i-func_801924C4/REPORT.md).
 * Set the object's state byte to 4 and clear the word its +0x10 link points at (if any); returns 0. */

typedef struct {
    unsigned char state;
    unsigned char pad[0xF];
    int *link;
} Obj;
int func_801924C4(Obj *o)
{
    o->state = 4;
    if (o->link != 0) {
        *o->link = 0;
    }
    return 0;
}
