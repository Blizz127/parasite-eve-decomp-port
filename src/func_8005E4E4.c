extern int D_8009D0E8;
extern unsigned int func_8005E038(void);
int func_8005E4E4(void) {
    int r = 0;
    if (D_8009D0E8) {
        unsigned int v = func_8005E038() & 0x20;
        r = (v > 0);
    }
    return r;
}
