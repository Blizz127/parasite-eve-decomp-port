extern char *D_800E2248;

void *func_800C2B10(int arg0) {
    register char *base asm("$2");

    arg0 *= 4;
    asm volatile("" : : : "memory");
    base = D_800E2248;
    arg0 += 8;
    return base + arg0;
}
