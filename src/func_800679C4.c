/* Matching C leaf (LINK_EXACT at -O2 -G0). Closed by band1: a bare
   __asm__ __volatile__("") between the two loads is a hard sched2 barrier,
   which frees the add for the bnez delay slot. */
extern short *D_800B1624;

int func_800679C4(int dx, int dy, int dz)
{
    char pad[32];
    register short *g asm("$7");
    register int nx asm("$9");
    register int ny asm("$8");
    register int na asm("$4");
    register int x asm("$3");
    register int b asm("$5");
    register int t asm("$2");
    register int sh asm("$3");

    g = D_800B1624;
    x = *(unsigned short *)((char *)g + 0x28) + dx;
    nx = x;
    t = *(unsigned short *)((char *)g + 0x2A);
    ny = t + dy;
    __asm__ __volatile__("" : "=r"(x) : "0"(x));
    x = (short)x;
    na = *(unsigned short *)((char *)g + 0x24);
    __asm__ __volatile__("");
    t = *(short *)((char *)g + 0x30);
    b = t;
    __asm__ __volatile__("" : "=r"(b) : "0"(b));
    na = na + dz;
    if (x < t) {
        goto set1;
    }
    t = *(short *)((char *)g + 0x32);
    b = t;
    __asm__ __volatile__("" : "=r"(b) : "0"(b));
    if (!(t < x)) {
        sh = ny << 16;
        goto done1;
    }
set1:
    nx = b;
    sh = ny << 16;
done1:
    x = sh >> 16;
    t = *(short *)((char *)g + 0x34);
    b = t;
    __asm__ __volatile__("" : "=r"(b) : "0"(b));
    if (x < t) {
        goto set2;
    }
    t = *(short *)((char *)g + 0x36);
    b = t;
    __asm__ __volatile__("" : "=r"(b) : "0"(b));
    if (!(t < x)) {
        goto done2;
    }
set2:
    ny = b;
done2:
    *(short *)((char *)g + 0x2C) = nx;
    *(short *)((char *)g + 0x2E) = ny;
    *(short *)((char *)g + 0x26) = na;
    return 0;
}
