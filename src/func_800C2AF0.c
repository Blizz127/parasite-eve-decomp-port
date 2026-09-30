extern unsigned int *D_800E2248;

int func_800C2AF0(unsigned int *base, int unused, int index,
                  unsigned int value) {
    base += 3;
    D_800E2248 = base;
    base[index + 18] = value;
    return 0;
}
