extern unsigned char D_80092888[];

unsigned char *func_8005DAB4(unsigned int index) {
    if (index >= 0x41) {
        return 0;
    }
    return D_80092888 + index * 32;
}
