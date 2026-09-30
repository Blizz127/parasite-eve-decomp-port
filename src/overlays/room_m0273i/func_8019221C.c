/* room_m0273i (PE.IMG room m0273i chunk 2, VRAM 0x8018EFE8)
 * func_8019221C — blob offset 0x3234, 0x30 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0022i func_80192200; C re-targeted by symbol address
 * (docs/evidence/room_m0273i-ports-2026-09-23/REPORT.md). */

typedef struct Obj {
    int pad0[2];
    unsigned char *p;
    void (*fn)();
    int f10;
    short f14;
    signed char f16;
} Obj;
extern void func_8019224C();
void func_8019221C(Obj *o)
{
    if (o->p[0xE] == o->f16) {
        o->fn = func_8019224C;
    }
}
