/* room_m0146i (PE.IMG room m0146i chunk 2, VRAM 0x8018EFE8)
 * func_80190700 — blob offset 0x1718, 0x1f0 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0137i func_8018F230; C re-targeted by symbol address
 * (docs/evidence/room_m0146i-ports-2026-09-23/REPORT.md). */

typedef struct { unsigned char r, g, b, pad3; unsigned char b4, b5, b6, pad7; short h8, hA; int padC; } P;
extern P D_80192688, D_80192698, D_801926C8, D_801926D8, D_801926E8;
extern void func_800C2B40();
extern int func_8006DC18();
extern int *func_800C2B28();

void func_80190700(unsigned char *a0, void *a1, unsigned char *a2)
{
    func_800C2B40(a2);
    *(int *)(a2 + 0x10) = func_8006DC18(0x28);
    *(short *)(a2 + 6) = 0;
    *(short *)(a2 + 8) = 0;
    *(void **)a2 = *(void **)(a0 + 8);
    *(short *)(a2 + 0xA) = *func_800C2B28(1);
    *(short *)(a2 + 0xC) = *func_800C2B28(2);
    *(short *)(a2 + 4) = *func_800C2B28(3);
    D_801926C8.h8 = 0xa;
    D_801926C8.b4 = 0x24;
    D_801926C8.b5 = 1;
    D_801926C8.hA = 0x80;
    D_801926C8.r = 0x80;
    D_801926C8.g = 0x80;
    D_801926C8.b = 0x80;
    D_801926C8.b6 = 0;
    D_80192698.h8 = 0x64;
    D_80192698.b4 = 0x0;
    D_80192698.b5 = 0;
    D_80192698.hA = 0x80;
    D_80192698.r = 0x80;
    D_80192698.g = 0x80;
    D_80192698.b = 0x80;
    D_80192698.b6 = 0;
    D_801926D8.b4 = 0x20;
    D_801926D8.b5 = 1;
    D_801926D8.h8 = 0xa0;
    D_801926D8.hA = 0x80;
    D_801926D8.r = 0x80;
    D_801926D8.g = 0x80;
    D_801926D8.b = 0x80;
    D_801926D8.b6 = 0;
    D_801926E8.b4 = 0x24;
    D_801926E8.b5 = 1;
    D_801926E8.h8 = 0x6e;
    D_801926E8.hA = 0x80;
    D_801926E8.r = 0x80;
    D_801926E8.g = 0x80;
    D_801926E8.b = 0x80;
    D_801926E8.b6 = 0;
    D_80192688.h8 = 0x6e;
    D_80192688.b4 = 0x24;
    D_80192688.b5 = 1;
    D_80192688.hA = 0x30;
    D_80192688.r = 0xa0;
    D_80192688.g = 0xa0;
    D_80192688.b = 0xa0;
    D_80192688.b6 = 0;
}
