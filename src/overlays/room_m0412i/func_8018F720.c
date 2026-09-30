/* room_m0412i (PE.IMG room m0412i chunk 2, VRAM 0x8018EFE8)
 * func_8018F720 — blob offset 0x738, 0x80 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0141i func_8018F57C; C re-targeted by symbol address
 * (docs/evidence/room_m0412i-ports-2026-09-23/REPORT.md). */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
extern void func_800C4E50();
void func_8018F720(int a0, int a1, void *p)
{
    H(p, 0x120) = 1500;
    H(p, 0x122) = 255;
    H(p, 0x114) = 16;
    B(p, 0x10C) = 64;
    B(p, 0x10D) = 32;
    B(p, 0x10E) = 16;
    B(p, 0x110) = 128;
    B(p, 0x111) = 128;
    B(p, 0x112) = 128;
    H(p, 0x116) = 240;
    H(p, 0x118) = 10;
    H(p, 0x124) = 0;
    H(p, 0x11A) = 0;
    P(p, 0x108) = (char *)p + 8;
    func_800C4E50((char *)p + 0x108);
}
