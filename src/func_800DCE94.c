extern int D_800E27EC;
extern unsigned char *D_800F33E0;
extern void *D_8009D254;
extern char D_800E1F18;
extern unsigned short D_800E11E6;
extern unsigned short D_800E2850[];
extern unsigned short D_800E222C;
extern unsigned short D_800E222E;
extern unsigned short D_800E2230;
extern short D_800F336C;
extern short D_800F336E;
extern short D_800F3370;
extern short D_800F3374;
extern int func_80071A54();
extern int func_80077CF4();
extern int func_800CE560();
extern short *func_800CE610();
extern void func_800CE870();
extern void func_800CEDA8();
extern void func_800CF3AC();
extern void func_800D004C();
extern void func_800D0728();
extern void func_800DCCCC();

int func_800DCE94(int a0, unsigned char *a1)
{
    short buf[4];
    int blk[2];
    short *p;
    int t;
    void *q;
    int s1v;
    int v;
    int ix;
    int av;

    switch (a0) {
    case 0:
        t = func_80071A54();
        q = D_8009D254;
        *(int *)(a1 + 8) = t;
        func_800CE870(q, 0, a1);
        return func_800CE560(*(void **)(D_800F33E0 + 8), 0x10, 0x10, func_800DCCCC);
    case 1:
        if (D_800E27EC < 0x11) {
            p = func_800CE610(*(void **)(D_800F33E0 + 8));
            if (p != 0) {
                p[6] = (func_80071A54() & 0x1FF) - 0x100;
                p[1] = *(int *)(a1 + 8);
                p[2] = func_80071A54();
                p[7] = (func_80071A54() & 0x1F) - 0x10;
                *(int *)(a1 + 8) += 0x8AA + (func_80071A54() & 0x1F);
            }
        }
        if (D_800E27EC < 0x49) {
            break;
        }
        return 1;
    case 2:
        if (D_800E27EC < 0x41) {
            s1v = (D_800E27EC << 6) + 0x800;
            D_800F3374 = 0x3C;
            buf[0] = *(unsigned short *)(a1 + 0);
            buf[1] = *(unsigned short *)(a1 + 2);
            buf[2] = *(unsigned short *)(a1 + 4);
            func_800CF3AC(&D_800E1F18, blk, D_800E27EC);
            v = 1;
            if (D_800E27EC & 1) {
                v = 3;
            }
            func_800D0728(buf, 0xC8, 0x12C, 0x10, 0, s1v, s1v, blk, 0, 0x80, v);
            s1v = func_80077CF4(D_800E27EC << 4) / 2 + 0x800;
            func_800D004C(buf, 0x2BC, 0x2BC, 0x10, 0, s1v, s1v, blk, 0, 0x80, 1);
            func_800D0728(buf, 0x258, 0x320, 0x18, 0, s1v, s1v, 0, blk, 0x80, 3);
        }
        D_800E222C = *(unsigned short *)(a1 + 0);
        D_800E222E = *(unsigned short *)(a1 + 2);
        D_800E2230 = *(unsigned short *)(a1 + 4);
        ix = D_800E11E6;
        av = *(unsigned short *)((char *)D_800E2850 + ix * 2);
        D_800F336C = 1;
        D_800F3370 = av;
        func_800CEDA8(1);
        D_800F336E = 0;
        D_800F3374 = 8;
        break;
    default:
        return 0;
    }
    return 0;
}
