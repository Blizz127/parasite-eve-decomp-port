union Sq {
    long long ll;
    struct {
        unsigned int lo;
        unsigned int hi;
    } w;
};

int func_80070FAC(int x, int y, int z)
{
    union Sq a;
    union Sq b;
    union Sq c;
    register unsigned int lo0 asm("$7");
    register unsigned int hi0 asm("$8");
    register unsigned int lo1 asm("$9");
    register unsigned int hi1 asm("$10");
    register unsigned int lo2 asm("$4");
    register unsigned int hi2 asm("$5");
    register unsigned int lo asm("$2");
    register unsigned int carry asm("$3");
    unsigned int hi;
    register unsigned int sum asm("$6");
    unsigned int carry2;

    if (x < 0) {
        x = -x;
    }
    if (y < 0) {
        y = -y;
    }
    if (z < 0) {
        z = -z;
    }
    asm volatile("" : "=r"(x), "=r"(y), "=r"(z) : "0"(x), "1"(y), "2"(z));
    a.ll = (long long)x * x;
    lo0 = a.w.lo;
    hi0 = a.w.hi;
    asm volatile("" : "=r"(lo0), "=r"(hi0) : "0"(lo0), "1"(hi0));
    b.ll = (long long)y * y;
    lo1 = b.w.lo;
    hi1 = b.w.hi;
    asm volatile("" : "=r"(lo1), "=r"(hi1) : "0"(lo1), "1"(hi1));
    c.ll = (long long)z * z;
    lo2 = c.w.lo;
    hi2 = c.w.hi;
    lo = lo0 + lo1;
    asm volatile("" : "=r"(lo1) : "0"(lo1));
    carry = lo < lo0;
    carry = carry + hi0;
    carry = carry + hi1;
    asm volatile("" : "=r"(hi1) : "0"(hi1));
    sum = lo + lo2;
    asm volatile("" : "=r"(lo0), "=r"(hi0), "=r"(lo1), "=r"(hi1) : "0"(lo0), "1"(hi0), "2"(lo1), "3"(hi1));
    carry2 = sum < lo;
    asm volatile("" : "=r"(lo2), "=r"(lo) : "0"(lo2), "1"(lo));
    carry2 = carry2 + carry;
    carry2 = carry2 + hi2;
    sum = sum >> 20;
    carry2 = carry2 << 12;
    return (int)(carry2 | sum);
}
