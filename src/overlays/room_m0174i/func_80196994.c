/* room_m0174i (PE.IMG room m0174i chunk 2, VRAM 0x8018EFE8)
 * func_80196994 — blob offset 0x79ac, 0x30 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0022i func_80192200; C re-targeted by symbol address
 * (docs/evidence/room_m0174i-ports-2026-09-23/REPORT.md). */

typedef struct Obj {
    int pad0[2];
    unsigned char *p;
    void (*fn)();
    int f10;
    short f14;
    signed char f16;
} Obj;
extern void func_801969C4();
void func_80196994(Obj *o)
{
    if (o->p[0xE] == o->f16) {
        o->fn = func_801969C4;
    }
}
