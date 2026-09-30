/*
 * func_80065260 — two-stage weighted ratio update (retail 0x80065260).
 *
 * VRAM 0x80065260 / file 0x55A60 / size 0x10C (67 words). Carved out of the
 * [0x556B4, asm] run.
 *
 * era -O2 -G0 + MASPSX_EXPAND_DIV=1 (the two signed `div $zero,..` blocks).
 * The f34 load must precede the NULL check, exactly as retail.
 */
typedef struct {
    char pad_00[0x38];
    int f38;
    int f3C;
    int f40;
    char pad_44[0x14];
    int f58;
    int f5C;
    int f60;
} Rec;

typedef struct {
    char pad_00[0x34];
    Rec *f34;
    int f38;
    int f3C;
} Obj;

void func_80065260(Obj *arg0) {
    register Rec *a1 asm("$5");
    register int v1 asm("$3");
    int a2;
    int v0;

    a1 = arg0->f34;
    if (arg0 == 0) {
        return;
    }
    v1 = a1->f38;
    a2 = a1->f58;
    if (v1 < a2) {
        v0 = a1->f40;
        arg0->f3C = v0 * v1 * v1 / a2;
        v1 = a1->f40;
        a2 = v1 * a1->f5C;
        v0 = a2 - a1->f60;
        a2 = v1 * a1->f38 * v0;
        v0 = a1->f58 * v1;
        arg0->f38 = a2 / v0;
    }
}
