typedef struct {
    char c0;
    char c1;
    char c2;
    signed char f3;
    short f4;
    unsigned short f6;
    unsigned short f8;
    unsigned short fA;
} S;

extern short D_800F32E0[];
extern short D_800F32E2;
extern short D_800F32E4;
extern int D_800F32F0;
extern int D_800F32F4;
extern int D_800F32F8;
extern short D_800F3308;

extern void func_800C2EAC(int a0);
extern void func_800C3098(int a0);
extern void func_800C2FF0(int a0, int a1);
extern void func_800C3238(int a0);
extern void func_800C3324(short *a0);

void func_800CC644(int a0, int a1, S *a2)
{
    register int v asm("$2");
    unsigned char c;

    func_800C2EAC(3);
    func_800C3098(0x10);
    func_800C2FF0(0x20, 0x20);
    func_800C3238(3);
    v = a2->f6;
    D_800F32E0[0] = v;
    v = a2->f8;
    D_800F32E2 = v;
    v = a2->fA;
    D_800F32E4 = v;
    v = a2->f4;
    D_800F32F0 = v;
    v = a2->f4;
    D_800F32F4 = v;
    v = a2->f4;
    D_800F32F8 = v;
    c = a2->f3;
    D_800F3308 = (signed char)c;
    func_800C3324(D_800F32E0);
}
