extern int D_800E27EC;
extern char D_800E1A14;
extern unsigned short D_800E21F8;
extern unsigned short D_800E21FA;
extern unsigned short D_800E21FC;
extern unsigned short D_800F336C;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern int func_80077AA4(int a0, int a1);
extern int func_80077CF4(int a0);
extern void func_800CF3AC(char *a0, int *a1, int a2);
extern void func_800CEE20(short *a0, short *a1, int a2, int a3, int a4, int a5, int a6, int a7, int *a8);

int func_800D8978(int a0, short *a1)
{
    short pos[4];
    short vec[4];
    int blk[2];
    int s1v;
    int w;
    int idx;
    int r;

    switch (a0) {
    case 1:
        {
        register int t asm("$3");
        t = D_800E21FA;
        t += (-D_800E27EC * 900) / 24;
        *(short *)((char *)a1 + 8) = t;
        t = *(unsigned short *)((char *)a1 + 4);
        *(short *)((char *)a1 + 6) = D_800E21F8;
        *(short *)((char *)a1 + 0xA) = D_800E21FC;
        *(short *)((char *)a1 + 2) += 0x80;
        *(short *)((char *)a1 + 4) = t + 1;
        }
        if (D_800E27EC < 0x18) {
            break;
        }
        return 1;
    case 2:
        if (a1[0] == 0) {
            s1v = 0x2000 - func_80077CF4(a1[2] << 7);
            if (a1[2] >= 8) {
                a1[2] = 0;
                a1[0] = 1;
            }
        } else {
            s1v = 0x1000;
        }
        pos[0] = a1[3];
        pos[1] = a1[4];
        pos[2] = a1[5];
        vec[0] = 0x400;
        vec[1] = 0;
        vec[2] = D_800E27EC << 7;
        vec[3] = 1;
        func_800CF3AC(&D_800E1A14, blk, D_800E27EC);
        s1v <<= 1;
        idx = D_800F336C;
        w = D_800E1204[idx] + ((idx == 4 && D_800F3428 != 0) ? 7 : 3);
        r = func_80077AA4(0, w);
        func_800CEE20(pos, vec, s1v, s1v, 0x44, r & 0xFFFF, 1, 0x80, blk);
        break;
    default:
        return 0;
    }
    return 0;
}
