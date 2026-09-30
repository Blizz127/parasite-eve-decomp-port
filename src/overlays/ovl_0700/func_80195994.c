/* ovl_0700 (PE.IMG subsystem overlay, VRAM 0x8018EFF0)
 * func_80195994 — blob offset 0x69A4, 0x234 bytes. Profile era_o2_g0 (default).
 * Camera-record fill split by two func_8018F55C calls (sibling of func_80195BC8). Levers:
 * mode/shift words are separate scalar short globals (D_8019C050..56) and the two records
 * separate Obj struct globals; prototyped `unsigned char` a1/a2 copied into int locals right
 * after each call (in-place `a &= 0xFF` lets sched1 hoist the andi above the calls; `a & 0xFF`
 * into a new local leaves p/a2 callee-saved order swapped). Evidence:
 * docs/evidence/ovl8-lane-2026-09-28/REPORT.md */
typedef struct { short vx, vy, vz, pad; } SVECTOR;
typedef struct {
    int d[3];
    int pad0;
    int z[3];
    int pad1;
    short c[3];
    short pad2;
    short r[3];
    short pad3;
} Obj;

extern short D_8019C050;
extern short D_8019C052;
extern short D_8019C054;
extern short D_8019C056;
extern Obj D_8019C05C;
extern Obj D_8019C08C;
extern int D_8019C810[3];
extern int D_8019C330[3];

typedef struct {
    unsigned char a, b;
    unsigned char pad[50];
} Rec;

extern Rec D_801EA378[];
extern unsigned char D_801D0260[];
extern int func_8006EC6C();
extern void func_8018F55C();

void func_80195994(short a0, unsigned char a1, unsigned char a2, int a3)
{
    int v[3];
    int buf[2];
    unsigned char *p;
    int s1, s0;

    p = D_801D0260;

    func_8018F55C(a3, D_801EA378[a0].a, func_8006EC6C(p, 2), v, buf);
    s1 = a1;
    D_8019C056 = s1;
    D_8019C08C.z[0] = 0;
    D_8019C08C.z[1] = 0;
    D_8019C08C.z[2] = 0;
    D_8019C054 = 1 << s1;
    D_8019C08C.d[0] = v[0] - D_8019C810[0];
    D_8019C08C.d[1] = v[1] - D_8019C810[1];
    D_8019C08C.d[2] = v[2] - D_8019C810[2];
    D_8019C08C.c[0] = D_8019C810[0];
    D_8019C08C.c[1] = D_8019C810[1];
    D_8019C08C.c[2] = D_8019C810[2];
    D_8019C08C.r[0] = v[0];
    D_8019C08C.r[1] = v[1];
    D_8019C08C.r[2] = v[2];
    func_8018F55C(a3, D_801EA378[a0].b, func_8006EC6C(p, 2), v, buf);
    s0 = a2;
    D_8019C05C.z[0] = 0;
    D_8019C05C.z[1] = 0;
    D_8019C05C.z[2] = 0;
    D_8019C052 = s0;
    D_8019C050 = 1 << s0;
    D_8019C05C.d[0] = v[0] - D_8019C330[0];
    D_8019C05C.d[1] = v[1] - D_8019C330[1];
    D_8019C05C.d[2] = v[2] - D_8019C330[2];
    D_8019C05C.c[0] = D_8019C330[0];
    D_8019C05C.c[1] = D_8019C330[1];
    D_8019C05C.c[2] = D_8019C330[2];
    D_8019C05C.r[0] = v[0];
    D_8019C05C.r[1] = v[1];
    D_8019C05C.r[2] = v[2];
}
