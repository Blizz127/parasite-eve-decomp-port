short *func_800CE610(int *pool) {
    register int *b1 asm("$2");
    register int *b2 asm("$8");
    register short *cur asm("$4");
    register int count asm("$5");
    register int limit asm("$6");
    register int stride asm("$7");
    register int i asm("$3");
    short *saved;
    char pad[8];

    b1 = pool;
    asm volatile("" : "=r"(b1) : "0"(b1));
    b2 = pool;
    asm volatile("" : "=r"(b2) : "0"(b2));
    cur = (short *)((char *)pool + 12);
    count = b1[1];
    stride = b1[0];
    i = 0;
    if (count <= 0) {
        return 0;
    }
    limit = count;
    for (;;) {
        short v;

        v = *cur;
        saved = cur;
        if (v == 0) {
            break;
        }
        i++;
        cur = (short *)((char *)cur + stride);
        if (i < limit) {
            continue;
        }
        break;
    }
    {
        register int cmp asm("$2");

        cmp = i < b2[1];
        if (cmp == 0) {
            return 0;
        }
    }
    *saved = 1;
    saved[1] = 0;
    return (short *)((char *)cur + 4);
}
