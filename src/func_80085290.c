typedef struct {
    unsigned char pad0[0x38];
    int f38;
    int f3C;
    unsigned char pad40[0x10];
    int f50;
    short f54;
    unsigned char pad56[0x1A];
    short f70;
    short pad72;
    short f74;
    unsigned char pad76[0x62];
    short fD8;
    unsigned char padDA[0x16];
    int fF0;
    unsigned char padF4[0x28];
} Voice;

typedef struct {
    unsigned char pad0[0x10];
    int f10;
    int f14;
    int f18;
    unsigned char pad1C[0x24];
    int f40;
    int f44;
    unsigned char pad48[0x10];
    short f58;
    unsigned char pad5A[0x1E];
    int f78;
    int f7C;
    int f80;
    unsigned char pad84[0x24];
    int fA8;
    int fAC;
    unsigned char padB0[0x10];
    short fC0;
} Ctl;

extern Voice D_800B8AC0[];
extern Voice D_800BC000[];
extern Ctl D_800B6980;
extern int *D_8009D2C8;
extern int D_800C0D90;
extern int D_800B6A30, D_800B69C8, D_8009D2B4, D_8009D2C0, D_800BCD50, D_800B6984, D_800B6988;
extern short D_800B69D4;
extern int D_800BCD60, D_800B699C, D_800B6A04, D_800B69EC, D_800B69F0;
extern short D_800B6A3C, D_800B6A38, D_800B69D0, D_8009D21E;
extern int D_8009D2CC;
extern short D_8009D220;
extern int D_8009D2D0;
extern short D_8009D2A2;
extern int D_800BCD6C, D_800B69B4, D_800BCD70, D_800B69B8, D_800BCD74, D_800B69BC;
extern int D_800B6A24, D_800B6A20, D_800B6A1C, D_800B8628;
extern short D_800B69E0, D_800B69DE, D_800B69DC, D_800B69E4;
extern short D_800C0D96, D_800C0D94, D_800C0D98, D_800C0D9A, D_800C0DA2, D_800C0DA0;
extern int D_800C0DA4, D_800C0DA8;
extern short D_800C0DAE, D_800C0DAC;
extern int D_800C0DB0, D_800C0DB4;
extern volatile int D_8009D268;
extern int D_8009D22C, D_8009D2B8, D_800C0DD8, D_800C0DD4, D_800C0DD0;
extern int D_8009D2F4, D_8009D2DC, D_8009D2E0;
extern int D_800BCD68, D_800BCD64, D_800BCD5C, D_800BCD58, D_800BCD54;
extern unsigned int D_8009D2C4;

extern void func_80085F74(int *);
extern void func_800862F4(int, int, int, int, int);
extern void func_8008CB54(int);
extern void func_80085A64(int);

void func_80085290(void)
{
    unsigned short i;
    Voice *p;
    int *c;
    int *q;

    q = &D_800C0D90;

    D_8009D2C8 = (int *)&D_800B6980;
    D_8009D2C0 = 1;
    D_800B6A30 = 0x7F0000;
    D_800B69C8 = 0x7F0000;
    D_8009D2B4 = 0x7FFF0000;
    D_800BCD50 = 0;
    D_800B6984 = 0;
    D_800B6988 = 0;
    D_800B69D4 = 0;
    D_800BCD60 = 0;
    D_800B699C = 0;
    D_800B6A04 = 0;
    D_800B69EC = 0;
    D_800B69F0 = 0;
    D_800B6A3C = 0;
    D_800B6A38 = 0;
    D_800B69D0 = 0;
    D_8009D21E = 0;
    D_8009D2CC = 0;
    D_8009D220 = 0;
    D_8009D2D0 = 0;
    D_8009D2A2 = 0;
    D_800BCD6C = 0;
    D_800B69B4 = 0;
    D_800BCD70 = 0;
    D_800B69B8 = 0;
    D_800BCD74 = 0;
    D_800B69BC = 0;
    D_800B6A24 = 0;
    D_800B6A20 = 0;
    D_800B6A1C = 0;
    D_800B8628 = 0;
    D_800B69E0 = 0;
    D_800B69DE = 0;
    D_800B69DC = 0;
    D_800B69E4 = 0;
    *q = 0x3FCF;
    D_800C0D96 = 0x3FFF;
    D_800C0D94 = 0x3FFF;
    D_800C0D98 = 0;
    D_800C0D9A = 0;
    D_800C0DA2 = 0x7FFF;
    D_800C0DA0 = 0x7FFF;
    D_800C0DA4 = 0;
    D_800C0DA8 = 1;
    D_800C0DAE = 0;
    D_800C0DAC = 0;
    D_800C0DB0 = 0;
    D_800C0DB4 = 0;
    func_80085F74(q);
    D_8009D268 = 0;
    D_8009D22C = 0;
    D_8009D2B8 = 0;
    D_800C0DD8 = 0;
    D_800C0DD4 = 0;
    D_800C0DD0 = 0;
    D_8009D2E0 = D_8009D2DC = D_8009D2F4 = D_8009D268;

    p = D_800B8AC0;
    for (i = 0; i < 24; i++, p++) {
        p->f38 = 0;
        p->fF0 = 24;
        p->f54 = 0;
        p->f50 = 0;
        func_800862F4(i, 0, 0, 0, 0);
    }
    for (i = 0; i < 24; i++, p++) {
        p->f38 = 0;
        p->fF0 = 24;
        p->f54 = 0;
        p->f50 = 0;
        func_800862F4(i, 0, 0, 0, 0);
    }
    p = D_800BC000;
    for (i = 12; i < 24; i++, p++) {
        p->f38 = 0;
        p->fF0 = i;
        p->f54 = 1;
        p->f50 = 0;
        p->fD8 = 0x7F00;
        p->f74 = 0;
        p->f70 = 0;
        p->f3C = 0;
    }
    c = D_8009D2C8;
    *(int *)((char *)c + 24) = 0;
    *(int *)((char *)c + 20) = 0;
    *(int *)((char *)c + 16) = 0;
    *(int *)((char *)c + 128) = 0;
    *(int *)((char *)c + 124) = 0;
    *(int *)((char *)c + 120) = 0;
    D_800BCD68 = 1;
    D_800BCD64 = 0x66A80000;
    D_800BCD5C = 0;
    D_800BCD58 = 0;
    D_800BCD54 = 0;
    *(int *)((char *)c + 168) = 0x3FFF0000;
    *(int *)((char *)c + 64) = 0x3FFF0000;
    *(int *)((char *)c + 172) = 0;
    *(int *)((char *)c + 68) = 0;
    *(short *)((char *)c + 0xC0) = 0;
    *(short *)((char *)c + 0x58) = 0;
    D_8009D2C4 |= 0x80;
    func_8008CB54(4);
    func_80085A64(1);
}
