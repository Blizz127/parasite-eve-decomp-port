extern int D_8009D218;
extern unsigned char D_800C0DE0[];

unsigned char *func_8005BEE8(void) {
    int select = D_8009D218;
    unsigned char *result = D_800C0DE0;

    if (select != 0) {
        result += 0x10;
    }
    return result;
}
