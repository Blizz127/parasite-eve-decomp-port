/* room_m0393i (PE.IMG room m0393i chunk 2, VRAM 0x8018EFE8)
 * func_80190BAC — blob offset 0x1bc4, 0x18 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0137i func_8018FA14; C re-targeted by symbol address
 * (docs/evidence/room_m0393i-ports-2026-09-23/REPORT.md). */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
void func_80190BAC(int a0, int a1, void *p)
{
    H(p, 0x10) = 600;
    H(p, 0x12) = 128;
    B(p, 0x15) = 0;
}
