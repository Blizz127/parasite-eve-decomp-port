/* ovl_0700 (PE.IMG subsystem overlay, VRAM 0x8018EFF0)
 * func_80192030 — blob offset 0x3040, 0x31C bytes. Local profile ovl_0700_dispatch_8018F034
 * (-O2 -G0 + MASPSX_THREE_WORD_SYMBOL_STORE=1 + MASPSX_DISPATCH_FOLD=jtbl_8018F034: the
 * 10-entry switch table lives in the raw [0x0,bin] header at blob 0x44).
 * Map-exit destination chooser: picks the D_8009D280 transition word from the current map id
 * (D_800A7918) and destination slot (D_8019CA68), then fades out. Lever: D_800A77F4 = 999 is
 * stored before the D_8009D280 seed (6w otherwise). Evidence: docs/evidence/ovl8-lane-2026-09-28/REPORT.md */
typedef struct { short x, y, w, h; } RECT;
extern int D_800A77FC;
extern int D_800A7918;
extern unsigned int D_8009D280;
extern int D_800A77F4;
extern unsigned int D_8019CA68;
extern signed char D_800B0DB5;
extern signed char D_800B0DB4;
extern signed char D_800B0DB2;
extern unsigned int D_800B0CD8;
extern unsigned char D_800BCE80[];
extern void func_80074D28();
extern void func_80074A44();
extern void func_80074F44();
extern void func_80074DC0();
extern void func_800755F0();
extern void func_80086C5C();
extern void func_800868AC();
extern void func_80073A44();
extern void func_80086FF8();
extern void func_80087024();
extern void func_80038D48();

void func_80192030(void)
{
    unsigned char f;
    int b;
    int r;
    int i;
    RECT rc;

    f = 1;
    b = D_800A77FC & 0x2000;
    D_800A77F4 = 999;
    D_8009D280 = 0xA9400048;
    r = D_800A7918;
    if (r == 0x80) {
        D_8009D280 = 0xA80650C8;
    } else if (r == 0x78) {
        D_8009D280 = 0xA8003348;
        f = 0;
    } else if (r == 0x208) {
        D_8009D280 = 0xA80032C8;
        f = 0;
    } else if (r == 0x210) {
        if (b) {
            D_8009D280 = 0xA8029148;
        } else {
            D_8009D280 = 0xA80290C8;
        }
    } else {
        switch (D_8019CA68) {
        case 0:
            D_8009D280 = 0xA80281C8;
            break;
        case 1:
            D_8009D280 = 0xA80260C8;
            break;
        case 2:
            D_8009D280 = 0xA8009348;
            break;
        case 3:
            D_8009D280 = 0xA8046048;
            break;
        case 4:
            if (r == 0x1C0) {
                D_8009D280 = 0xA80034C8;
                f = 0;
            } else if (b) {
                D_8009D280 = 0xA80222C8;
            } else {
                D_8009D280 = 0xA80630C8;
            }
            break;
        case 5:
            D_8009D280 = 0xA8049048;
            break;
        case 6:
            D_8009D280 = 0xA8002248;
            break;
        case 7:
            if (r == 0xD0) {
                D_8009D280 = 0xA80033C8;
                f = 0;
            } else if (r == 0x178) {
                D_8009D280 = 0xA80033C8;
                f = 0;
            } else if ((unsigned int)(r - 0x180) < 0x88) {
                D_8009D280 = 0xA80201C8;
            } else {
                D_8009D280 = 0xA8004348;
            }
            break;
        case 8:
            if (r == 0xE0) {
                D_8009D280 = 0xA80034C8;
                f = 0;
            } else if (r >= 0x128) {
                D_8009D280 = 0xA8005448;
            } else {
                D_8009D280 = 0xA8067248;
            }
            break;
        case 9:
            if (r == 0xB8 || r == 0x148) {
                D_8009D280 = 0xA8003448;
                f = 0;
            } else if (b) {
                D_8009D280 = 0xA8029148;
            } else {
                D_8009D280 = 0xA80290C8;
            }
            break;
        }
    }
    func_80074D28(0);
    func_80074A44(1);
    rc.x = 0;
    rc.y = 0;
    rc.w = 0x140;
    rc.h = 0x1C0;
    func_80074F44(&rc, 0, 0, 0);
    func_80074DC0(0);
    func_800755F0(D_800BCE80);
    if (f == 1) {
        func_80086C5C(D_800B0DB5, 0x1E, 0);
    }
    func_800868AC(0x1E, 0);
    for (i = 0; i < 0x1E; i++) {
        func_80073A44(0);
    }
    if (f == 1) {
        D_800B0DB4 = -1;
        D_800B0DB2 = -1;
        D_800B0CD8 &= ~0x40;
        func_80086FF8();
    }
    func_80087024();
    func_80038D48();
}
