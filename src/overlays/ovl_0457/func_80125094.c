/* ovl_0457 (PE.IMG credits/XA overlay, VRAM 0x80120D00)
 * func_80125094 — blob offset 0x4394, 0x3B0 bytes. Profile ovl_0457_dispatch_801235BC (local: -O2 -G0,
 * three-word symbol store + MASPSX_DISPATCH_FOLD=jtbl_801235BC); LINK_EXACT at the overlay VMA
 * (docs/evidence/ovl_0457-func_80125094/REPORT.md). Credits page step: blit the current page, pad
 * left/right page select, then the XA/stream state machine (D_80172CCC 0..5).
 * Lever: (char *) on the D_80011610 base keeps retail's off+const-then-base association. */

typedef struct { short x, y, w, h; } RECT;
typedef struct {
    short x, y;
    unsigned char *p;
    unsigned short a, b;
    int q;
} Page;
extern int D_80172CBC;
extern Page D_80125B84[];
extern unsigned int D_8009D26C;
extern unsigned int D_8009D1F4;
extern unsigned short D_80172CC4;
extern unsigned int D_80172CCC;
extern unsigned short D_80172CC8;
extern int D_80172CC0;
extern unsigned int D_80172CD4;
extern int D_80125BA4[];
extern unsigned short D_80093168;
extern unsigned short D_80093166;
extern int D_80011610;
extern void func_800750CC();
extern void func_80074DC0();
extern unsigned int func_8010BF94();
extern void func_8010C86C();
extern int func_8010C89C();
extern void func_8010BE3C();
extern void func_8010BFA0();
extern void func_8010C01C();
extern int func_8010C078();
extern void func_8007506C();
void func_80125094(void)
{
    RECT r;
    int off;
    unsigned int v;

    r.x = D_80125B84[D_80172CBC].x;
    r.y = D_80125B84[D_80172CBC].y;
    r.w = 480;
    r.h = 224;
    func_800750CC(&r, D_80125B84[D_80172CBC].p);
    func_80074DC0(0);
    if ((int)D_8009D26C < 0) {
        if (D_8009D1F4 & 0x1000000) {
            if (D_80172CC4 != 0) {
                v = D_80172CC4 - 1;
                goto set;
            }
        } else if (D_8009D1F4 & 0x2000000) {
            if (D_80172CC4 < 30) {
                v = D_80172CC4 + 1;
            set:
                D_80172CC4 = v;
                D_80172CCC = 0;
            }
        }
    }
    switch (D_80172CCC) {
    case 0:
        func_8010C86C(func_8010BF94(D_80125BA4[D_80172CC4]) >> 4);
        off = (D_80093168 - D_80093166) << 11;
        func_8010C89C(D_80125BA4[D_80172CC4], (char *)D_80011610 + (off + 0x38800), (char *)D_80011610 + (off + 0x43000));
        D_80172CCC = 1;
        break;
    case 1:
        if (func_8010C89C(0, 0, (char *)D_80011610 + (((D_80093168 - D_80093166) << 11) + 0x43000)) == 0) {
            D_80172CCC = 2;
            D_80172CC8 = 0;
        }
        break;
    case 2:
        D_80172CC0 = D_80125B84[D_80172CBC].q;
        func_8010BE3C(0);
        func_8010BFA0((char *)D_80011610 + (((D_80093168 - D_80093166) << 11) + 0x38800), 1);
        func_8010C01C(D_80172CC0, 0x3C00);
        D_80172CCC = 3;
        break;
    case 3:
        if (func_8010C078(1) == 0) {
            D_80172CCC = 4;
            D_80172CC8 = D_80125B84[D_80172CBC].a + 120;
        }
        break;
    case 4:
        if (D_80172CC8 < D_80125B84[D_80172CBC].a + 360) {
            r.x = D_80172CC8;
            r.y = D_80125B84[D_80172CBC].b + 52;
            r.w = 24;
            r.h = 120;
            func_8007506C(&r, D_80172CC0);
            D_80172CC0 += 0x1800;
            D_80172CC8 += 24;
        } else {
            D_80172CCC = 5;
        }
        break;
    case 5:
        if (D_80172CD4 % 450 == 0) {
            D_80172CCC = 6;
        }
        break;
    case 6:
    case 7:
        break;
    }
}
