/* room_m0273i — func_80192F5C, blob offset 0x3F74, 0x18C bytes. Flags -O2 -G0 + MASPSX_EXPAND_DIV=1 (era_o2_g0_expand_div);
 * LINK_EXACT at the target VMA (lane ovl13 2026-09-28).
 * Sine-bob object updater + yaw/distance outputs (func_80079FB4, func_8005186C); base read before the object pointer. */

typedef struct {
    char pad0[0x28];
    int x;
    int pad2C;
    int z;
    char pad34[6];
    short r;
} Obj;
typedef struct {
    int f0;
    int f4;
    Obj *o;
    int *out[2];
    int f14;
    int t;
    int base;
    short on;
    short amp;
    short per;
} St;
typedef struct { short s, c; } SC;
extern SC D_800966EC[];
extern Obj *D_8009D254;
extern int func_80079FB4();
extern int func_8005186C();

int func_80192F5C(St *a)
{
    Obj *o = a->o;
    int **p = a->out;
    int t;
    int q;
    int w;

    if (a->on != 0) {
        t = a->t;
        q = (t << 12) / a->per;
        a->t = t + 1;
        w = (D_800966EC[(q - 0x800) & 0xFFF].c + 0x1000) / 2;
        w = ((w * a->amp) / 4096 + 8) << 12;
        a->o->pad2C = a->base - w;
    }
    if (p[0] != 0) {
        *p[0] = (func_80079FB4(o->x - D_8009D254->x, o->z - D_8009D254->z) - o->r) & 0xFFF;
    }
    if (p[1] != 0) {
        *p[1] = func_8005186C(((D_8009D254->x - o->x) >> 16) * ((D_8009D254->x - o->x) >> 16) +
                              ((D_8009D254->z - o->z) >> 16) * ((D_8009D254->z - o->z) >> 16));
    }
    return 0;
}
