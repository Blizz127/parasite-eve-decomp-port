/* ovl_0457 (PE.IMG credits/XA overlay, VRAM 0x80120D00)
 * func_80123E08 — blob offset 0x3108, 0x184 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the overlay VMA (docs/evidence/ovl_0457-func_80123E08/REPORT.md).
 * Credits init: display/graph reset, scroll state (D_80172CA4 = 0xE000, D_80172CA8 = -0x100), clear all of VRAM, parse the embedded TIM D_8012682C (pixel/CLUT pointers, TPage and CLUT ids), start the libpress decoder (func_8010BE3C/func_8010BD4C) on the stream buffer, enable display. */

typedef struct { short x, y, w, h; } RECT;
extern int D_80172CA4;
extern int D_80172CA8;
extern int D_80172CDC;
extern unsigned short D_80172CC4;
extern int D_80172CBC;
extern unsigned char *D_80172C98;
extern unsigned short *D_80172C9C;
extern short *D_80172C90;
extern short *D_80172C94;
extern unsigned short D_80172CA0;
extern unsigned short D_80172CA2;
extern unsigned char D_8012682C;
extern unsigned char *D_80011610;
extern unsigned short D_80093166;
extern unsigned short D_80093168;
extern void func_80074A44();
extern void func_80074BB8();
extern void func_80074D28();
extern void func_80074F44();
extern void func_80074DC0();
extern unsigned char *func_800718D0();
extern unsigned short *func_800719C4();
extern short *func_80071964();
extern short *func_80071944();
extern unsigned short func_80077A64();
extern unsigned short func_80077AA4();
extern void func_8010BE3C();
extern void func_8010BD4C();
void func_80123E08(void)
{
    RECT r;
    func_80074A44(1);
    func_80074BB8(0);
    func_80074D28(0);
    D_80172CA4 = 0xE000;
    D_80172CA8 = -0x100;
    D_80172CDC = 1;
    D_80172CC4 = 0;
    D_80172CBC ^= 1;
    r.x = 0;
    r.y = 0;
    r.w = 0x3FF;
    r.h = 0x1FF;
    func_80074F44(&r, 0, 0, 0);
    func_80074DC0(0);
    func_800718D0(&D_8012682C);
    D_80172C98 = func_800718D0(&D_8012682C);
    D_80172C9C = func_800719C4(&D_8012682C);
    D_80172C90 = func_80071964(&D_8012682C);
    D_80172C94 = func_80071944(&D_8012682C);
    D_80172CA0 = func_80077A64(0, 0, D_80172C90[0], D_80172C90[1]);
    D_80172CA2 = func_80077AA4(D_80172C94[0], D_80172C94[1]);
    func_8010BE3C(0);
    func_8010BD4C(D_80011610 + (((D_80093168 - D_80093166) << 11) + 0x43000));
    func_80074D28(1);
}
