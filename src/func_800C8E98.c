extern char D_800E09B4;
extern char D_800E098C;
extern char D_800E09C8;

extern int func_800C251C(int a0, char *a1);
extern int func_800C2758(int a0, char *a1, char *a2);
extern void func_800C8F08(int a0);

int func_800C8E98(int a0)
{
    int r;

    r = func_800C251C(a0, &D_800E09B4) | func_800C2758(a0, &D_800E098C, &D_800E09C8);
    if (r == -1) {
        func_800C8F08(a0);
    }
    return 0;
}
