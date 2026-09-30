union Sq {
    long long ll;
    struct {
        unsigned int lo;
        unsigned int hi;
    } w;
};

int func_8003708C(int a0, int a1)
{
    union Sq p;
    register unsigned int lo asm("$2");
    register unsigned int hi asm("$3");

    p.ll = (long long)a0 * a1;
    lo = p.w.lo;
    hi = p.w.hi;
    lo = lo >> 16;
    hi = hi << 16;
    return (int)(hi | lo);
}
