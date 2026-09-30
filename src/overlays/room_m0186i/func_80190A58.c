/* room_m0186i — func_80190A58, blob offset 0x1A70, 0x1B4 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl5 2026-09-27).
 * Spawn-point init: 4 SVECTOR locals, ApplyMatrix-style func_80078C34 pair; SVECTOR out (not VECTOR: frame size); store order via climb. */

typedef struct { int vx, vy, vz, pad; } VECTOR;
typedef struct { short vx, vy, vz, pad; } SVECTOR;
extern SVECTOR D_8018F014;
extern SVECTOR D_8018F01C;
extern SVECTOR D_8018F024;
extern SVECTOR D_8018F02C;
extern short D_800942EC;
extern unsigned char *func_800C2B50();
extern int *func_800C2B10();
extern void func_80078C34();

#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))

void func_80190A58(int a0, int a1, unsigned char *o)
{
    SVECTOR a, b, c, d;
    SVECTOR out;
    unsigned char *x;

    x = func_800C2B50();
    a = D_8018F014;
    b = D_8018F01C;
    c = D_8018F024;
    d = D_8018F02C;
    if (*func_800C2B10(1) == 0) {
        func_80078C34(x + 4, &a, o + 8);
        func_80078C34(x + 4, &b, &out);
    } else {
        func_80078C34(x + 4, &c, o + 8);
        func_80078C34(x + 4, &d, &out);
    }
    H(o, 0) = out.vx + W(x, 0x18);
    H(o, 2) = D_800942EC;
    H(o, 4) = out.vz + W(x, 0x20);
    H(o, 0x12) = 0x80;
    H(o, 0x10) = 0x800;
    H(o, 0x14) = 0x258;
    H(o, 0x16) = 0;
}
