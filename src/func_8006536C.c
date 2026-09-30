extern int D_800A3180[];
extern unsigned char D_8009CDB4;

void func_8006536C(void)
{
    register unsigned int i asm("$6");
    register unsigned int j asm("$4");
    register unsigned char *base asm("$7");
    register unsigned int row asm("$5");
    register unsigned int off asm("$3");

    i = 0;
    base = (unsigned char *)D_800A3180;
    row = 0;
    do {
        j = 0;
        off = row;
        do {
            *(int *)(off + (unsigned int)base) = 0;
            j++;
            off += 4;
        } while (j < 3);
        i++;
        row += 12;
    } while (i < 28);
    D_8009CDB4 = 0;
}
