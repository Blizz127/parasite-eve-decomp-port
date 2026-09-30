/* room_m0414i (PE.IMG room m0414i chunk 2, VRAM 0x8018EFE8)
 * func_80190080 — blob offset 0x1098, 0x1c4 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0141i func_801900B4; C re-targeted by symbol address
 * (docs/evidence/room_m0414i-ports-2026-09-23/REPORT.md). */

typedef struct { short x, y, z, pad; } SV;
typedef struct {
    SV trail[6];
    SV vel[6];
    int grav[6];
    unsigned char pad78[0];
    short alpha[6];
    unsigned char pad84[0];
    short f84, f86, f88;
} Fx;
extern int *func_800C2B50();
extern void func_800C2B28();
extern int func_80071A54();
extern int func_80077CF4();
extern int func_80077DC4();
void func_80190080(int a0, int a1, Fx *p)
{
    int *r = func_800C2B50();
    int a;
    unsigned int i;

    func_800C2B28(0);
    a = func_80071A54() % 4096;
    for (i = 0; i < 6; i++) {
        p->trail[i].x = r[6];
        p->trail[i].y = r[7];
        p->trail[i].z = r[8];
        p->vel[i].x = (func_80077CF4(a) * (func_80071A54() % 2048 + 2048)) >> 12;
        p->vel[i].y = -(func_80071A54() % 6096 + 3000);
        p->vel[i].z = (func_80077DC4(a) * (func_80071A54() % 2048 + 2048)) >> 12;
        p->grav[i] = 500;
        p->alpha[i] = 1;
    }
    p->f86 = 128;
    p->f84 = 1024;
    p->f88 = 800;
}
