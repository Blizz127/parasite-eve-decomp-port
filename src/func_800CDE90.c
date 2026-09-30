typedef struct {
    char c0;
    signed char f1;
    short c2;
    unsigned short f4;
    unsigned short f6;
    unsigned short f8;
    unsigned short fA;
    unsigned short fC;
    unsigned short fE;
} S;

extern short D_800F33E8[];
extern short D_800F33EA;
extern short D_800F33EC;
extern short D_800F3410;

extern void func_800C2EAC(int a0);
extern void func_800C3098(int a0);
extern void func_800C2FF0(int a0, int a1);
extern void func_800C3238(int a0);
extern void func_800C3B04(short *a0);

void func_800CDE90(int a0, int a1, S *a2)
{
    register int v asm("$2");
    unsigned char c;

    func_800C2EAC(3);
    func_800C3098(0x100);
    func_800C2FF0(0x20, 0x20);
    func_800C3238(3);
    v = a2->f4;
    v += a2->fA;
    D_800F33E8[0] = v;
    v = a2->f6;
    v += a2->fC;
    D_800F33EA = v;
    v = a2->f8;
    v += a2->fE;
    D_800F33EC = v;
    c = a2->f1;
    D_800F3410 = (signed char)c;
    func_800C3B04(D_800F33E8);
}
