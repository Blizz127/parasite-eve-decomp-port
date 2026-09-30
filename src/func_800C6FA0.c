void func_800C6FA0(unsigned char *record, unsigned short scale)
{
    unsigned char *c;
    int i;
    unsigned int *s;

    s = (unsigned int *)0x1F800000;
    c = record + *(unsigned short *)(record + 8);
    for (i = 0; i < *(unsigned short *)(record + 0xA); i++, c += 4) {
        s[12] = c[0] * scale;
        if (s[12] > 0x7FFF) {
            s[12] = 0x7FFF;
        }
        s[12] >>= 7;
        c[0] = s[12];
        s[12] = c[1] * scale;
        if (s[12] > 0x7FFF) {
            s[12] = 0x7FFF;
        }
        s[12] >>= 7;
        c[1] = s[12];
        s[12] = c[2] * scale;
        if (s[12] > 0x7FFF) {
            s[12] = 0x7FFF;
        }
        s[12] >>= 7;
        c[2] = s[12];
    }
}
