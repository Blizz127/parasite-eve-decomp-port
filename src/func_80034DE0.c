/* VRAM 0x80034DE0 / file 0x255E0 / size 0x130. */
typedef struct {
    unsigned char b0;
    unsigned char pad1[3];
    unsigned int w4;
    unsigned int w8;
    unsigned int flags;
    short half10;
    unsigned char pad2[0x26];
} BceRecord;

extern BceRecord D_800BCEA8[];
extern unsigned char D_8009D1CE;
extern unsigned char D_8009CE80;
extern unsigned char D_8009CE88;
extern unsigned int D_8009D1F8;

extern void func_800374E8(void);
extern int func_8005BCB0(void);
extern void func_80037454(unsigned short arg0, unsigned short arg1,
                          unsigned short arg2, unsigned short arg3);
extern void func_800375E0(int a0, int a1, short *a2);
extern void func_8005E894(int a0, int a1);
extern void func_80061C34(int a0, int a1, int a2, int a3);

void func_80034DE0(void) {
    short v;

    switch (D_8009D1CE) {
    case 1:
        v = -1;
        func_800374E8();
        func_80037454(func_8005BCB0() ? 0x14 : 0x61, D_8009CE80 < 2 ? 0xF : 0xC3, 0, 0);
        func_800375E0(0, 2, &v);
        D_800BCEA8[0].b0 = 2;
        D_8009CE88 = 0x4B;
        D_800BCEA8[0].w4 = D_8009D1F8;
        D_800BCEA8[0].flags |= 0x2000000;
        D_8009D1CE++;
        break;
    case 2:
        if (D_8009CE88 == 0) {
            func_800374E8();
            D_8009D1CE = 0;
        }
        break;
    }
    D_8009CE88--;
    func_8005E894(0, D_8009CE80 < 2 ? 0xB : 0xBF);
    func_80061C34(0x140, 0x14, 0, 0);
}
