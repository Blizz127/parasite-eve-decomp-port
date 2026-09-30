extern unsigned char D_800B8628[];
extern int D_8009D2F4;

void func_8008CB08(unsigned char **a0) {
    *a0 = D_800B8628;
    *a0 = *a0 + D_8009D2F4 * 0x24;
    D_8009D2F4 = D_8009D2F4 + 1;
}
