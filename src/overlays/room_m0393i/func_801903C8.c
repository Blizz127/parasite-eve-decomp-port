/* room_m0393i (PE.IMG room m0393i chunk 2, VRAM 0x8018EFE8)
 * func_801903C8 — blob offset 0x13e0, 0x1f0 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0137i func_8018F230; C re-targeted by symbol address
 * (docs/evidence/room_m0393i-ports-2026-09-23/REPORT.md). */

typedef struct { unsigned char r, g, b, pad3; unsigned char b4, b5, b6, pad7; short h8, hA; int padC; } P;
extern P D_80194838, D_80194848, D_80194878, D_80194888, D_80194898;
extern void func_800C2B40();
extern int func_8006DC18();
extern int *func_800C2B28();

void func_801903C8(unsigned char *a0, void *a1, unsigned char *a2)
{
    func_800C2B40(a2);
    *(int *)(a2 + 0x10) = func_8006DC18(0x28);
    *(short *)(a2 + 6) = 0;
    *(short *)(a2 + 8) = 0;
    *(void **)a2 = *(void **)(a0 + 8);
    *(short *)(a2 + 0xA) = *func_800C2B28(1);
    *(short *)(a2 + 0xC) = *func_800C2B28(2);
    *(short *)(a2 + 4) = *func_800C2B28(3);
    D_80194878.h8 = 0xa;
    D_80194878.b4 = 0x24;
    D_80194878.b5 = 1;
    D_80194878.hA = 0x80;
    D_80194878.r = 0x80;
    D_80194878.g = 0x80;
    D_80194878.b = 0x80;
    D_80194878.b6 = 0;
    D_80194848.h8 = 0x64;
    D_80194848.b4 = 0x0;
    D_80194848.b5 = 0;
    D_80194848.hA = 0x80;
    D_80194848.r = 0x80;
    D_80194848.g = 0x80;
    D_80194848.b = 0x80;
    D_80194848.b6 = 0;
    D_80194888.b4 = 0x20;
    D_80194888.b5 = 1;
    D_80194888.h8 = 0xa0;
    D_80194888.hA = 0x80;
    D_80194888.r = 0x80;
    D_80194888.g = 0x80;
    D_80194888.b = 0x80;
    D_80194888.b6 = 0;
    D_80194898.b4 = 0x24;
    D_80194898.b5 = 1;
    D_80194898.h8 = 0x6e;
    D_80194898.hA = 0x80;
    D_80194898.r = 0x80;
    D_80194898.g = 0x80;
    D_80194898.b = 0x80;
    D_80194898.b6 = 0;
    D_80194838.h8 = 0x6e;
    D_80194838.b4 = 0x24;
    D_80194838.b5 = 1;
    D_80194838.hA = 0x30;
    D_80194838.r = 0xa0;
    D_80194838.g = 0xa0;
    D_80194838.b = 0xa0;
    D_80194838.b6 = 0;
}
