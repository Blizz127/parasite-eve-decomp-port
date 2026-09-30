/* room_m0273i (PE.IMG room m0273i chunk 2, VRAM 0x8018EFE8)
 * func_8018F51C — blob offset 0x534, 0x88 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0022i func_8018F500; C re-targeted by symbol address
 * (docs/evidence/room_m0273i-ports-2026-09-23/REPORT.md). */

typedef struct Ctx {
    unsigned char pad[0xE];
    unsigned char fE;
} Ctx;
typedef struct Obj {
    int pad0[2];
    Ctx *ctx;
    void (*fn)();
} Obj;
extern int func_800DFB78();
extern int func_8018FEB4();
int func_8018F51C(Obj *o)
{
    int r = func_800DFB78(o);

    if (r != 1) {
        if (r < 2) {
            if (r != 0) {
                return 0;
            }
            if (o->ctx->fE < 2) {
                return 0;
            }
            o->fn(o);
            return 0;
        }
    } else {
        func_8018FEB4(o);
    }
    return 0;
}
