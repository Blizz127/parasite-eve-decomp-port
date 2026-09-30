void func_8003E0A4(unsigned char *a0, int a1, int a2) {
    register int v asm("$2") = 2;
    if ((*(unsigned char **)a0)[2] == v) v = 3; else v = 1;
    *(int *)(a0 + 0x24) = a1;
    *(short *)(a0 + 0x28) = v;
    *(short *)(a0 + 0x2A) = a2;
}
