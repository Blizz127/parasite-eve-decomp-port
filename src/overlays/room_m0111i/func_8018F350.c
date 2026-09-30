/* room_m0111i (PE.IMG room m0111i chunk 2, VRAM 0x8018EFE8)
 * func_8018F350 — blob offset 0x368, 0x1b0 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0107i func_8018F528; C re-targeted by symbol address
 * (docs/evidence/room_m0111i-ports-2026-09-23/REPORT.md). */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
extern int *func_800C2B10();
extern void func_800C4E50();
extern void func_8006DF50();
extern int D_800B0E64;
void func_8018F350(int a0, int a1, void *p)
{
    volatile int *q;

    H(p, 0x13C) = 0;
    H(p, 0x138) = 0;
    H(p, 0x13A) = 128;
    H(p, 0x114) = 16;
    H(p, 0x11A) = 0;
    H(p, 0x11C) = 128;
    B(p, 0x110) = *func_800C2B10(1);
    B(p, 0x111) = *func_800C2B10(2);
    B(p, 0x112) = *func_800C2B10(3);
    B(p, 0x10C) = *func_800C2B10(4);
    B(p, 0x10D) = *func_800C2B10(5);
    B(p, 0x10E) = *func_800C2B10(6);
    H(p, 0x116) = 1200;
    H(p, 0x118) = 600;
    P(p, 0x108) = (char *)p + 8;
    func_800C4E50((char *)p + 0x108);
    H(p, 0x12C) = 16;
    H(p, 0x132) = 0;
    H(p, 0x134) = 128;
    B(p, 0x128) = *func_800C2B10(1) >> 1;
    B(p, 0x129) = *func_800C2B10(2) >> 1;
    B(p, 0x12A) = *func_800C2B10(3) >> 1;
    B(p, 0x124) = *func_800C2B10(4) >> 1;
    B(p, 0x125) = *func_800C2B10(5) >> 1;
    B(p, 0x126) = *func_800C2B10(6) >> 1;
    H(p, 0x12E) = 1200;
    H(p, 0x130) = 300;
    P(p, 0x120) = (char *)p + 8;
    func_800C4E50((char *)p + 0x120);
    q = &D_800B0E64;
    if (*q != 0) {
        func_8006DF50(*q, 1406, 0, 128, 127);
    }
}
