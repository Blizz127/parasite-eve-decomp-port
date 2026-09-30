/* room_m0418i — func_801923D8, blob offset 0x33F0, 0x16C bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl7 2026-09-27).
 * Three 0x18-byte sprite records (SPR D_801995F8[3]) seeded + registered via func_800C4E50; first-block store order via climb. */

typedef struct { unsigned char *p; unsigned char a[4]; unsigned char b[4]; short h[5]; } SPR;
extern SPR D_801995F8[3];
extern unsigned char D_80198C90[];
extern void func_800C4E50();

void func_801923D8(int a0, int a1, char *a2)
{
    *(short *)(a2 + 8) = 0;
    *(short *)(a2 + 0xA) = 0;
    *(short *)(a2 + 0xC) = 0;
    *(short *)(a2 + 0x12) = 0;
    D_801995F8[0].h[0] = 0x20;
    D_801995F8[0].h[3] = 0x1F4;
    D_801995F8[0].h[4] = 0x80;
    D_801995F8[0].b[0] = 0xF0;
    D_801995F8[0].b[1] = 0xC8;
    D_801995F8[0].b[2] = 0x78;
    D_801995F8[0].a[0] = 0;
    D_801995F8[0].a[1] = 0;
    D_801995F8[0].a[2] = 0;
    D_801995F8[0].h[1] = 0x960;
    D_801995F8[0].h[2] = 0;
    D_801995F8[0].p = D_80198C90;
    func_800C4E50(&D_801995F8[0]);
    D_801995F8[1].h[0] = 0x20;
    D_801995F8[1].h[3] = 0x5DC;
    D_801995F8[1].h[4] = 0x80;
    D_801995F8[1].b[0] = 0;
    D_801995F8[1].b[1] = 0;
    D_801995F8[1].b[2] = 0;
    D_801995F8[1].a[0] = 0xFF;
    D_801995F8[1].a[1] = 0xFF;
    D_801995F8[1].a[2] = 0xFF;
    D_801995F8[1].h[1] = 0x960;
    D_801995F8[1].h[2] = 0;
    D_801995F8[1].p = D_80198C90 + 0x200;
    func_800C4E50(&D_801995F8[1]);
    D_801995F8[2].h[0] = 0x20;
    D_801995F8[2].h[3] = 0x5DC;
    D_801995F8[2].h[4] = 0x80;
    D_801995F8[2].b[0] = 0;
    D_801995F8[2].b[1] = 0;
    D_801995F8[2].b[2] = 0;
    D_801995F8[2].a[0] = 0xFF;
    D_801995F8[2].a[1] = 0xFF;
    D_801995F8[2].a[2] = 0xFF;
    D_801995F8[2].h[1] = 0x960;
    D_801995F8[2].h[2] = 0;
    D_801995F8[2].p = D_80198C90 + 0x400;
    func_800C4E50(&D_801995F8[2]);
}
