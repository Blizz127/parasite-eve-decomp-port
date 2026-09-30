extern int D_8009CF40;
extern int D_800A18FC[];
extern int D_800A18D8[];
extern int D_800A1920[];
extern int D_800A1940[];
extern void func_8005BA78(int, int, int *, int *);
extern void func_8006062C(int, int);

void func_800437B4(int a0) {
    int l10;
    int l14;
    int *p;
    int *q;
    int cur;
    int target;
    int cnt;

    func_8005BA78(a0, D_800A18D8[a0] + ((D_8009CF40 * D_800A18FC[a0]) >> 7),
                  &l10, &l14);
    p = &D_800A1920[a0];
    cur = *p;
    target = l14;
    if (target >= cur) {
        *p = (cur + target) >> 1;
    } else if (cur < 0x30) {
        *p = 0x30;
        D_800A1940[a0] = 4;
    } else {
        q = &D_800A1940[a0];
        cnt = *q;
        if (cnt > 0) {
            cnt -= 2;
        }
        *q = cnt;
        if (cnt == 0) {
            *p = target;
        }
    }
    func_8006062C(l10, D_800A1920[a0]);
}
