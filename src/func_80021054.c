extern unsigned int D_8009D278[];
extern signed char D_8009CE3C;
int func_80021054(void) {
    unsigned int *obj = (unsigned int *)D_8009D278[0];
    if (*(int *)((unsigned char *)obj + 0x4C) & 0x10000) return -1;
    return D_8009CE3C;
}
