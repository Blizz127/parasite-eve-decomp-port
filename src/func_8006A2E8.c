extern unsigned short D_800BCE9E;
extern unsigned short D_800BCE8A;
extern unsigned char D_800B0DB1;
int func_8006A2E8(int a0, unsigned int a1) {
    if (a1 < 0x10) {
        unsigned short v = a1;
        D_800BCE9E = v;
        D_800BCE8A = v;
        D_800B0DB1 = v;
    }
    return 0;
}
