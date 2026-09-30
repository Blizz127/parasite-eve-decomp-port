/* room_m0418i (PE.IMG room m0418i chunk 2, VRAM 0x8018EFE8)
 * func_80198288 — blob offset 0x92a0, 0x3c bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0022i func_801920FC; C re-targeted by symbol address
 * (docs/evidence/room_m0418i-ports-2026-09-23/REPORT.md). */

typedef struct Obj {
    int pad0[3];
    void (*fn)();
    int f10;
    short f14;
    signed char f16, f17, f18, f19;
    unsigned char f1A;
    unsigned char pad1B[0x2B];
    short f46;
} Obj;
extern void func_801983BC();
int func_80198288(Obj *o)
{
    o->f16 = -1;
    o->f17 = -1;
    o->f18 = -1;
    o->f19 = 3;
    o->fn = func_801983BC;
    o->f10 = 0;
    o->f14 = 0;
    o->f1A = 0;
    o->f46 = 0;
    return 0;
}
