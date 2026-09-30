/* room_m0418i — func_8019155C, blob offset 0x2574, 0x1E4 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl7 2026-09-27).
 * Object init + func_8006E498 handle, three sprite descriptor seeds, owner position copy, optional func_8006DF50 on D_800B0E64; volatile pointer local keeps the two D_800B0E64 reads. */

typedef struct { int w[8]; } B32;
extern unsigned char D_80199680, D_80199681, D_80199682, D_80199684, D_80199685, D_80199686;
extern short D_80199688, D_8019968A;
extern unsigned char D_801996A0, D_801996A1, D_801996A2, D_801996A4, D_801996A5, D_801996A6;
extern short D_801996A8, D_801996AA;
extern unsigned char D_801996B0, D_801996B1, D_801996B2, D_801996B4, D_801996B5, D_801996B6;
extern short D_801996B8, D_801996BA;
extern int D_800B0E64;
extern int D_80199528;
extern int D_8019956C;
extern int D_8019957C;
extern char *D_8009D254;
extern void func_800C2B40();
extern int func_8006DC18();
extern int func_8006E498();
extern void func_8006DF50();

void func_8019155C(char *a0, int a1, char *a2)
{
    char *p;
    volatile int *e;
    int r;
    int x, y;
    B32 *b;

    func_800C2B40(a2);
    e = &D_800B0E64;
    r = func_8006E498(*e, 0x10D8704);
    p = *(char **)(a0 + 8);
    *(char **)a2 = p;
    b = *(B32 **)(p + 0x238);
    D_80199528 = r;
    *(B32 *)(a2 + 4) = *b;
    *(int *)(a2 + 0x24) = func_8006DC18(0x2E);
    D_80199684 = 0x80;
    D_80199685 = 0x8;
    D_80199688 = -400;
    D_8019968A = 0x80;
    D_80199680 = 0x80;
    D_80199681 = 0x80;
    D_80199682 = 0x80;
    D_80199686 = 0;
    D_801996A4 = 0x44;
    D_801996A5 = 0x2;
    D_801996A8 = -400;
    D_801996AA = 0x80;
    D_801996A0 = 0x80;
    D_801996A1 = 0x80;
    D_801996A2 = 0x80;
    D_801996A6 = 0;
    D_801996B4 = 0x60;
    D_801996B5 = 0x40;
    D_801996B8 = -500;
    D_801996BA = 0x80;
    D_801996B0 = 0x80;
    D_801996B1 = 0x80;
    D_801996B2 = 0x80;
    D_801996B6 = 0;
    x = *(short *)(D_8009D254 + 0x2A);
    y = *(short *)(D_8009D254 + 0x32);
    D_8019956C = x;
    D_8019957C = y;
    if (*e != 0) {
        func_8006DF50(*e, 0x603, 0, 0x80, 0x7F);
    }
}
