/* ovl_03C5 (PE.IMG movie-controller overlay, VRAM 0x80120D00)
 * func_80121004 — blob offset 0x304, 0x26C bytes. Profile ovl_03C5_three_word_symbol_store (local:
 * -O2 -G0 + MASPSX_THREE_WORD_SYMBOL_STORE=1); LINK_EXACT at the overlay VMA
 * (docs/evidence/ovl_03C5-func_80121004/REPORT.md). Movie display setup for buffer `buf`:
 * SetDefDispEnv/SetDefDrawEnv at 480 (24-bit, width x2/3) or 320 wide. `signed char` params give
 * retail's sll 24 / sra 24 (plain char is unsigned here). */

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
extern unsigned char D_800B0DBA;
extern int D_80122420;
extern int D_80122424;
extern DISPENV D_800BCE80[];
extern DRAWENV D_800BCDC8[];
extern unsigned char D_801223F6;
extern void func_800749D8();
extern void func_80074924();
void func_80121004(signed char buf, signed char wide)
{
    short y[2];

    if (buf == 0) {
        y[0] = 240;
        y[1] = 0;
    } else {
        y[0] = 0;
        y[1] = 240;
    }
    if (wide != 0) {
        D_801223F6 = 3;
        func_800749D8(&D_800BCE80[buf], 0, y[0], 480, 240);
        D_800BCE80[buf].isrgb24 = 1;
        D_800BCE80[buf].disp.w = D_800BCE80[buf].disp.w * 2 / 3;
        func_80074924(&D_800BCDC8[buf], 0, y[1], 480, 240);
        D_800BCDC8[buf].clip.w = D_800BCDC8[buf].clip.w * 2 / 3;
        D_800BCDC8[buf].isbg = 1;
        D_800BCDC8[buf].dtd = 1;
        D_800BCDC8[buf].dfe = 0;
        D_800BCDC8[buf].r0 = 0;
        D_800BCDC8[buf].g0 = 0;
        D_800BCDC8[buf].b0 = 0;
    } else {
        D_801223F6 = 2;
        func_800749D8(&D_800BCE80[buf], 0, y[0], 320, 240);
        func_80074924(&D_800BCDC8[buf], 0, y[1], 320, 240);
        D_800BCDC8[buf].isbg = 1;
        D_800BCDC8[buf].dtd = 1;
        D_800BCDC8[buf].dfe = 0;
        D_800BCDC8[buf].r0 = 0;
        D_800BCDC8[buf].g0 = 0;
        D_800BCDC8[buf].b0 = 0;
    }
}
