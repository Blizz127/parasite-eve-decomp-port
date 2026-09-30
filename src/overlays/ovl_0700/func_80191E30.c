/* ovl_0700 (PE.IMG map-exit overlay, VRAM 0x8018EFF0)
 * func_80191E30 — blob offset 0x2E40, 0xCC bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the overlay VMA (docs/evidence/ovl_0700-func_80191E30/REPORT.md).
 * Push matrix, install D_8019CC30 + a local 0x300 config in the D_800BCFA4/A8 globals around func_8006DF50, restore, pop. A pointer local for &D_800BCFA4 is load-bearing (keeps it in $s1; frame size). */

extern unsigned char D_801D0260;
extern unsigned char D_8019CC30;
extern void *D_800BCFA4;
extern void *D_800BCFA8;
extern int func_8006EC6C();
extern void func_80078A94();
extern void func_80078E94();
extern void func_80078E04();
extern int func_8006DF50();
extern void func_80078B38();
int func_80191E30(int a0)
{
    int cfg[2];
    int h;
    void **p;
    void *save_a4;
    void *save_a8;
    int r;
    cfg[0] = 0x300;
    h = func_8006EC6C(&D_801D0260, 3);
    func_80078A94();
    func_80078E94(&D_8019CC30);
    func_80078E04(&D_8019CC30);
    p = &D_800BCFA4;
    save_a4 = *p;
    *p = &D_8019CC30;
    save_a8 = D_800BCFA8;
    D_800BCFA8 = cfg;
    r = func_8006DF50(h, a0, 0, 0x80, 1);
    *p = save_a4;
    D_800BCFA8 = save_a8;
    func_80078B38();
    return r;
}
