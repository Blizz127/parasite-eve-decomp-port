typedef struct {
    int p0;
    unsigned short f4;
    short f6;
    short f8;
    short fA;
    short fC;
} S;

extern int D_800F348C;
extern int D_800F3490;
extern int D_800F3494;
extern short D_800E27EA;

extern void func_800C2EAC(int a0);
extern void func_800C3098(int a0);
extern void func_800C2FF0(int a0, int a1);
extern void func_800C3238(int a0);
extern void func_800C42A4(void *a0, void *a1, int a2);

void func_800CD50C(int a0, int a1, S *a2)
{
    int *p;
    short *q;
    register int v asm("$2");


    func_800C2EAC(3);
    func_800C3098(0x10);
    func_800C2FF0(0x20, 0x20);
    func_800C3238(3);
    p = &D_800F348C;
    q = &D_800E27EA;
    v = a2->f8;
    *p = v;
    v = a2->fA;
    D_800F3490 = v;
    v = a2->fC;
    D_800F3494 = v;
    v = a2->f4;
    *q = v;
    func_800C42A4((void *)(q - 5), (void *)(p - 5), 1);
}
