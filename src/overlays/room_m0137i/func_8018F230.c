/* room_m0137i — func_8018F230, blob offset 0x248, 0x1F0 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl 2026-09-27).
 * effect init: record from func_800C2B28 params + five colour/size descriptors D_80190F50..F90; lever: per-descriptor field order */

typedef struct { unsigned char r, g, b, pad3; unsigned char b4, b5, b6, pad7; short h8, hA; int padC; } P;
extern P D_80190F50, D_80190F60, D_80190F70, D_80190F80, D_80190F90;
extern void func_800C2B40();
extern int func_8006DC18();
extern int *func_800C2B28();

void func_8018F230(unsigned char *a0, void *a1, unsigned char *a2)
{
    func_800C2B40(a2);
    *(int *)(a2 + 0x10) = func_8006DC18(0x28);
    *(short *)(a2 + 6) = 0;
    *(short *)(a2 + 8) = 0;
    *(void **)a2 = *(void **)(a0 + 8);
    *(short *)(a2 + 0xA) = *func_800C2B28(1);
    *(short *)(a2 + 0xC) = *func_800C2B28(2);
    *(short *)(a2 + 4) = *func_800C2B28(3);
    D_80190F70.h8 = 0xa;
    D_80190F70.b4 = 0x24;
    D_80190F70.b5 = 1;
    D_80190F70.hA = 0x80;
    D_80190F70.r = 0x80;
    D_80190F70.g = 0x80;
    D_80190F70.b = 0x80;
    D_80190F70.b6 = 0;
    D_80190F60.h8 = 0x64;
    D_80190F60.b4 = 0x0;
    D_80190F60.b5 = 0;
    D_80190F60.hA = 0x80;
    D_80190F60.r = 0x80;
    D_80190F60.g = 0x80;
    D_80190F60.b = 0x80;
    D_80190F60.b6 = 0;
    D_80190F80.b4 = 0x20;
    D_80190F80.b5 = 1;
    D_80190F80.h8 = 0xa0;
    D_80190F80.hA = 0x80;
    D_80190F80.r = 0x80;
    D_80190F80.g = 0x80;
    D_80190F80.b = 0x80;
    D_80190F80.b6 = 0;
    D_80190F90.b4 = 0x24;
    D_80190F90.b5 = 1;
    D_80190F90.h8 = 0x6e;
    D_80190F90.hA = 0x80;
    D_80190F90.r = 0x80;
    D_80190F90.g = 0x80;
    D_80190F90.b = 0x80;
    D_80190F90.b6 = 0;
    D_80190F50.h8 = 0x6e;
    D_80190F50.b4 = 0x24;
    D_80190F50.b5 = 1;
    D_80190F50.hA = 0x30;
    D_80190F50.r = 0xa0;
    D_80190F50.g = 0xa0;
    D_80190F50.b = 0xa0;
    D_80190F50.b6 = 0;
}
