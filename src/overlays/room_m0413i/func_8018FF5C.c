/* room_m0413i (PE.IMG room m0413i chunk 2, VRAM 0x8018EFE8)
 * func_8018FF5C — blob offset 0xf74, 0x7c bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0022i func_8018FF44; C re-targeted by symbol address
 * (docs/evidence/room_m0413i-ports-2026-09-23/REPORT.md). */

typedef struct Obj {
    unsigned char state;
    unsigned char pad1[2];
    unsigned char f3;
    int pad4[2];
    void (*fn)();
    int f10;
    short f14;
    signed char f16, f17, f18, f19;
    unsigned char f1A;
    unsigned char pad1B[0x31];
    int f4C, f50, f54;
    int pad58[5];
    int f6C, f70, f74;
    short pad78;
    short f7A;
    short pad7C[2];
    short f80, f82, f84, f86, f88;
    short pad8A[3];
    unsigned char f90, f91;
} Obj;
extern void func_80190388();
int func_8018FF5C(Obj *o)
{
    o->f3 = 1;
    o->f16 = -1;
    o->f17 = -1;
    o->f18 = -1;
    o->f19 = 3;
    o->f74 = 0x10000;
    o->fn = func_80190388;
    o->f10 = 0;
    o->f14 = 0;
    o->f1A = 0;
    o->f4C = 0;
    o->f50 = 0;
    o->f54 = 0;
    o->f6C = 0;
    o->f70 = 0;
    o->f7A = 0;
    o->f80 = 0;
    o->f82 = 0;
    o->f84 = 0;
    o->f86 = 0;
    o->f88 = 0;
    o->f90 = 0;
    o->f91 = 0;
    return 0;
}
