extern unsigned char D_80092478[];

unsigned char *func_8005DA8C(unsigned int index) {
    if (index >= 0x41) {
        return 0;
    }
    return D_80092478 + index * 16;
}
