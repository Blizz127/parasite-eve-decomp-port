/* ovl_0457 (PE.IMG credits/XA overlay, VRAM 0x80120D00)
 * func_80124F40 — blob offset 0x4240, 0x154 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the overlay VMA (docs/evidence/ovl_0457-func_80124F40/REPORT.md).
 * Set up display/draw env pair i (D_800BCE80 / D_800BCDC8): 320x224 display at y 0 or 224, RGB24 on, screen y 8 / h 224; 480x224 draw env at the other half, clip width scaled by 2/3, dtd/isbg cleared (includes retail's disp.w self-assignment). */

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
extern DISPENV D_800BCE80[];
extern DRAWENV D_800BCDC8[];
extern void func_800749D8();
extern void func_80074924();
void func_80124F40(unsigned short i)
{
    int dispy, drawy;
    if (i != 0) {
        dispy = 0;
        drawy = 0xE0;
    } else {
        dispy = 0xE0;
        drawy = 0;
    }
    func_800749D8(&D_800BCE80[i], 0, dispy, 320, 224);
    D_800BCE80[i].isrgb24 = 1;
    D_800BCE80[i].screen.y = 8;
    D_800BCE80[i].screen.h = 224;
    D_800BCE80[i].disp.w = D_800BCE80[i].disp.w;
    func_80074924(&D_800BCDC8[i], 0, drawy, 480, 224);
    D_800BCDC8[i].clip.w = D_800BCDC8[i].clip.w * 2 / 3;
    D_800BCDC8[i].dtd = 0;
    D_800BCDC8[i].isbg = 0;
}
