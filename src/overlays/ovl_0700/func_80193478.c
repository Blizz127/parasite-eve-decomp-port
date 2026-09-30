/* ovl_0700 (PE.IMG subsystem overlay, VRAM 0x8018EFF0)
 * func_80193478 — blob offset 0x4488, 0x470 bytes. Profile era_o2_g0 (default).
 * Per-frame camera/object path step: four func_8018F55C spline samples, roll clamp and
 * decay, yaw spin, then copies the two driven objects' MATRIX+SVECTOR into their shadows.
 * Lever: the six object pointers are one array `Obj *D_801EA578[6]` — the in-struct
 * (array) global load then conflicts with the Obj field stores, so cc1 reloads the pointer
 * after every store as retail does (six scalar pointer globals: 256 diff lines).
 * Evidence: docs/evidence/ovl8-lane-2026-09-28/REPORT.md */
typedef struct { short vx, vy, vz, pad; } SVECTOR;
typedef struct { int vx, vy, vz; } LVEC;
typedef struct {
    short m[3][3];
    int t[3];
} MATRIX;
typedef struct {
    int pad[2];
    MATRIX mat;
    SVECTOR rot;
} Obj;

extern unsigned char D_801D0260[];
extern Obj *D_801EA578[6];
extern int D_8019C038;
extern int D_8019C03C;
extern int func_8006EC6C();
extern int func_8018F55C();

void func_80193478(void)
{
    SVECTOR ra;
    SVECTOR rb;

    func_8018F55C(D_8019C038, 0x4A, func_8006EC6C(D_801D0260, 2), D_801EA578[4]->mat.t, &D_801EA578[4]->rot);
    func_8018F55C(D_8019C038, 0x4B, func_8006EC6C(D_801D0260, 2), D_801EA578[5]->mat.t, &D_801EA578[5]->rot);
    func_8018F55C(D_8019C03C, 0x4C, func_8006EC6C(D_801D0260, 2), D_801EA578[0]->mat.t, &ra);
    func_8018F55C(D_8019C03C, 0x4D, func_8006EC6C(D_801D0260, 2), D_801EA578[2]->mat.t, &rb);
    D_801EA578[0]->rot.vx = ra.vx;
    D_801EA578[0]->rot.vy = ra.vy;
    D_801EA578[0]->rot.vz += ra.vz;
    if (D_801EA578[0]->rot.vz > 0x200) {
        D_801EA578[0]->rot.vz = 0x200;
    }
    if (D_801EA578[0]->rot.vz < -0x200) {
        D_801EA578[0]->rot.vz = -0x200;
    }
    if (D_801EA578[0]->rot.vz > 0) {
        D_801EA578[0]->rot.vz -= 8;
    }
    if (D_801EA578[0]->rot.vz < 0) {
        D_801EA578[0]->rot.vz += 8;
    }
    D_801EA578[2]->rot.vx = rb.vx;
    D_801EA578[2]->rot.vy = rb.vy;
    D_801EA578[2]->rot.vz += rb.vz;
    if (D_801EA578[2]->rot.vz > 0x180) {
        D_801EA578[2]->rot.vz = 0x180;
    }
    if (D_801EA578[2]->rot.vz < -0x180) {
        D_801EA578[2]->rot.vz = -0x180;
    }
    if (D_801EA578[2]->rot.vz > 0) {
        D_801EA578[0]->rot.vz -= 8;
    }
    if (D_801EA578[2]->rot.vz < 0) {
        D_801EA578[0]->rot.vz += 8;
    }
    D_801EA578[4]->rot.vy -= 0x400;
    D_801EA578[5]->rot.vy -= 0x400;
    D_801EA578[0]->rot.vy -= 0x400;
    D_801EA578[2]->rot.vy -= 0x400;
    D_801EA578[1]->mat = D_801EA578[0]->mat;
    D_801EA578[1]->rot = D_801EA578[0]->rot;
    D_801EA578[3]->mat = D_801EA578[2]->mat;
    D_801EA578[3]->rot = D_801EA578[2]->rot;
    D_8019C038 += 4;
    D_8019C03C += 16;
}
