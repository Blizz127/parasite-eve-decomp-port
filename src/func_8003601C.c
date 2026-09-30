extern unsigned int *D_8009D20C;

void func_8003601C(void) {
    unsigned int *p;

    p = D_8009D20C;
    while (p != 0) {
        if (p[0x63] != 0 && (p[0x26] & 0x400000)) {
            p[0xA] = ((unsigned int *)p[0x63])[0xA];
            p[0xB] = ((unsigned int *)p[0x63])[0xB];
            p[0xC] = ((unsigned int *)p[0x63])[0xC];
            ((unsigned short *)p)[0x1C] = ((unsigned short *)p[0x63])[0x1C];
            ((unsigned short *)p)[0x1D] = ((unsigned short *)p[0x63])[0x1D];
            ((unsigned short *)p)[0x1E] = ((unsigned short *)p[0x63])[0x1E];
        }
        p = (unsigned int *)p[1];
    }
}
