/* room_m0022i (PE.IMG room m0022i chunk 2, VRAM 0x8018EFE8)
 * func_80192200 — blob offset 0x3218, 0x30 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room_m0022i-batch-2026-09-23/REPORT.md). */

typedef struct Obj {
    int pad0[2];
    unsigned char *p;
    void (*fn)();
    int f10;
    short f14;
    signed char f16;
} Obj;
extern void func_80192230();
void func_80192200(Obj *o)
{
    if (o->p[0xE] == o->f16) {
        o->fn = func_80192230;
    }
}
