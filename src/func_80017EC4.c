extern unsigned int D_8009D2F0;

int func_80017EC4(int a0)
{
    unsigned short *ptr = *(unsigned short **)a0;
    unsigned short v1 = *ptr;
    unsigned char a0_local = *(unsigned char *)(D_8009D2F0 + 0xF);
    if (a0_local < v1) {
        v1 = a0_local;
    }
    *(unsigned int *)(D_8009D2F0 + 0x14) = v1 << 16;
    return 1;
}