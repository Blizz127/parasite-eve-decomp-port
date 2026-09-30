/* ovl_0457 (PE.IMG credits/XA overlay, VRAM 0x80120D00)
 * func_80124AA4 — blob offset 0x3DA4, 0x160 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the overlay VMA (docs/evidence/ovl_0457-func_80124AA4/REPORT.md).
 * Per-frame image flip: LoadImage the current D_80125B88 frame into the back half, advance the D_80172CC4 frame counter when state 6 (-> state 0, or 7 after 31 frames), then DrawSync/VSync/PutDispEnv/PutDrawEnv on buffer D_8009CDDC and toggle it. */

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
typedef struct { unsigned char *p; int pad[3]; } Img;
extern int D_8009CDDC;
extern int D_80172CBC;
extern int D_80172CCC;
extern unsigned short D_80172CC4;
extern Img D_80125B88[];
extern DISPENV D_800BCE80[];
extern DRAWENV D_800BCDC8[];
extern void func_8007506C();
extern void func_80074DC0();
extern void func_80073A44();
extern void func_800755F0();
extern void func_80075424();
void func_80124AA4(void)
{
    RECT r;
    if (D_8009CDDC != 0) {
        r.x = 0;
        r.y = 0;
        r.w = 480;
        r.h = 224;
    } else {
        r.x = 0;
        r.y = 224;
        r.w = 480;
        r.h = 224;
    }
    func_8007506C(&r, D_80125B88[D_80172CBC].p);
    if (D_80172CCC == 6) {
        D_80172CC4++;
        D_80172CBC ^= 1;
        if (D_80172CC4 < 31) {
            D_80172CCC = 0;
        } else {
            D_80172CCC = 7;
        }
    }
    func_80074DC0(0);
    func_80073A44(2);
    func_800755F0(&D_800BCE80[D_8009CDDC]);
    func_80075424(&D_800BCDC8[D_8009CDDC]);
    D_8009CDDC ^= 1;
}
