/* room_m0412i (PE.IMG room m0412i chunk 2, VRAM 0x8018EFE8)
 * func_801929C4 — blob offset 0x39dc, 0x5c bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0022i func_801914A0; C re-targeted by symbol address
 * (docs/evidence/room_m0412i-ports-2026-09-23/REPORT.md). */

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
int func_801929C4(Obj *o)
{
    Sub *s = &o->sub;

    if (o->f14 != 0 && o->ctx->p0 != 0) {
        *o->ctx->p0->f18 = 4;
    }
    o->f3 = 0;
    o->state = 4;
    if (s->link != 0) {
        *s->link = 0;
    }
    return 0;
}
