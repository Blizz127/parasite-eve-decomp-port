/* room_m0318i (PE.IMG room m0318i chunk 2, VRAM 0x8018EFE8)
 * func_80190D4C — blob offset 0x1d64, 0x28 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0022i func_80190CD4; C re-targeted by symbol address
 * (docs/evidence/room_m0318i-ports-2026-09-23/REPORT.md). */

typedef struct Obj {
    int pad0[3];
    int (*fn)();
} Obj;
int func_80190D4C(Obj *o)
{
    o->fn(o);
    return 0;
}
