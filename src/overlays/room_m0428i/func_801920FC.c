/* room_m0428i (PE.IMG room m0428i chunk 2, VRAM 0x8018EFE8)
 * func_801920FC — blob offset 0x3114, 0x3c bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0022i func_801920FC; C re-targeted by symbol address
 * (docs/evidence/room_m0428i-ports-2026-09-23/REPORT.md). */

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
extern void func_80192230();
int func_801920FC(Obj *o)
{
    o->f16 = -1;
    o->f17 = -1;
    o->f18 = -1;
    o->f19 = 3;
    o->fn = func_80192230;
    o->f10 = 0;
    o->f14 = 0;
    o->f1A = 0;
    o->f46 = 0;
    return 0;
}
