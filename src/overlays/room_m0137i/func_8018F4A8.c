/* room_m0137i (PE.IMG room m0137i chunk 2, VRAM 0x8018EFE8)
 * func_8018F4A8 — blob offset 0x4C0, 0x1C0 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room-lane-2026-09-23/REPORT.md).
 * Spawn a spark at the linked model's transformed offset D_8018EFF4, with random or model-derived velocity; register via func_800C6800(o, 1411, p). */

#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
typedef struct { short vx, vy, vz, pad; } SVECTOR;
extern void **func_800C2B50();
extern void func_80078C34();
extern int *func_800C2B10();
extern int func_80071A54();
extern void func_800C6800();
extern SVECTOR D_8018EFF4;
extern SVECTOR D_8018EFFC;
void func_8018F4A8(void *o, int a1, short *p)
{
    void **r = func_800C2B50();
    SVECTOR a = D_8018EFF4;
    SVECTOR out;
    SVECTOR b = D_8018EFFC;
    SVECTOR out2;
    void *m = P(*r, 0x238);
    void *mat = (char *)m + 0xA0;

    func_80078C34(mat, &a, &out);
    p[0] = out.vx + W(m, 0xB4);
    p[1] = out.vy + W(m, 0xB8);
    p[2] = out.vz + W(m, 0xBC);
    p[8] = 256;
    p[9] = 128;
    p[10] = 0;
    ((unsigned char *)p)[0x16] = 0;
    if (*func_800C2B10(1) == 0) {
        p[4] = func_80071A54() % 10 - 5;
        p[5] = 0;
        p[6] = func_80071A54() % 10 - 5;
    } else {
        func_80078C34(mat, &b, &out2);
        p[4] = out2.vx << 1;
        p[5] = 0;
        p[6] = out2.vz << 1;
    }
    func_800C6800(o, 1411, p);
}
