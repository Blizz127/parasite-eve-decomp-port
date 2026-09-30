extern void func_8008F178(unsigned char *a0, int a1);

void func_80089F58(unsigned char *a0, int a1)
{
    *(short *)(a0 + 0x6C) = 0x6E00;
    *(int *)(a0 + 0x00) = a1;
    *(short *)(a0 + 0xDE) = 0;
    *(short *)(a0 + 0xE0) = 0;
    *(short *)(a0 + 0x82) = 0;
    *(int *)(a0 + 0x34) = 0;
    *(short *)(a0 + 0xE4) = 0;
    *(short *)(a0 + 0x7A) = 0;
    *(short *)(a0 + 0xD2) = 0;
    *(short *)(a0 + 0xD0) = 0;
    *(int *)(a0 + 0x44) = 0x32000000;
    *(short *)(a0 + 0x72) = 0;
    *(short *)(a0 + 0xCE) = 0;
    *(int *)(a0 + 0x38) = 0;
    *(short *)(a0 + 0xEC) = 0;
    *(short *)(a0 + 0x84) = 0;
    *(short *)(a0 + 0xB4) = 0;
    *(short *)(a0 + 0xA6) = 0;
    *(short *)(a0 + 0x94) = 0;
    *(short *)(a0 + 0xB6) = 0;
    *(short *)(a0 + 0xA8) = 0;
    *(short *)(a0 + 0x96) = 0;
    *(short *)(a0 + 0xBC) = 0;
    *(short *)(a0 + 0xBA) = 0;
    func_8008F178(a0, 0);
}
