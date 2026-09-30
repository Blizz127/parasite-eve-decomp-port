/* room_m0022i (PE.IMG room m0022i chunk 2, VRAM 0x8018EFE8)
 * func_801921D8 — blob offset 0x31f0, 0x28 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room_m0022i-batch-2026-09-23/REPORT.md). */

typedef struct Obj {
    int pad0[3];
    int (*fn)();
} Obj;
int func_801921D8(Obj *o)
{
    o->fn(o);
    return 0;
}
