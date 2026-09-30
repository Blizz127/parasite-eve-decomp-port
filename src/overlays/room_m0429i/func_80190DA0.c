/* room_m0429i (PE.IMG room m0429i chunk 2, VRAM 0x8018EFE8)
 * func_80190DA0 — blob offset 0x1db8, 0x70 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0022i func_80190CFC; C re-targeted by symbol address
 * (docs/evidence/room_m0429i-ports-2026-09-23/REPORT.md). */

typedef struct Inner {
    unsigned char pad[0x18];
    unsigned char *f18;
} Inner;
typedef struct Ctx {
    Inner *p0;
    unsigned char pad4[0xA];
    unsigned char fE;
    unsigned char padF[7];
    unsigned short f16;
    unsigned short pad18;
    unsigned short f1A;
} Ctx;
typedef struct Sub {
    void (*fn)();
    int *link;
} Sub;
typedef struct Obj {
    unsigned char state;
    unsigned char pad1[2];
    unsigned char f3;
    int pad4;
    Ctx *ctx;
    Sub sub;
    short f14;
    signed char f16, f17;
} Obj;
extern void func_80190E10();
void func_80190DA0(Obj *o)
{
    int v;
    Ctx *c;
    int lo, hi;

    if (o->f16 < 0 || o->f16 == o->ctx->fE) {
        v = o->f17;
        if (v < 0) {
            o->sub.fn = func_80190E10;
        } else {
            c = o->ctx;
            lo = c->f1A;
            hi = c->f16;
            if (lo < v && v <= hi) {
                o->sub.fn = func_80190E10;
            }
        }
    }
}
