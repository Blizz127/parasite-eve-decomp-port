/* ovl_0457 (PE.IMG credits/XA overlay, VRAM 0x80120D00)
 * func_8012489C — blob offset 0x3B9C, 0x208 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the overlay VMA (docs/evidence/ovl_0457-func_8012489C/REPORT.md).
 * Credits init: display reset, ClearImage of the 1024x512 VRAM, the two
 * D_80125B88 frame-buffer pointers from the PE.IMG sector delta
 * (D_80093168 - D_80093166) << 11, stream/image setup through D_8012682C,
 * then both credit-layer inits and PutDispEnv on buffer D_8009CDDC.
 * Levers: `$2` pin on `off`, a separate `lo` for base+off with `off += 0x54000`
 * in place (retail adds before the base), volatile D_80172CC4 (stored twice). */
typedef struct { short x, y, w, h; } RECT;
typedef struct {
    RECT disp;
    RECT screen;
    unsigned char isinter;
    unsigned char isrgb24;
    unsigned char pad0, pad1;
} DISPENV;
extern int D_8009CDDC;
extern unsigned short D_80093168;
extern unsigned short D_80093166;
extern int D_80011610;
extern int D_80172CA4;
extern int D_80172CA8;
extern int D_80172CBC;
extern int D_80172CCC;
extern short D_80172CC8;
extern volatile short D_80172CC4;
extern int D_80125B88;
extern int D_80125B90;
extern int D_80125B98;
extern int D_80125BA0;
extern short *D_80172C90;
extern short *D_80172C94;
extern int D_80172C98;
extern int D_80172C9C;
extern short D_80172CA0;
extern short D_80172CA2;
extern char D_8012682C;
extern DISPENV D_800BCE80[];
extern void func_80074A44();
extern void func_80074BB8();
extern void func_80074D28();
extern void func_80074F44();
extern void func_80074DC0();
extern void func_8010BE3C();
extern void func_8010BD4C();
extern int func_80071994();
extern int func_800719C4();
extern short *func_80071964();
extern short *func_80071944();
extern short func_80077A64();
extern short func_80077AA4();
extern void func_80124F40();
extern void func_800755F0();
void func_8012489C(void)
{
    RECT r;
    unsigned short *p;
    char *s;
    register int off asm("$2");
    int base;
    int lo;

    func_80074A44(1);
    func_80074BB8(0);
    func_80074D28(0);
    D_80172CA4 = 0xE000;
    D_80172CA8 = -0x100;
    p = &D_80093168;
    r.x = 0;
    r.y = 0;
    r.w = 0x3FF;
    r.h = 0x1FF;
    D_80172CBC = 0;
    D_80172CCC = 0;
    D_80172CC8 = 0;
    D_80172CC4 = 0;
    D_80172CC4 = 0;
    off = (*p - D_80093166) << 11;
    base = D_80011610;
    lo = base + off;
    off += 0x54000;
    D_80125B88 = lo;
    D_80125B90 = base + off;
    D_80125B98 = base + off;
    D_80125BA0 = lo;
    func_80074F44(&r, 0, 0, 0);
    func_80074DC0(0);
    func_8010BE3C(0);
    func_8010BD4C((char *)D_80011610 + (((*p - D_80093166) << 11) + 0x43000));
    s = &D_8012682C;
    D_80172C98 = func_80071994(s);
    D_80172C9C = func_800719C4(s);
    D_80172C90 = func_80071964(s);
    D_80172C94 = func_80071944(s);
    D_80172CA0 = func_80077A64(0, 0, D_80172C90[0], D_80172C90[1]);
    D_80172CA2 = func_80077AA4(D_80172C94[0], D_80172C94[1]);
    func_80124F40(0);
    func_80124F40(1);
    func_800755F0(&D_800BCE80[D_8009CDDC]);
    func_80074D28(1);
}
