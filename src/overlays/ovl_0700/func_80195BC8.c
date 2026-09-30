/* ovl_0700 (PE.IMG subsystem overlay, VRAM 0x8018EFF0)
 * func_80195BC8 — blob offset 0x6BD8, 0x174 bytes. Profile era_o2_g0 (default).
 * Fills two camera-relative records from two SVECTOR positions. Levers: SVECTOR struct
 * pointer params + separate scalar short globals for the mode/shift words (a fixed-address
 * scalar store may then move past the struct-pointer loads, as retail's sched did) + separate
 * Obj struct globals; int params masked in place; shift stored after the mask
 * (`s = a; m = 1 << a`). Evidence: docs/evidence/ovl8-lane-2026-09-28/REPORT.md */
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

void func_80195BC8(SVECTOR *a0, SVECTOR *a1, int a2, int a3)
{
    a2 &= 0xFF;
    a3 &= 0xFF;
    D_8019C08C.d[0] = a1->vx - D_8019C810[0];
    D_8019C08C.d[1] = a1->vy - D_8019C810[1];
    D_8019C08C.d[2] = a1->vz - D_8019C810[2];
    D_8019C08C.z[0] = 0;
    D_8019C08C.z[1] = 0;
    D_8019C08C.z[2] = 0;
    D_8019C08C.c[0] = D_8019C810[0];
    D_8019C08C.c[1] = D_8019C810[1];
    D_8019C08C.c[2] = D_8019C810[2];
    D_8019C08C.r[0] = a1->vx;
    D_8019C08C.r[1] = a1->vy;
    D_8019C08C.r[2] = a1->vz;
    D_8019C056 = a2;
    D_8019C054 = 1 << a2;
    D_8019C05C.d[0] = a0->vx - D_8019C330[0];
    D_8019C05C.d[1] = a0->vy - D_8019C330[1];
    D_8019C05C.d[2] = a0->vz - D_8019C330[2];
    D_8019C05C.z[0] = 0;
    D_8019C05C.z[1] = 0;
    D_8019C05C.z[2] = 0;
    D_8019C05C.c[0] = D_8019C330[0];
    D_8019C05C.c[1] = D_8019C330[1];
    D_8019C05C.c[2] = D_8019C330[2];
    D_8019C05C.r[0] = a0->vx;
    D_8019C05C.r[1] = a0->vy;
    D_8019C05C.r[2] = a0->vz;
    D_8019C052 = a3;
    D_8019C050 = 1 << a3;
}
