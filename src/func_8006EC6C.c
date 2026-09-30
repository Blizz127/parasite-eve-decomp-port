int func_8006EC6C(void *base, short index) {
    unsigned char *bytes = base;
    return (int)(bytes + *(int *)(bytes + index * 4));
}
