void func_800C4E50(unsigned char *a0);

void func_8018FBE8(int a0, int a1, unsigned char *a2)
{
    register int v0 asm("$2");
    register int v1 asm("$3");
    register unsigned char *q asm("$4");

    v1 = 0x80;
    v0 = 0x10;
    *(short *)(a2 + 0x120) = (short)v0;
    v0 = 0xC8;
    *(short *)(a2 + 0x12) = (short)v1;
    *(short *)(a2 + 0x128) = (short)v1;
    v1 = 0xFF;
    *(short *)(a2 + 0x126) = (short)v0;
    v0 = 0x40;
    a2[0x118] = (unsigned char)v0;
    v0 = 0x80;
    a2[0x119] = (unsigned char)v0;
    v0 = 0x4B0;
    *(short *)(a2 + 0x122) = (short)v0;
    v0 = (int)(a2 + 0x14);
    q = a2 + 0x114;
    *(short *)(a2 + 0x10) = 0;
    a2[0x11C] = (unsigned char)v1;
    a2[0x11D] = (unsigned char)v1;
    a2[0x11E] = (unsigned char)v1;
    a2[0x11A] = (unsigned char)v1;
    *(short *)(a2 + 0x124) = 0;
    *(int *)q = v0;
    func_800C4E50(q);
}
