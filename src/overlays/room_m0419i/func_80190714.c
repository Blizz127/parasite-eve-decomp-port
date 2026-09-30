/* room_m0419i (PE.IMG room m0419i chunk 2, VRAM 0x8018EFE8)
 * func_80190714 — blob offset 0x172c, 0x1b4 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0075i func_801906D0; C re-targeted by symbol address
 * (docs/evidence/room_m0419i-ports-2026-09-23/REPORT.md). */

extern int func_80071A54();

#define SH(o, x) (*(short *)((char *)(o) + (x)))

void func_80190714(int a0, unsigned char *a1, unsigned char *o)
{
    unsigned int i;

    for (i = 0; i < SH(o, 0x182); i++) {
        SH(o + i * 4, 0x160) = func_80071A54() % 8;
        SH(o + i * 4, 0x162) = func_80071A54() % 8;
    }
    if (SH(a1, 2) >= 0x1F) {
        if (SH(o, 0x180) >= 5) {
            SH(o, 0x180) -= 4;
        } else {
            SH(o, 0x180) = 0;
        }
        for (i = 0; i < SH(o, 0x182); i++) {
            SH(o + i * 16, 0x40) += SH(o + i * 16, 0xC0);
            SH(o + i * 16, 0x42) += SH(o + i * 16, 0xC2);
            SH(o + i * 16, 0x44) += SH(o + i * 16, 0xC4);
            SH(o + i * 16, 0x48) += SH(o + i * 16, 0xC8);
            SH(o + i * 16, 0x4A) += SH(o + i * 16, 0xCA);
            SH(o + i * 16, 0x4C) += SH(o + i * 16, 0xCC);
        }
    }
    if (SH(a1, 2) > 0 && SH(a1, 2) < 0x20) {
        SH(o, 0x180) += 4;
    }
    if (SH(a1, 2) == 0x41) {
        a1[1] = 2;
    }
}
