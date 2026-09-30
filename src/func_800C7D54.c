extern char D_800E0840;
extern char D_800E0808;
extern char D_800E085C;

extern int func_800C251C(int a0, char *a1);
extern int func_800C2758(int a0, char *a1, char *a2);
extern void func_800C7DC4(int a0);

int func_800C7D54(int a0)
{
    int r;

    r = func_800C251C(a0, &D_800E0840) | func_800C2758(a0, &D_800E0808, &D_800E085C);
    if (r == -1) {
        func_800C7DC4(a0);
    }
    return 0;
}
