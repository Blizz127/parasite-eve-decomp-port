extern short D_800E2260[];
extern void func_800C2EAC();
extern void func_800C3098();
extern void func_800C2FF0();
extern void func_800C3238();
extern void func_800C3B04();

void func_800CC55C(int a0, int a1, unsigned char *a2)
{
    unsigned int i;

    func_800C2EAC(3);
    func_800C3098(0x10);
    func_800C2FF0(0x20, 0x20);
    func_800C3238(2);
    *(int *)(D_800E2260 + 8) = *(short *)(a2 + 6);
    *(int *)(D_800E2260 + 10) = *(short *)(a2 + 6);
    *(int *)(D_800E2260 + 12) = *(short *)(a2 + 6);
    D_800E2260[0x14] = a2[1];
    for (i = 0; i < *(signed char *)(a2 + 2); i++) {
        D_800E2260[0] = *(unsigned short *)(a2 + i * 8 + 0x26);
        D_800E2260[1] = *(unsigned short *)(a2 + i * 8 + 0x28);
        D_800E2260[2] = *(unsigned short *)(a2 + i * 8 + 0x2A);
        func_800C3B04(D_800E2260);
    }
}
