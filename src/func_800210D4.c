extern unsigned char D_8009CE3C;
extern unsigned int D_8009D1A0;
extern unsigned int *D_8009D278;

int func_800210D4(void) {
    if (D_8009CE3C == 0) {
        if ((D_8009D1A0 & 2) == 0 || (D_8009D278[0x13] & 0x10000) == 0) {
            return 1;
        }
    }
    return 0;
}
