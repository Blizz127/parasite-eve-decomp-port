/* room_m0153i (PE.IMG room m0153i chunk 2, VRAM 0x8018EFE8)
 * func_8019371C — blob offset 0x4734, 0x30 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0022i func_80192200; C re-targeted by symbol address
 * (docs/evidence/room_m0153i-ports-2026-09-23/REPORT.md). */

typedef struct Obj {
    int pad0[2];
    unsigned char *p;
    void (*fn)();
    int f10;
    short f14;
    signed char f16;
} Obj;
extern void func_8019374C();
void func_8019371C(Obj *o)
{
    if (o->p[0xE] == o->f16) {
        o->fn = func_8019374C;
    }
}
