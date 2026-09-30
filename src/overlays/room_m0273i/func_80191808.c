/* room_m0273i (PE.IMG room m0273i chunk 2, VRAM 0x8018EFE8)
 * func_80191808 — blob offset 0x2820, 0x28 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0022i func_80190CD4; C re-targeted by symbol address
 * (docs/evidence/room_m0273i-ports-2026-09-23/REPORT.md). */

typedef struct Obj {
    int pad0[3];
    int (*fn)();
} Obj;
int func_80191808(Obj *o)
{
    o->fn(o);
    return 0;
}
