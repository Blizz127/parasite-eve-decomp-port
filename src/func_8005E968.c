extern int D_8009D110;
extern int D_8009D114;

void func_8005E968(int a0) {
    D_8009D110 = a0;
    D_8009D114 = (a0 >> 1) & 0x7F7F7F;
}
