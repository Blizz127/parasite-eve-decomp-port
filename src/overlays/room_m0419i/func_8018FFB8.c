/* room_m0419i (PE.IMG room m0419i chunk 2, VRAM 0x8018EFE8)
 * func_8018FFB8 — blob offset 0xfd0, 0x49c bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0075i func_8018FF74; C re-targeted by symbol address
 * (docs/evidence/room_m0419i-ports-2026-09-23/REPORT.md). */

extern int func_80071A54();
extern int *func_800C2B10();

#define SH(o, x) (*(short *)((char *)(o) + (x)))

void func_8018FFB8(int a0, int a1, unsigned char *o)
{
    unsigned int i;

    if (*func_800C2B10(1) == 0) {
        SH(o, 0x182) = 6;
    } else {
        SH(o, 0x182) = 3;
    }
    SH(o, 0x180) = 0;
    for (i = 0; i < SH(o, 0x182); i++) {
        SH(o + i * 4, 0x140) = 0x400;
        SH(o + i * 4, 0x142) = 0x400;
        SH(o + i * 16, 0x40) = func_80071A54() % 140 - 70;
        SH(o + i * 16, 0x42) = func_80071A54() % 140 - 70;
        SH(o + i * 16, 0x44) = func_80071A54() % 140 - 70;
        SH(o + i * 16, 0x48) = func_80071A54() % 140 - 70;
        SH(o + i * 16, 0x4A) = func_80071A54() % 140 - 70;
        SH(o + i * 16, 0x4C) = func_80071A54() % 140 - 70;
        SH(o + i * 4, 0x160) = func_80071A54() % 8;
        SH(o + i * 4, 0x162) = func_80071A54() % 8;
        if (*func_800C2B10(1) == 0) {
            SH(o + i * 16, 0xC0) = func_80071A54() % 40 - 20;
            SH(o + i * 16, 0xC2) = 0;
            SH(o + i * 16, 0xC4) = func_80071A54() % 40 - 20;
            SH(o + i * 16, 0xC8) = func_80071A54() % 40 - 20;
            SH(o + i * 16, 0xCA) = 0;
            SH(o + i * 16, 0xCC) = func_80071A54() % 40 - 20;
        } else {
            SH(o + i * 16, 0xC0) = func_80071A54() % 26 - 13;
            SH(o + i * 16, 0xC2) = -func_80071A54() % 10;
            SH(o + i * 16, 0xC4) = func_80071A54() % 26 - 13;
            SH(o + i * 16, 0xC8) = func_80071A54() % 26 - 13;
            SH(o + i * 16, 0xCA) = -func_80071A54() % 10;
            SH(o + i * 16, 0xCC) = func_80071A54() % 26 - 13;
        }
    }
}
