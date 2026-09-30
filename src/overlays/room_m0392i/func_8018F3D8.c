/* room_m0392i (PE.IMG room m0392i chunk 2, VRAM 0x8018EFE8)
 * func_8018F3D8 — blob offset 0x3f0, 0x1f0 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0137i func_8018F230; C re-targeted by symbol address
 * (docs/evidence/room_m0392i-ports-2026-09-23/REPORT.md). */

typedef struct { unsigned char r, g, b, pad3; unsigned char b4, b5, b6, pad7; short h8, hA; int padC; } P;
extern P D_801946C0, D_801946D0, D_801946E0, D_801946F0, D_80194700;
extern void func_800C2B40();
extern int func_8006DC18();
extern int *func_800C2B28();

void func_8018F3D8(unsigned char *a0, void *a1, unsigned char *a2)
{
    func_800C2B40(a2);
    *(int *)(a2 + 0x10) = func_8006DC18(0x28);
    *(short *)(a2 + 6) = 0;
    *(short *)(a2 + 8) = 0;
    *(void **)a2 = *(void **)(a0 + 8);
    *(short *)(a2 + 0xA) = *func_800C2B28(1);
    *(short *)(a2 + 0xC) = *func_800C2B28(2);
    *(short *)(a2 + 4) = *func_800C2B28(3);
    D_801946E0.h8 = 0xa;
    D_801946E0.b4 = 0x24;
    D_801946E0.b5 = 1;
    D_801946E0.hA = 0x80;
    D_801946E0.r = 0x80;
    D_801946E0.g = 0x80;
    D_801946E0.b = 0x80;
    D_801946E0.b6 = 0;
    D_801946D0.h8 = 0x64;
    D_801946D0.b4 = 0x0;
    D_801946D0.b5 = 0;
    D_801946D0.hA = 0x80;
    D_801946D0.r = 0x80;
    D_801946D0.g = 0x80;
    D_801946D0.b = 0x80;
    D_801946D0.b6 = 0;
    D_801946F0.b4 = 0x20;
    D_801946F0.b5 = 1;
    D_801946F0.h8 = 0xa0;
    D_801946F0.hA = 0x80;
    D_801946F0.r = 0x80;
    D_801946F0.g = 0x80;
    D_801946F0.b = 0x80;
    D_801946F0.b6 = 0;
    D_80194700.b4 = 0x24;
    D_80194700.b5 = 1;
    D_80194700.h8 = 0x6e;
    D_80194700.hA = 0x80;
    D_80194700.r = 0x80;
    D_80194700.g = 0x80;
    D_80194700.b = 0x80;
    D_80194700.b6 = 0;
    D_801946C0.h8 = 0x6e;
    D_801946C0.b4 = 0x24;
    D_801946C0.b5 = 1;
    D_801946C0.hA = 0x30;
    D_801946C0.r = 0xa0;
    D_801946C0.g = 0xa0;
    D_801946C0.b = 0xa0;
    D_801946C0.b6 = 0;
}
