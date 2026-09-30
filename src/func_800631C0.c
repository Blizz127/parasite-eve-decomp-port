/* Null-safe flag query at VRAM 0x800631C0 / file 0x539C0. */
unsigned int func_800631C0(const unsigned char *arg0) {
    unsigned int result = 0;
    if (arg0 != 0) {
        result = *(const unsigned int *)(arg0 + 0x48) < 1;
    }
    return result;
}
