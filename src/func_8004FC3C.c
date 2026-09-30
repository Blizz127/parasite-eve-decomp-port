extern int func_80054288();
extern int func_800556E8(int);
extern int func_8004324C(int);

int func_8004FC3C(int a0) {
    if (a0 < func_80054288()) {
        return func_8004324C(func_800556E8(a0));
    }
    return 0;
}
