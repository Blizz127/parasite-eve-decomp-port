/* ovl_0700 (PE.IMG map-exit overlay, VRAM 0x8018EFF0)
 * func_80191EFC — blob offset 0x2F0C, 0xEC bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the overlay VMA (docs/evidence/ovl_0700-func_80191EFC/REPORT.md).
 * Project pos[0..2] through D_8019CC30 (func_8006DFA8) and hand the screen coords to func_800868F0/func_80086A28. Same &D_800BCFA4 pointer-local lever as func_80191E30. */

typedef struct { short vx, vy, vz, pad; } SVECTOR;
extern unsigned char D_8019CC30;
extern void *D_800BCFA4;
extern void *D_800BCFA8;
extern void func_80078A94();
extern void func_80078E94();
extern void func_80078E04();
extern void func_8006DFA8();
extern void func_800868F0();
extern void func_80086A28();
extern void func_80078B38();
void func_80191EFC(int a0, int *pos)
{
    int cfg[2];
    SVECTOR v;
    int sx, sy;
    void **p;
    void *save_a4;
    void *save_a8;
    cfg[0] = 0x300;
    func_80078A94();
    func_80078E94(&D_8019CC30);
    func_80078E04(&D_8019CC30);
    p = &D_800BCFA4;
    save_a4 = *p;
    *p = &D_8019CC30;
    save_a8 = D_800BCFA8;
    D_800BCFA8 = cfg;
    v.vx = pos[0];
    v.vy = pos[1];
    v.vz = pos[2];
    func_8006DFA8(&v, &sx, &sy);
    func_800868F0(a0, 0, sy);
    func_80086A28(a0, 0, sx);
    *p = save_a4;
    D_800BCFA8 = save_a8;
    func_80078B38();
}
