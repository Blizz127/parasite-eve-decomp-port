extern char D_800E0F38;
extern char D_800E0F18;
extern char D_800E0F48;

extern int func_800C251C(int a0, char *a1);
extern int func_800C2758(int a0, char *a1, char *a2);
extern void func_800CD960(int a0);

int func_800CD8F0(int a0)
{
    int r;

    r = func_800C251C(a0, &D_800E0F38) | func_800C2758(a0, &D_800E0F18, &D_800E0F48);
    if (r == -1) {
        func_800CD960(a0);
    }
    return 0;
}
