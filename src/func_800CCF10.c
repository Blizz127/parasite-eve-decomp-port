extern char D_800E0E78;
extern char D_800E0E48;
extern char D_800E0E90;

extern int func_800C251C(int a0, char *a1);
extern int func_800C2758(int a0, char *a1, char *a2);
extern void func_800CCF80(int a0);

int func_800CCF10(int a0)
{
    int r;

    r = func_800C251C(a0, &D_800E0E78) | func_800C2758(a0, &D_800E0E48, &D_800E0E90);
    if (r == -1) {
        func_800CCF80(a0);
    }
    return 0;
}
