/* room_m0187i (PE.IMG room m0187i chunk 2, VRAM 0x8018EFE8)
 * func_8018FBE4 — blob offset 0xbfc, 0x1b4 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0186i func_80190A58; C re-targeted by symbol address
 * (docs/evidence/room_m0187i-ports-2026-09-23/REPORT.md). */

typedef struct { int vx, vy, vz, pad; } VECTOR;
typedef struct { short vx, vy, vz, pad; } SVECTOR;
extern SVECTOR D_8018EFF4;
extern SVECTOR D_8018EFFC;
extern SVECTOR D_8018F004;
extern SVECTOR D_8018F00C;
extern short D_800942EC;
extern unsigned char *func_800C2B50();
extern int *func_800C2B10();
extern void func_80078C34();

#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))

void func_8018FBE4(int a0, int a1, unsigned char *o)
{
    SVECTOR a, b, c, d;
    SVECTOR out;
    unsigned char *x;

    x = func_800C2B50();
    a = D_8018EFF4;
    b = D_8018EFFC;
    c = D_8018F004;
    d = D_8018F00C;
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
