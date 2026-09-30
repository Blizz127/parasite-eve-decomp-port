extern short D_800F3430[];
extern void func_800C2EAC();
extern void func_800C3098();
extern void func_800C2FF0();
extern void func_800C3238();
extern void func_800C3B04();

void func_800CC480(int a0, unsigned char *a1, unsigned char *a2)
{
    short v;
    register unsigned char *p asm("$17");

    p = a2;
    func_800C2EAC(3);
    func_800C3098(0x10);
    func_800C2FF0(0x20, 0x20);
    func_800C3238(2);
    v = 0x80 - *(unsigned short *)(a1 + 2) * 2;
    if (v < 0) {
        v = 0;
    }
    D_800F3430[0x14] = v;
    D_800F3430[0] = *(unsigned short *)(p + 6);
    D_800F3430[1] = *(unsigned short *)(p + 8);
    D_800F3430[2] = *(unsigned short *)(p + 0xA);
    *(int *)(D_800F3430 + 8) = *(short *)(p + 4);
    *(int *)(D_800F3430 + 10) = *(short *)(p + 4) * 2;
    *(int *)(D_800F3430 + 12) = *(short *)(p + 4);
    func_800C3B04(D_800F3430);
}
