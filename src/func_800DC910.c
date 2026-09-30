extern int D_800E27EC;
extern char D_800E1EE8;
extern unsigned short D_800F336C;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern int func_80071A54(void);
extern int func_80077AA4(int a0, int a1);
extern void func_800CF3AC(char *a0, int *a1, int a2);
extern void func_800CEE20(short *a0, short *a1, int a2, int a3, int a4, int a5, int a6, int a7, int *a8);

int func_800DC910(int a0, short *a1)
{
    int blk[2];
    short vec[4];
    int s1v;
    int w;
    int idx;
    int t;
    int r;
    unsigned short u;

    switch (a0) {
    case 1:
        r = func_80071A54();
        u = a1[0] - 3;
        a1[0] = u + (r & 7);
        r = func_80071A54();
        u = a1[2] - 3;
        a1[2] = u + (r & 7);
        r = func_80071A54();
        u = a1[1] - 2;
        a1[1] = u - (r & 3);
        if (D_800E27EC < 0x20) {
            break;
        }
        return 1;
    case 2:
        vec[0] = 0;
        vec[1] = 0;
        vec[3] = 0;
        vec[2] = D_800E27EC * 24;
        func_800CF3AC(&D_800E1EE8, blk, D_800E27EC);
        s1v = (D_800E27EC << 7) + 0x2000;
        idx = D_800F336C;
        w = D_800E1204[idx];
        if (idx == 4 && D_800F3428 != 0) {
            w += 4;
        }
        t = func_80077AA4(0x30, w);
        func_800CEE20(a1, vec, s1v, s1v, 0x82, t & 0xFFFF, 1, 0x80, blk);
        break;
    default:
        return 0;
    }
    return 0;
}
