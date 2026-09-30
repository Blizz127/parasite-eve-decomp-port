typedef struct {
    unsigned char b0;
    unsigned char b1;
    unsigned char b2;
    unsigned char b3;
} B4;

extern B4 D_800C22D8;
extern int D_800E27EC;
extern void *D_8009D254;
extern unsigned char *D_800E2368;
extern unsigned short D_800E11F6;
extern unsigned short D_800E2850[];
extern short D_800F3368;
extern short D_800F336A;
extern unsigned short D_800F336C;
extern short D_800F336E;
extern short D_800F3370;
extern short D_800F3372;
extern short D_800F3374;
extern short D_800F3376;
extern short D_800F3378;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern int func_80077AA4(int a0, int a1);
extern int func_80077CF4(int a0);
extern void func_800CE870();
extern void func_800CEDA8();
extern void func_800CEE20();
extern void func_800D0728();

int func_800D7764(int a0, short *a1)
{
    short vec[4];
    short pos[4];
    B4 blk;
    int s0v;
    int s2v;
    int w;
    int idx;
    int t;
    int ix;
    int av;
    int v;
    void *q;

    blk = D_800C22D8;
    switch (a0) {
    case 0:
        q = D_8009D254;
        a1[4] = 0;
        a1[5] = 0;
        func_800CE870(q, 0, a1);
        return 0;
    case 1:
        if (D_800E27EC < 0x3C) {
            break;
        }
        return 1;
    case 2:
        ix = D_800E11F6;
        D_800F3368 = 0x20;
        D_800F336A = 2;
        D_800F3376 = 0x20;
        D_800F3378 = 0x20;
        av = *(unsigned short *)((char *)D_800E2850 + ix * 2);
        D_800F336C = 1;
        D_800F3370 = av;
        func_800CEDA8(1, ix * 2);
        D_800F336E = 1;
        D_800F3372 = 0;
        D_800F3374 = 0x64;
        s2v = func_80077CF4((D_800E27EC << 11) / 60) / 32;
        idx = D_800F336C;
        s0v = 0x2800;
        w = D_800E1204[idx] + ((idx == 4 && D_800F3428 != 0) ? 6 : 2);
        t = func_80077AA4(0, w);
        func_800CEE20(a1, 0, s0v, s0v, 4, t & 0xFFFF, 3, s2v, 0);
        if (*(short *)(D_800E2368 + 0x1E) == 0xB) {
            break;
        }
        pos[0] = a1[0];
        pos[1] = a1[1];
        pos[2] = a1[2];
        vec[0] = 0;
        vec[1] = 0;
        vec[2] = D_800E27EC << 3;
        vec[3] = 0;
        s0v = (D_800E27EC & 1) * 60 + 0x1000;
        v = 3;
        if (*(short *)(D_800E2368 + 0x1E) == 0xA) {
            v = 1;
        }
        func_800D0728(pos, 0x190, 0x1F4, 0x14, vec, s0v, s0v, 0, &blk, s2v, v);
        break;
    default:
        return 0;
    }
    return 0;
}
