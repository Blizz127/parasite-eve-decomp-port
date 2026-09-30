extern char D_800E0D24;
extern char D_800E0CEC;
extern char D_800E0D40;

extern int func_800C251C(int a0, char *a1);
extern int func_800C2758(int a0, char *a1, char *a2);
extern void func_800CBFA4(int a0);

int func_800CBF34(int a0)
{
    int r;

    r = func_800C251C(a0, &D_800E0D24) | func_800C2758(a0, &D_800E0CEC, &D_800E0D40);
    if (r == -1) {
        func_800CBFA4(a0);
    }
    return 0;
}
