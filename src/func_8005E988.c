extern unsigned int D_8009D110;
extern unsigned int D_8009D114;

void func_8005E988(int a0, int a1)
{
    unsigned int c;

    if (a0 >= a1) {
        if (a1 < a0) {
            c = 0x404080;
        } else {
            c = 0x808080;
        }
    } else {
        c = 0x408080;
    }
    D_8009D110 = c;
    D_8009D114 = c >> 1;
}
