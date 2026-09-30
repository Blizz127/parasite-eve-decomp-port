/* room_m0348i (PE.IMG room m0348i chunk 2, VRAM 0x8018EFE8)
 * func_8018FA2C — blob offset 0xa44, 0x24 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0174i func_80190B44; C re-targeted by symbol address
 * (docs/evidence/room_m0348i-ports-2026-09-23/REPORT.md). */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
void func_8018FA2C(int a0, int a1, void *p)
{
    H(p, 0x10) = 2772;
    H(p, 0x12) = 255;
    H(p, 0x8) = 0;
    H(p, 0xA) = 0;
    H(p, 0xC) = 0;
    H(p, 0x14) = 0;
}
