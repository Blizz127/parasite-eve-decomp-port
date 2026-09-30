/* ovl_0700 (PE.IMG subsystem overlay, VRAM 0x8018EFF0)
 * func_8019234C — blob offset 0x335C, 0x3F4 bytes. Profile era_o2_g0 (default).
 * Map-exit overlay main loop: per-frame update, ordering-table relink scan over the
 * current double buffer, VSync/Put{Draw,Disp}Env/DrawOTag, buffer swap + ClearOTagR, then
 * the destination chooser func_80192030. Levers: scan/i initialised before the double-buffer
 * load (reorg then steals them into the func_80037870 guard's delay slots as retail);
 * `db->ot[i]` read in place; the swap uses its own pointer local (reusing `db` makes it the
 * first-allocated loop register). Evidence: docs/evidence/ovl8-lane-2026-09-28/REPORT.md */
typedef struct {
    int pad0;
    unsigned int *ot;
    unsigned char draw[0x5C];
    unsigned char disp[0x14];
} DB;

extern int D_8009CDDC;
extern unsigned char D_8019C00E;
extern short D_8019C034;
extern int D_8019C03C;
extern short D_8019C024;
extern unsigned int D_8009D26C;
extern short D_8019C02A;
extern unsigned char D_8019C330[];
extern unsigned char *D_801EA578;
extern int D_8019C0C0;
extern unsigned char D_8019C044;
extern unsigned char D_8019C045;
extern DB *D_8019C9C0;
extern int D_8019CC14;
extern DB D_8019C1F8[2];

extern void func_80196498();
extern void func_80191DE8();
extern int func_80073A44();
extern void func_80071A64();
extern int func_80071A54();
extern void func_8003EB04();
extern void func_80074D28();
extern void func_8006A25C();
extern void func_801942FC();
extern void func_8018F05C();
extern void func_8018F92C();
extern short func_80194108();
extern void func_80192740();
extern void func_80192800();
extern void func_80193478();
extern int func_80191E30();
extern void func_80191EFC();
extern void func_80037870();
extern void func_80074DC0();
extern void func_80193AB0();
extern void func_80074A44();
extern void func_80075424();
extern void func_800755F0();
extern void func_800753B4();
extern void func_800752AC();
extern void func_8019BF8C();
extern void func_80192030();

void func_8019234C(void)
{
    int idx;
    unsigned char started;
    int h;
    int i;
    int scan;
    DB *db;
    DB *nx;
    unsigned int *ot;

    idx = 0;
    D_8009CDDC = 0;
    D_8019C00E = 0;
    func_80196498();
    func_80191DE8(D_8019C034 != 10);
    func_80071A64(func_80073A44(-1));
    started = 0;
    D_8019C03C = func_80071A54() % 3000000;
    h = 0;
    if (D_8019C034 == 10) {
        D_8019C03C = 120000;
    }
    while (D_8019C024 == 0) {
        func_8003EB04();
        if ((D_8009D26C & 0xF000006) == 0xF000006) {
            func_80073A44(0);
            func_80074D28(0);
            D_8019C00E = 1;
            D_8019C024 = 1;
            func_8006A25C();
        }
        func_801942FC();
        if (D_8019C00E != 0) continue;
        func_8018F05C();
        func_8018F92C(D_8019C330);
        if (D_8019C02A != 0) {
            D_8019C02A = func_80194108(D_8019C02A);
        }
        func_80192740();
        func_80192800();
        func_80193478();
        if (started == 0) {
            h = func_80191E30(0xABE, D_801EA578 + 0x1C);
            started = 1;
        } else if (D_8019C0C0 == 0) {
            func_80191EFC(h, D_801EA578 + 0x1C);
        }
        if (D_8019C034 == 10 || (D_8019C044 == 1 && D_8019C045 == 0)) {
            func_80037870();
        }
        scan = 1;
        i = 0xFFF;
        db = D_8019C9C0;
        for (; i >= 0; i--) {
            if (scan) {
                if ((db->ot[i] | 0x80000000) == (unsigned int)&db->ot[i - 1]) {
                    idx = i;
                    scan = 0;
                }
            } else {
                if ((db->ot[i] | 0x80000000) != (unsigned int)&db->ot[i - 1]) {
                    db->ot[idx] = (unsigned int)&db->ot[i] & 0xFFFFFF;
                    scan = 1;
                }
            }
        }
        D_8019CC14 = func_80073A44(1);
        func_80074DC0(0);
        func_80073A44(2);
        func_80193AB0();
        func_80074A44(1);
        func_80075424(D_8019C9C0->draw);
        func_800755F0(D_8019C9C0->disp);
        func_800753B4(&D_8019C9C0->ot[0xFFF]);
        nx = &D_8019C1F8[0];
        if (D_8019C9C0 == nx) {
            nx = &D_8019C1F8[1];
        }
        D_8019C9C0 = nx;
        D_8009CDDC ^= 1;
        func_800752AC(nx->ot, 0x1000);
        func_8019BF8C(D_8019C9C0);
    }
    if (D_8019C00E == 0) {
        func_80192030();
    }
}
