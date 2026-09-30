void func_80030220(unsigned char **a0, int a1, int a2) {
    unsigned char *p;

    p = *a0;
    switch (a1 & 0xFF) {
    case 0x28:
        *(short *)(p + 0xE) = a2;
        break;
    case 0x29:
        p[0x4] = a2;
        break;
    case 0x2A:
        p[0x5] = a2;
        break;
    case 0x2C:
        *(int *)(p + 0x10) = a2;
        break;
    case 0x2D:
        *(int *)(p + 0x14) = a2;
        break;
    case 0x2B:
        *(short *)(p + 0xC) = a2;
        break;
    case 0x2E:
        p[0x7] = a2;
        break;
    case 0x2F:
        p[0x6] = a2;
        break;
    case 0x3C:
        *(int *)(p + 0x88) = a2;
        break;
    case 0x3D:
        *(short *)(p + 0x8C) = a2;
        break;
    case 0x3E:
        *(short *)(p + 0x8E) = a2;
        break;
    case 0x3F:
        p[0x90] = a2;
        break;
    case 0x40:
        p[0x91] = a2;
        break;
    case 0x41:
        p[0x92] = a2;
        break;
    case 0x42:
        p[0x93] = a2;
        break;
    case 0x43:
        p[0x94] = a2;
        break;
    case 0x44:
        p[0x95] = a2;
        break;
    case 0x45:
        *(short *)(p + 0x96) = a2;
        break;
    case 0x4A:
        *(int *)p = (*(int *)p & ~0x10) | ((a2 & 1) << 4);
        break;
    case 0x50:
        p[0xA4] = a2;
        switch (a2) {
        case 1:
            *(short *)(p + 0xA6) = 0x190;
            break;
        case 2:
            *(short *)(p + 0xA6) = 0x46;
            break;
        case 3:
            *(short *)(p + 0xA6) = 0x14;
            break;
        }
        break;
    case 0x51:
        *(int *)p = (*(int *)p & ~0x100000) | ((a2 & 1) << 20);
        break;
    case 0x5A:
        *(short *)(p + 0x98) = a2;
        break;
    case 0x5B:
        *(short *)(p + 0x9A) = a2;
        break;
    case 0x5C:
        p[0x9E] = a2;
        break;
    case 0x5D:
        p[0x9F] = a2;
        break;
    case 0x5E:
        p[0xAE] = a2;
        break;
    case 0x5F:
        p[0xAF] = a2;
        break;
    case 0x60:
        p[0xBC] = a2;
        break;
    case 0x32:
        *(short *)(p + 0xB0) = a2;
        break;
    case 0x33:
        *(short *)(p + 0xB2) = a2;
        break;
    case 0x34:
        *(short *)(p + 0xB4) = a2;
        break;
    case 0x35:
        *(short *)(p + 0xB6) = a2;
        break;
    case 0x36:
        *(short *)(p + 0xB8) = a2;
        break;
    case 0x37:
        *(short *)(p + 0xBA) = a2;
        break;
    case 0x64:
        *(int *)(p + 0xCC) = (*(int *)(p + 0xCC) & ~0x3) | ((a2 & 0x3) << 0);
        break;
    case 0x65:
        *(int *)(p + 0xCC) = (*(int *)(p + 0xCC) & ~0xC) | ((a2 & 0x3) << 2);
        break;
    case 0x66:
        *(int *)(p + 0xCC) = (*(int *)(p + 0xCC) & ~0x30) | ((a2 & 0x3) << 4);
        break;
    case 0x67:
        *(int *)(p + 0xCC) = (*(int *)(p + 0xCC) & ~0xC0) | ((a2 & 0x3) << 6);
        break;
    case 0x68:
        *(int *)(p + 0xCC) = (*(int *)(p + 0xCC) & ~0x300) | ((a2 & 0x3) << 8);
        break;
    case 0x69:
        *(int *)(p + 0xCC) = (*(int *)(p + 0xCC) & ~0xC00) | ((a2 & 0x3) << 10);
        break;
    case 0x6A:
        *(int *)(p + 0xCC) = (*(int *)(p + 0xCC) & ~0x3000) | ((a2 & 0x3) << 12);
        break;
    case 0x6B:
        *(int *)(p + 0xCC) = (*(int *)(p + 0xCC) & ~0xC000) | ((a2 & 0x3) << 14);
        break;
    case 0x6C:
        *(int *)(p + 0xCC) = (*(int *)(p + 0xCC) & ~0x30000) | ((a2 & 0x3) << 16);
        break;
    case 0x6D:
        *(int *)(p + 0xCC) = (*(int *)(p + 0xCC) & ~0x40000) | ((a2 & 0x1) << 18);
        break;
    case 0x6E:
        *(int *)(p + 0xCC) = (*(int *)(p + 0xCC) & ~0xF80000) | ((a2 & 0x1F) << 19);
        break;
    case 0x78:
        *(short *)(p + 0xD0) = a2;
        break;
    case 0x79:
        p[0xD6] = a2;
        break;
    case 0x7A:
        p[0xD7] = a2;
        break;
    case 0x7B:
        *(short *)(p + 0xA0) = a2;
        break;
    case 0x7C:
        *(short *)(p + 0xA2) = a2;
        break;
    }
}
