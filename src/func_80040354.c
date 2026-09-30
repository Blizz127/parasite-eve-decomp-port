unsigned short func_80040354(unsigned short n, unsigned char *p) {
    int crc;
    unsigned short i;
    unsigned short j;

    crc = 0xFFFF;
    for (i = 0; i < n; i++) {
        crc ^= p[i] << 8;
        for (j = 0; j < 8; j++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ 0x1021;
            } else {
                crc = crc << 1;
            }
        }
    }
    return ~crc;
}
