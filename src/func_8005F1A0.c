extern int D_8009D0D8;
extern int D_8009CDB0;
extern int func_8005DC28(int);

int func_8005F1A0(unsigned char *p)
{
    int w;
    register int ch asm("$4");
    int v;
    unsigned char c;
    int t;

    w = 0;
    while ((c = *p) != 0xFF) {
        p++;
        ch = c;
        v = ch & 0xFF;
        if (D_8009D0D8 != 0) {
            v = v + (D_8009D0D8 << 8);
            D_8009D0D8 = 0;
        }
        if ((unsigned int)(ch & 0xFF) >= 0xFA) {
            D_8009D0D8 = (ch & 0xFF) - 0xFA;
            v = -1;
        }
        ch = v;
        if (ch >= 0) {
            t = 0;
            if (ch < 0xA || ch == 0xF) {
                t = 1;
            }
            D_8009CDB0 = t + 1;
            if (ch >= 0x100) {
                ch -= 0x13;
            }
            w += ((func_8005DC28(ch) >> 4) & 0xF) + D_8009CDB0;
        }
    }
    return w;
}
