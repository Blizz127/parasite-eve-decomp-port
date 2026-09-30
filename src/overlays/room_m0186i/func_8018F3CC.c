/* room_m0186i (PE.IMG room m0186i chunk 2, VRAM 0x8018EFE8)
 * func_8018F3CC — blob offset 0x3e4, 0x144 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0167i func_8018F3AC; C re-targeted by symbol address
 * (docs/evidence/room_m0186i-ports-2026-09-23/REPORT.md). */

#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))

typedef struct { int w[8]; } S32;
extern unsigned char D_80194360, D_80194361, D_80194362, D_80194364, D_80194365, D_80194366;
extern unsigned char D_80194380, D_80194381, D_80194382, D_80194384, D_80194385, D_80194386;
extern short D_80194368, D_8019436A, D_80194388, D_8019438A;
extern void func_800C2B40();
extern int func_8006DC18();

void func_8018F3CC(void *a0, int a1, void *a2)
{
    void *x;

    func_800C2B40(a2);
    W(a2, 0x24) = func_8006DC18(0xB);
    x = P(a0, 8);
    P(a2, 0) = x;
    *(S32 *)((char *)a2 + 4) = *(S32 *)P(x, 0x238);
    H(a2, 0x28) = 0x28;
    H(a2, 0x2A) = 0;
    H(a2, 0x2C) = 0;
    D_80194384 = 4;
    D_80194385 = 1;
    D_8019438A = 0x80;
    D_80194388 = 0;
    D_80194380 = 0x80;
    D_80194364 = 8;
    D_80194365 = 2;
    D_80194381 = 0x80;
    D_80194382 = 0x80;
    D_80194386 = 0;
    D_80194368 = 0;
    D_8019436A = 0x30;
    D_80194360 = 0x80;
    D_80194361 = 0x80;
    D_80194362 = 0x80;
    D_80194366 = 0;
}
