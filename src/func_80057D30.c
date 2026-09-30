extern unsigned char *D_8009D048;
extern unsigned char D_800C0E48[];
extern signed char D_800C0E22;
extern unsigned char D_800C0EAC[];
extern void func_80059A40(int *);
extern void func_80054E4C(int);
extern void func_80054CF8(void);
extern void func_800512AC(int, int);

int func_80057D30(int idx)
{
    int tmp;
    int v;
    short *e;
    int w;

    if (D_8009D048 == D_800C0E48 && D_800C0E22 == idx) {
        func_80059A40(&tmp);
    }
    e = (short *)((idx << 1) + (int)D_8009D048);
    w = *e;
    *e = 0;
    v = w;
    if (v >= 0x100) {
        D_800C0EAC[(v - 0x100) << 5] = 0;
    }
    if (D_8009D048 == D_800C0E48 && D_800C0E22 == idx) {
        D_800C0E22 = -1;
        func_80054E4C(tmp);
        func_80054CF8();
        func_800512AC(3, 0);
    }
    return v;
}
