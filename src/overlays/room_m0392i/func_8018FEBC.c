/* room_m0392i (PE.IMG room m0392i chunk 2, VRAM 0x8018EFE8)
 * func_8018FEBC — blob offset 0xed4, 0x104 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0137i func_8018FD14; C re-targeted by symbol address
 * (docs/evidence/room_m0392i-ports-2026-09-23/REPORT.md). */

extern unsigned char *func_800C2B50();
extern int func_800C6B90();

void func_8018FEBC(void *a0, unsigned char *a1, unsigned char *a2)
{
    unsigned char *e;
    short t;
    short v;
    int w;

    e = func_800C2B50();
    if (*(short *)(e + 0xC) - 0x3C < *(short *)(a1 + 2)) {
        t = *(short *)(a2 + 0xA);
        if (t >= 5) {
            *(short *)(a2 + 0xA) = t - 4;
        }
        if (*(short *)(e + 0xC) - 0x1E < *(short *)(a1 + 2)) {
            v = *(short *)(a2 + 8);
            if (v > *(short *)(e + 0xA) * 6) {
                w = *(short *)(e + 0xA) * 6;
                *(short *)(a2 + 8) = v - w;
            }
        }
    } else {
        *(short *)(a2 + 8) += *(short *)(e + 0xA);
    }
    if (func_800C6B90(a2, *(short *)(a2 + 8) >> 3)) {
        *(short *)(e + 8) = 1;
    }
    if (*(short *)(a1 + 2) == *(short *)(e + 0xC)) {
        a1[1] = 2;
    }
}
