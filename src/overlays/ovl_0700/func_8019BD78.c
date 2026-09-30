/* ovl_0700 (PE.IMG map-exit overlay, VRAM 0x8018EFF0)
 * func_8019BD78 — blob offset 0xCD88, 0x1D8 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the overlay VMA (docs/evidence/ovl_0700-func_8019BD78/REPORT.md).
 * Graphics init: geometry/fog/back colour, the two-buffer DB at D_8019C1F8 (ClearOTag, draw/disp envs, isbg/rgb cleared), clear both 320x240 halves of VRAM, enable display, D_8019C9C0 = &db[0]. */

typedef struct { short x, y, w, h; } RECT;
typedef struct {
    RECT clip;
    short ofs[2];
    RECT tw;
    unsigned short tpage;
    unsigned char dtd;
    unsigned char dfe;
    unsigned char isbg;
    unsigned char r0, g0, b0;
    unsigned long dr_env[16];
} DRAWENV;
typedef struct {
    RECT disp;
    RECT screen;
    unsigned char isinter;
    unsigned char isrgb24;
    unsigned char pad0, pad1;
} DISPENV;
typedef struct {
    unsigned char *primptr;
    unsigned long *ot;
    DRAWENV draw;
    DISPENV disp;
} DB;
extern DB D_8019C1F8[2];
extern DB *D_8019C9C0;
extern unsigned long *D_800B0E38;
extern unsigned long *D_800B0E3C;
extern void func_80074D28();
extern void func_80078FC4();
extern void func_80078FE4();
extern void func_80077E64();
extern void func_80079004();
extern void func_80079024();
extern void func_8019BF8C();
extern void func_800752AC();
extern void func_80074924();
extern void func_800749D8();
extern void func_80074F44();
extern void func_80074DC0();
void func_8019BD78(void)
{
    RECT r;
    func_80074D28(0);
    func_80078FC4(0x80, 0x80, 0x80);
    func_80078FE4(0, 0, 0);
    func_80077E64(0x1964, 0x2CEC, 0x300);
    func_80079004(0xA0, 0x78);
    func_80079024(0x300);
    func_8019BF8C(&D_8019C1F8[0]);
    func_8019BF8C(&D_8019C1F8[1]);
    D_8019C1F8[0].ot = D_800B0E38;
    D_8019C1F8[1].ot = D_800B0E3C;
    func_800752AC(D_8019C1F8[0].ot, 0x1000);
    func_800752AC(D_8019C1F8[1].ot, 0x1000);
    func_80074924(&D_8019C1F8[0].draw, 0, 0, 320, 240);
    func_800749D8(&D_8019C1F8[0].disp, 0, 240, 320, 240);
    func_80074924(&D_8019C1F8[1].draw, 0, 240, 320, 240);
    func_800749D8(&D_8019C1F8[1].disp, 0, 0, 320, 240);
    D_8019C1F8[0].draw.isbg = 0;
    D_8019C1F8[1].draw.isbg = 0;
    D_8019C1F8[0].draw.r0 = 0;
    D_8019C1F8[0].draw.g0 = 0;
    D_8019C1F8[0].draw.b0 = 0;
    D_8019C1F8[1].draw.r0 = 0;
    D_8019C1F8[1].draw.g0 = 0;
    D_8019C1F8[1].draw.b0 = 0;
    r.x = 0; r.y = 0; r.w = 320; r.h = 240;
    func_80074F44(&r, 0, 0, 0);
    r.x = 0; r.y = 240; r.w = 320; r.h = 240;
    func_80074F44(&r, 0, 0, 0);
    func_80074DC0(0);
    func_80074D28(1);
    D_8019C9C0 = D_8019C1F8;
}
