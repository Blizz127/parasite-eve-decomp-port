/* room_m0186i (PE.IMG room m0186i chunk 2, VRAM 0x8018EFE8)
 * func_80190254 — blob offset 0x126C, 0x130 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room-lane-2026-09-23/REPORT.md).
 * Burst init: 8 particles at the func_800C2B50() point on the floor (D_800942EC), random direction and upward speed; volatile x field + volatile floor keep retail's store-then-load order. */

typedef struct { short x, y, z, pad; } SV;
typedef struct { volatile short x; short y, z, pad; } PV;
typedef struct {
    PV pos[8];
    SV dir[8];
    short f80, f82;
    unsigned char f84, f85;
    short f86, f88;
} Fx;
extern int *func_800C2B50();
extern int func_80071A54();
extern int func_80077CF4();
extern int func_80077DC4();
extern volatile short D_800942EC;
void func_80190254(int a0, int a1, Fx *p)
{
    int *r = func_800C2B50();
    unsigned int i;

    for (i = 0; i < 8; i++) {
        p->pos[i].x = r[6];
        p->pos[i].y = D_800942EC;
        p->pos[i].z = r[8];
        p->dir[i].x = func_80077CF4(func_80071A54() % 4096);
        p->dir[i].z = func_80077DC4(func_80071A54() % 4096);
        p->dir[i].y = -(func_80071A54() % 8000 + 4000);
    }
    p->f82 = 128;
    p->f80 = 1024;
    p->f88 = -30;
    p->f85 = 0;
    p->f86 = 600;
}
