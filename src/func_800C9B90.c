extern char D_800E0AA4;
extern char D_800E0A84;
extern char D_800E0AB4;

extern int func_800C251C(int a0, char *a1);
extern int func_800C2758(int a0, char *a1, char *a2);
extern void func_800C9C00(int a0);

int func_800C9B90(int a0)
{
    int r;

    r = func_800C251C(a0, &D_800E0AA4) | func_800C2758(a0, &D_800E0A84, &D_800E0AB4);
    if (r == -1) {
        func_800C9C00(a0);
    }
    return 0;
}
