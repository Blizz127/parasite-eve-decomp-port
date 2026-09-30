extern unsigned int *D_8009D058;

void func_80055FB4(int index) {
    D_8009D058[index >> 5] |= 1u << (index & 0x1F);
}
