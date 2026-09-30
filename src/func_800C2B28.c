extern char *D_800E2248;

void *func_800C2B28(int arg0) {
    register char *base asm("$2");

    arg0 *= 4;
    asm volatile("" : : : "memory");
    base = D_800E2248;
    arg0 += 0x48;
    return base + arg0;
}
