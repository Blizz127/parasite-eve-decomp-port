/* room_m0087i (PE.IMG room m0087i chunk 2, VRAM 0x8018EFE8)
 * func_80191CD8 — blob offset 0x2cf0, 0x28 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0022i func_80190CD4; C re-targeted by symbol address
 * (docs/evidence/room_m0087i-ports-2026-09-23/REPORT.md). */

typedef struct Obj {
    int pad0[3];
    int (*fn)();
} Obj;
int func_80191CD8(Obj *o)
{
    o->fn(o);
    return 0;
}
