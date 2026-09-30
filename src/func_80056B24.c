extern unsigned char D_800A1F94[];
extern unsigned char D_800A1FB4[];

int func_80056B24(int n)
{
    unsigned char *src;
    unsigned char *dst;
    register int ret asm("$10");
    int rem;
    int got;
    int b;
    int e;
    int t;
    int v;

    ret = 0;
    if (n > 0) {
        src = D_800A1F94;
        dst = D_800A1FB4;
    } else {
        src = D_800A1FB4;
        dst = D_800A1F94;
        n = -n;
    }
    if (src == 0 || dst == 0) {
        goto ret3;
    }
    rem = *(unsigned short *)(src + 0xA) - n;
    got = *(unsigned short *)(dst + 0xA) + n;
    if (rem < 0) {
        got = got + rem;
        rem = 0;
        ret = 1;
    }
    b = dst[9];
    e = *(short *)(dst + 0x12);
    t = b + e;
    if (t >= 0x3E8) {
        t = 0x3E7;
    }
    if (t < got) {
        t = b + e;
        if (t >= 0x3E8) {
            t = 0x3E7;
        }
        rem = rem + (got - t);
        got = b + e;
        if (got >= 0x3E8) {
            got = 0x3E7;
        }
        ret = 2;
    }
    *(short *)(src + 0xA) = rem;
    *(short *)(dst + 0xA) = got;
    if (rem == 0) {
        v = *(unsigned short *)(src + 0xC);
        if (v != 0) {
            *(short *)(src + 0xA) = v;
            *(short *)(src + 0xC) = 0;
        }
    }
    goto done;
ret3:
    ret = 3;
done:
    return ret;
}
