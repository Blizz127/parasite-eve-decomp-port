/* ovl_0700 (PE.IMG map/camera subsystem overlay)
 * func_80195F6C — blob offset 0x6F7C, 0x52C bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the overlay VMA (docs/evidence/ovl14-lane-2026-09-28/REPORT.md).
 * Double-buffered POLY_G4 gradient + DR_TPAGE primitive init.  Lever: field stores written as
 * D[i].field (array element, not through &D[i]) so cse1 keeps sym+off(i*36) addressing
 * separate from the call-argument pointer; loop strength reduction then gives retail's
 * sb/sh sym+off($s1) form. */

typedef struct {
    unsigned int tag;
    unsigned char r0, g0, b0, code;
    short x0, y0;
    unsigned char r1, g1, b1, p1;
    short x1, y1;
    unsigned char r2, g2, b2, p2;
    short x2, y2;
    unsigned char r3, g3, b3, p3;
    short x3, y3;
} POLY_G4;
typedef struct {
    unsigned int tag;
    unsigned int code[1];
} DR_TPAGE;

extern POLY_G4 D_8019CB60[2];
extern POLY_G4 D_801EA598[2];
extern POLY_G4 D_8019C9D8[2];
extern POLY_G4 D_8019CA20[2];
extern DR_TPAGE D_8019CA70[2];
extern DR_TPAGE D_8019CA80[2];
extern DR_TPAGE D_8019CA98[2];
extern void func_80077BC4();
extern void func_80077B04();
extern void func_80077C84();

#define setRGB0(p, r, g, b) ((p).r0 = (r), (p).g0 = (g), (p).b0 = (b))
#define setRGB1(p, r, g, b) ((p).r1 = (r), (p).g1 = (g), (p).b1 = (b))
#define setRGB2(p, r, g, b) ((p).r2 = (r), (p).g2 = (g), (p).b2 = (b))
#define setRGB3(p, r, g, b) ((p).r3 = (r), (p).g3 = (g), (p).b3 = (b))
#define setXY4(p, _x0, _y0, _x1, _y1, _x2, _y2, _x3, _y3) \
    ((p).x0 = (_x0), (p).y0 = (_y0), (p).x1 = (_x1), (p).y1 = (_y1), \
     (p).x2 = (_x2), (p).y2 = (_y2), (p).x3 = (_x3), (p).y3 = (_y3))

void func_80195F6C(void)
{
    unsigned int i;

    for (i = 0; i < 2; i++) {
        func_80077BC4(&D_8019CB60[i]);
        setRGB0(D_8019CB60[i], 0x14, 0x44, 0x64);
        setRGB1(D_8019CB60[i], 0x14, 0x44, 0x64);
        setRGB2(D_8019CB60[i], 5, 0x11, 0x19);
        setRGB3(D_8019CB60[i], 5, 0x11, 0x19);
        setXY4(D_8019CB60[i], 0, 0, 0x140, 0, 0, 0xF0, 0x140, 0xF0);
        func_80077BC4(&D_801EA598[i]);
        setRGB0(D_801EA598[i], 0xA, 0x18, 0x28);
        setRGB1(D_801EA598[i], 0xA, 0x18, 0x28);
        setRGB2(D_801EA598[i], 5, 7, 0xF);
        setRGB3(D_801EA598[i], 5, 7, 0xF);
        setXY4(D_801EA598[i], 0, 0, 0x140, 0, 0, 0xF0, 0x140, 0xF0);
        func_80077BC4(&D_8019C9D8[i]);
        func_80077B04(&D_8019C9D8[i], 1);
        setRGB0(D_8019C9D8[i], 0, 0, 0);
        setRGB1(D_8019C9D8[i], 0, 0, 0);
        setRGB2(D_8019C9D8[i], 0xFF, 0xFF, 0xFF);
        setRGB3(D_8019C9D8[i], 0xFF, 0xFF, 0xFF);
        setXY4(D_8019C9D8[i], 0, 0xC8, 0x140, 0xC8, 0, 0xF0, 0x140, 0xF0);
        func_80077BC4(&D_8019CA20[i]);
        func_80077B04(&D_8019CA20[i], 1);
        setRGB0(D_8019CA20[i], 0xFF, 0xFF, 0xFF);
        setRGB1(D_8019CA20[i], 0xFF, 0xFF, 0xFF);
        setRGB2(D_8019CA20[i], 0, 0, 0);
        setRGB3(D_8019CA20[i], 0, 0, 0);
        setXY4(D_8019CA20[i], 0, 0, 0x140, 0, 0, 0x28, 0x140, 0x28);
        func_80077C84(&D_8019CA70[i], 0, 0, 0);
        func_80077C84(&D_8019CA80[i], 0, 0, 0x20);
        func_80077C84(&D_8019CA98[i], 0, 0, 0x40);
    }
}
