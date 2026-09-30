/* VRAM 0x8002FF78 / file 0x20778 / size 0x194. */
extern unsigned char **D_8009D254;
extern short D_800942EC;

void func_8002FF78(unsigned char id, int v) {
    unsigned char *p = *D_8009D254;

    switch (id) {
    case 0:
        *(int *)(p + 0x0) = v;
        break;
    case 1:
        *(short *)(p + 0x4) = v;
        break;
    case 2:
        *(short *)(p + 0x6) = v;
        break;
    case 3:
        *(int *)(p + 0x8) = v;
        break;
    case 4:
        *(short *)(p + 0xC) = v;
        break;
    case 5:
        *(short *)(p + 0xE) = v;
        break;
    case 6:
        *(short *)(p + 0x10) = v;
        break;
    case 10:
        *(short *)(p + 0x1C) = v;
        break;
    case 11:
        *(short *)(p + 0x1E) = v;
        break;
    case 12:
        *(short *)(p + 0x20) = v;
        break;
    case 14:
        *(short *)(p + 0x22) = v;
        break;
    case 18:
        *(short *)(p + 0x26) = v;
        break;
    case 30:
        *(short *)(p + 0x50) = v;
        break;
    case 31:
        *(unsigned char *)(p + 0x56) = v;
        break;
    case 32:
        *(unsigned char *)(p + 0x57) = v;
        break;
    case 33:
        *(short *)(p + 0x58) = v;
        break;
    case 34:
        *(unsigned char *)(p + 0x5E) = v;
        break;
    case 255:
        D_800942EC = v;
        break;
    }
}
