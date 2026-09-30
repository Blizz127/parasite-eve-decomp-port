/* room_m0401i (PE.IMG room m0401i chunk 2, VRAM 0x8018EFE8)
 * func_801903D0 — blob offset 0x13e8, 0x1f0 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0137i func_8018F230; C re-targeted by symbol address
 * (docs/evidence/room_m0401i-ports-2026-09-23/REPORT.md). */

typedef struct { unsigned char r, g, b, pad3; unsigned char b4, b5, b6, pad7; short h8, hA; int padC; } P;
extern P D_801955A0, D_801955B0, D_801955E0, D_801955F0, D_80195600;
extern void func_800C2B40();
extern int func_8006DC18();
extern int *func_800C2B28();

void func_801903D0(unsigned char *a0, void *a1, unsigned char *a2)
{
    func_800C2B40(a2);
    *(int *)(a2 + 0x10) = func_8006DC18(0x28);
    *(short *)(a2 + 6) = 0;
    *(short *)(a2 + 8) = 0;
    *(void **)a2 = *(void **)(a0 + 8);
    *(short *)(a2 + 0xA) = *func_800C2B28(1);
    *(short *)(a2 + 0xC) = *func_800C2B28(2);
    *(short *)(a2 + 4) = *func_800C2B28(3);
    D_801955E0.h8 = 0xa;
    D_801955E0.b4 = 0x24;
    D_801955E0.b5 = 1;
    D_801955E0.hA = 0x80;
    D_801955E0.r = 0x80;
    D_801955E0.g = 0x80;
    D_801955E0.b = 0x80;
    D_801955E0.b6 = 0;
    D_801955B0.h8 = 0x64;
    D_801955B0.b4 = 0x0;
    D_801955B0.b5 = 0;
    D_801955B0.hA = 0x80;
    D_801955B0.r = 0x80;
    D_801955B0.g = 0x80;
    D_801955B0.b = 0x80;
    D_801955B0.b6 = 0;
    D_801955F0.b4 = 0x20;
    D_801955F0.b5 = 1;
    D_801955F0.h8 = 0xa0;
    D_801955F0.hA = 0x80;
    D_801955F0.r = 0x80;
    D_801955F0.g = 0x80;
    D_801955F0.b = 0x80;
    D_801955F0.b6 = 0;
    D_80195600.b4 = 0x24;
    D_80195600.b5 = 1;
    D_80195600.h8 = 0x6e;
    D_80195600.hA = 0x80;
    D_80195600.r = 0x80;
    D_80195600.g = 0x80;
    D_80195600.b = 0x80;
    D_80195600.b6 = 0;
    D_801955A0.h8 = 0x6e;
    D_801955A0.b4 = 0x24;
    D_801955A0.b5 = 1;
    D_801955A0.hA = 0x30;
    D_801955A0.r = 0xa0;
    D_801955A0.g = 0xa0;
    D_801955A0.b = 0xa0;
    D_801955A0.b6 = 0;
}
