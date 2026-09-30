extern short D_800E2818[];
extern void func_800C2EAC();
extern void func_800C3098();
extern void func_800C2FF0();
extern void func_800C3238();
extern void func_800C3B04();

void func_800CC7AC(int a0, int a1, unsigned char *a2)
{
    unsigned int i;
    unsigned char c;

    func_800C2EAC(3);
    func_800C3098(0x10);
    func_800C2FF0(0x10, 0x10);
    func_800C3238(0);
    c = a2[3];
    D_800E2818[0x14] = (short)(c << 8) >> 7;
    for (i = 0; i < *(short *)(a2 + 4); i++) {
        D_800E2818[0] = *(unsigned short *)(a2 + i * 2 + 8);
        D_800E2818[1] = *(unsigned short *)(a2 + i * 2 + 0x28);
        D_800E2818[2] = *(unsigned short *)(a2 + i * 2 + 0x48);
        func_800C3B04(D_800E2818);
    }
}
