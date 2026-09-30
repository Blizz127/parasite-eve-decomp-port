typedef struct { short m[3][3]; int t[3]; } MATRIX;

extern unsigned char D_800E2250[];
extern MATRIX D_800F33C0;
extern MATRIX D_800F32B0;
extern unsigned char D_800F3460[];
extern void func_800C2EAC(unsigned int a0);
extern void func_800C3098(int a0);
extern void func_800C2FF0(int a0, int a1);
extern void func_800C3238(int a0);
extern void func_800C42A4(unsigned char *a0, MATRIX *a1, int a2);

void func_800CD404(int a0, int a1, unsigned char *a2)
{
    unsigned short i;
    unsigned char *p;

    func_800C2EAC(3);
    func_800C3098(0x10);
    func_800C2FF0(0x20, 0x20);
    func_800C3238(2);
    D_800F33C0.t[0] = *(short *)(a2 + 8);
    D_800F33C0.t[1] = *(short *)(a2 + 0xA);
    D_800F33C0.t[2] = *(short *)(a2 + 0xC);
    D_800E2250[4] = a2[3] * 2 - 0x80;
    func_800C42A4(D_800E2250, &D_800F33C0, 1);
    func_800C3238(3);
    for (i = 0; i < 2; i++) {
        p = a2 + i * 8;
        D_800F32B0.t[0] = *(short *)(p + 0x10);
        D_800F32B0.t[1] = *(short *)(p + 0x12);
        D_800F32B0.t[2] = *(short *)(p + 0x14);
        func_800C42A4(D_800F3460, &D_800F32B0, 1);
    }
}
