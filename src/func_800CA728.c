extern char D_800E0BA0;
extern char D_800E0B68;
extern char D_800E0BBC;

extern int func_800C251C(int a0, char *a1);
extern int func_800C2758(int a0, char *a1, char *a2);
extern void func_800CA798(int a0);

int func_800CA728(int a0)
{
    int r;

    r = func_800C251C(a0, &D_800E0BA0) | func_800C2758(a0, &D_800E0B68, &D_800E0BBC);
    if (r == -1) {
        func_800CA798(a0);
    }
    return 0;
}
