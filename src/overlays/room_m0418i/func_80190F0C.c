/* room_m0418i — func_80190F0C, blob offset 0x1F24, 0x140 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl7 2026-09-27).
 * Two 0x18-byte sprite records (SPR D_801995C8[2]) + object fields, each record registered via func_800C4E50; store order (callee-saved constant allocation) via two climbs. */

typedef struct { unsigned char *p; unsigned char a[4]; unsigned char b[4]; short h[5]; } SPR;
extern SPR D_801995C8[2];
extern unsigned char D_80198B90[];
extern void func_800C4E50();

void func_80190F0C(int a0, int a1, char *a2)
{
    a2[0xC] = 0;
    *(short *)(a2 + 0xA) = 0x80;
    *(short *)(a2 + 8) = 0x12C;
    a2[0xD] = 0x9C;
    D_801995C8[0].h[0] = 8;
    D_801995C8[0].h[3] = 0;
    D_801995C8[0].h[4] = 0x80;
    D_801995C8[0].b[0] = 0x78;
    D_801995C8[0].b[1] = 0x64;
    D_801995C8[0].b[2] = 0x1E;
    D_801995C8[0].a[0] = 0;
    D_801995C8[0].a[1] = 0;
    D_801995C8[0].a[2] = 0;
    D_801995C8[0].h[1] = 0x708;
    D_801995C8[0].h[2] = 0x4B0;
    D_801995C8[0].p = D_80198B90;
    func_800C4E50(&D_801995C8[0]);
    D_801995C8[1].h[0] = 8;
    D_801995C8[1].b[0] = 0x14;
    D_801995C8[1].b[1] = 0x14;
    D_801995C8[1].h[3] = 0;
    D_801995C8[1].h[4] = 0x80;
    D_801995C8[1].b[2] = 0;
    D_801995C8[1].a[0] = 0x78;
    D_801995C8[1].a[1] = 0x64;
    D_801995C8[1].a[2] = 0x1E;
    D_801995C8[1].h[1] = 0x4B0;
    D_801995C8[1].h[2] = 0x320;
    D_801995C8[1].p = D_80198B90 + 0x80;
    func_800C4E50(&D_801995C8[1]);
}
