extern char D_800E0FCC;
extern char D_800E0FB4;
extern char D_800E0FD8;

extern int func_800C251C(int a0, char *a1);
extern int func_800C2758(int a0, char *a1, char *a2);
extern void func_800CE1DC(int a0);

int func_800CE16C(int a0)
{
    int r;

    r = func_800C251C(a0, &D_800E0FCC) | func_800C2758(a0, &D_800E0FB4, &D_800E0FD8);
    if (r == -1) {
        func_800CE1DC(a0);
    }
    return 0;
}
