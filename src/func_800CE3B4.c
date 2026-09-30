typedef struct {
    char c0;
    signed char f1;
    char c2;
    unsigned char f3;
    short c4;
    unsigned short f6;
    unsigned short f8;
    unsigned short fA;
} S;

extern short D_800E22A8[];
extern short D_800E22AA;
extern short D_800E22AC;
extern short D_800E22D0;
extern unsigned char D_800E22CC;

extern void func_800C2EAC(int a0);
extern void func_800C3098(int a0);
extern void func_800C2FF0(int a0, int a1);
extern void func_800C3238(int a0);
extern void func_800C3B04(short *a0);

void func_800CE3B4(int a0, int a1, S *a2)
{
    register int v asm("$2");
    unsigned char c;

    func_800C2EAC(3);
    func_800C3098(0x10);
    func_800C2FF0(0x20, 0x20);
    func_800C3238(2);
    v = a2->f6;
    D_800E22A8[0] = v;
    v = a2->f8;
    D_800E22AA = v;
    v = a2->fA;
    D_800E22AC = v;
    c = a2->f1;
    D_800E22D0 = (signed char)c;
    c = a2->f3;
    D_800E22CC = c * 2 - 0x60;
    func_800C3B04(D_800E22A8);
    func_800C3098(0x10);
}
