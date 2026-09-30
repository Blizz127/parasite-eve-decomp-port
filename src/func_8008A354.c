extern unsigned char *D_8009D2C8;
extern unsigned char D_8009B8F4[];

void func_8008A354(int a0, unsigned char *a1)
{
    unsigned int i;

    if (a0 == 0) {
        if (*(int *)(D_8009D2C8 + 4) != 0) {
            goto body;
        }
        return;
    }
    if (a0 != *(unsigned short *)(D_8009D2C8 + 0x54)) {
        return;
    }
body:
    *(int *)(D_8009D2C8 + 0x18) = 0xFFFFFF;
    i = 0;
    do {
        i++;
        *(short *)(a1 + 0x56) = 3;
        *(short *)(a1 + 0x58) = 1;
        *(unsigned char **)a1 = D_8009B8F4;
        *(int *)(a1 + 0xF4) |= 0x4400;
        *(short *)(a1 + 0x116) = 5;
        a1 += 0x11C;
    } while (i < 0x18);
}
