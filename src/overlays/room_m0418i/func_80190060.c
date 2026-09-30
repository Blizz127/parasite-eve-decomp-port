/* room_m0418i — func_80190060, blob offset 0x1078, 0x1E4 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl7 2026-09-27).
 * Object init (owner pointer + 32-byte block copy + func_8006DC18 handle) and four sprite descriptor seeds; sibling of func_8018F3C4. */

typedef struct { int w[8]; } B32;
extern unsigned char D_801994B8, D_801994B9, D_801994BA, D_801994BC, D_801994BD, D_801994BE;
extern short D_801994C0, D_801994C2;
extern unsigned char D_801995A8, D_801995A9, D_801995AA, D_801995AC, D_801995AD, D_801995AE;
extern short D_801995B0, D_801995B2;
extern unsigned char D_801995B8, D_801995B9, D_801995BA, D_801995BC, D_801995BD, D_801995BE;
extern short D_801995C0, D_801995C2;
extern unsigned char D_80199690, D_80199691, D_80199692, D_80199694, D_80199695, D_80199696;
extern short D_80199698, D_8019969A;
extern void func_800C2B40();
extern int func_8006DC18();

void func_80190060(char *a0, int a1, char *a2)
{
    char *p;

    func_800C2B40(a2);
    p = *(char **)(a0 + 8);
    *(char **)a2 = p;
    *(B32 *)(a2 + 4) = **(B32 **)(p + 0x238);
    *(int *)(a2 + 0x24) = func_8006DC18(0x2E);
    *(short *)(a2 + 0x28) = 0x1E;
    *(short *)(a2 + 0x2A) = 0;
    *(short *)(a2 + 0x2C) = 0;
    D_801994BC = 0x46;
    D_801994BD = 3;
    D_801994C0 = -400;
    D_801994C2 = 0x80;
    D_801994B8 = 0xf0;
    D_801994B9 = 0x40;
    D_801994BA = 0x40;
    D_801994BE = 0;
    D_801995AC = 0xa4;
    D_801995AD = 10;
    D_801995B0 = -30;
    D_801995B2 = 0x80;
    D_801995A8 = 0xa0;
    D_801995A9 = 0x80;
    D_801995AA = 0xf0;
    D_801995AE = 0;
    D_801995BC = 0x44;
    D_801995BD = 2;
    D_801995C0 = -30;
    D_801995C2 = 0x80;
    D_801995B8 = 0xa0;
    D_801995B9 = 0x80;
    D_801995BA = 0xf0;
    D_801995BE = 0;
    D_80199694 = 0x4c;
    D_80199695 = 5;
    D_80199698 = -30;
    D_8019969A = 0x80;
    D_80199690 = 0xa0;
    D_80199691 = 0x80;
    D_80199692 = 0xf0;
    D_80199696 = 0;
}
