/* room_m0418i — func_80191E78, blob offset 0x2E90, 0x350 bytes. Flags -O2 -G0.
 * Two 4-sprite func_800CEE20 passes over a2 (+8 positions, +0x28 scales x
 * +0x58, +0x38 depths): first with a white tint and doubled z, then with
 * the a2 positions directly and mode 0x80/2.
 * Levers: D_800F3368 as one struct (keeps its stores below the stack copies);
 * store order a6C,a6E,a68,... and a6A second in the second block (sched1 lists);
 * loop 2 reads the table before the scale multiply and holds the compare
 * constant in a block local `four` so loop.c hoists it into $s6 after the
 * table base ($s7), as retail does. No inline asm. */

typedef struct { short x, y; unsigned short z; short w; } SV;
typedef struct { unsigned char r, g, b, cd; } CV;
#define SH(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
extern int D_8019956C, D_8019957C;
extern SV D_8018F040;
extern CV D_8018F048;
extern unsigned short D_800E11EA;
extern unsigned short D_800E2850[];
extern unsigned short D_800E1204[];
extern int D_800F3428;
struct S {
    short a68, a6A;
    unsigned short a6C;
    short a6E;
    unsigned short a70;
    short a72, a74, a76, a78;
};
extern struct S D_800F3368;
extern void func_80071A44();
extern int func_80077AA4();
extern void func_800CEE20();

void func_80191E78(int a0, int a1, char *a2)
{
    SV a;
    SV b;
    CV c;
    unsigned int i;
    int s;
    int w;
    int t;

    func_80071A44(&b, 0, 8);
    b.x = D_8019956C;
    b.y = 0;
    b.z = D_8019957C;
    a = b;
    b = D_8018F040;
    c = D_8018F048;
    t = D_800E2850[D_800E11EA];
    D_800F3368.a6C = 3;
    D_800F3368.a6E = 0;
    D_800F3368.a68 = 0x40;
    D_800F3368.a6A = 4;
    D_800F3368.a76 = 0x40;
    D_800F3368.a78 = 0x40;
    D_800F3368.a74 = 1000;
    D_800F3368.a72 = 0;
    D_800F3368.a70 = t;
    for (i = 0; i < 4; i++) {
        b = *(SV *)(a2 + i * 8 + 8);
        c.r = c.g = c.b = 0xFF;
        b.z = b.z * 2;
        s = (W(a2 + i * 4, 0x28) * SH(a2, 0x58)) >> 12;
        w = D_800E1204[D_800F3368.a6C];
        if (D_800F3368.a6C == 4 && D_800F3428 != 0) {
            w += 4;
        }
        func_800CEE20(&a, &b, s, s, 0, (unsigned short)func_80077AA4(0, w), 1, SH(a2 + i * 2, 0x38), 0);
    }
    D_800F3368.a68 = 0x40;
    D_800F3368.a6A = 4;
    D_800F3368.a76 = 0x40;
    D_800F3368.a78 = 0x40;
    D_800F3368.a74 = 0;
    D_800F3368.a72 = 5;
    for (i = 0; i < 4; i++) {
        int four;
        w = D_800E1204[D_800F3368.a6C];
        four = 4;
        s = (W(a2 + i * 4, 0x28) * SH(a2, 0x58)) >> 12;
        if (D_800F3368.a6C == four && D_800F3428 != 0) {
            w += 4;
        }
        func_800CEE20(&a, a2 + (i * 8 + 8), s, s, 0x80, (unsigned short)func_80077AA4(0x80, w), 2, SH(a2 + i * 2, 0x38), 0);
    }
}
