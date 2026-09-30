extern unsigned int D_8009D270;
extern unsigned int D_8009CDE8;
extern unsigned int D_8009D2BC;
extern unsigned int D_8009D2E4;
extern unsigned char *D_8009D2C8;
extern unsigned int D_800B2900[];
extern int func_80085084(void);
extern int func_80085EB4(int a0);
extern void func_800850F4(unsigned char *a0, unsigned int a1);
extern void func_8008A02C(unsigned int *a0, unsigned int a1, unsigned int a2);

int func_800871AC(unsigned char *a0, unsigned int a1)
{
    unsigned int size;
    unsigned int n;
    unsigned int chunk;
    unsigned int *q;
    unsigned int *d;
    unsigned int v;
    unsigned int i;
    unsigned int c;

    if (D_8009D270 & 1) {
        if (func_80085084() != 0) {
            return -1;
        }
        a0 += 0x14;
        size = *(unsigned int *)a0;
        a0 += 4;
        v = *(unsigned int *)a0;
        a0 += 4;
        n = *(unsigned int *)a0;
        if (n == 0) {
            n = 0x100;
        }
        n -= v;
        a0 += 0x24;
        q = (unsigned int *)a0;
        chunk = n << 6;
        a0 += chunk;
        {
            unsigned int t = a1 - 0x40;
            asm("" : "=r"(t) : "0"(t));
            a1 = t - chunk;
        }
        if (a1 >= size) {
            chunk = size;
        } else {
            chunk = a1;
        }
        if (n < 0x31) {
            if (*(unsigned int *)(D_8009D2C8 + 0x6C) != 0) {
                if (!(*(unsigned int *)(D_8009D2C8 + 0x68) & 0x100)) {
                    a1 = 0x38000;
                    D_8009CDE8 |= 0x100;
                    goto done;
                }
            } else if (*(unsigned int *)(D_8009D2C8 + 4) != 0) {
                if (!(*(unsigned int *)D_8009D2C8 & 0x100)) {
                    a1 = 0x38000;
                    D_8009CDE8 |= 0x100;
                    goto done;
                }
            }
        }
        a1 = 0x8000;
        D_8009CDE8 &= ~0x100;
done:
        func_80085EB4(a1);
        func_800850F4(a0, chunk);
        D_8009D2BC = a1 + chunk;
        D_8009D2E4 = size - chunk;
        func_8008A02C(q, a1, n);
        i = 0x50;
        if (a1 == 0x8000) {
            i = 0x20;
        }
        d = (unsigned int *)((unsigned char *)D_800B2900 + (i << 6));
        chunk = n << 4;
    loop:
        chunk--;
        *d++ = *q++;
        if (chunk != 0) {
            goto loop;
        }
        D_8009D270 &= ~1;
        return D_8009D2E4;
    }
    func_80085EB4(D_8009D2BC);
    c = a1;
    if (c >= D_8009D2E4) {
        c = D_8009D2E4;
    }
    func_800850F4(a0, c);
    D_8009D2BC += c;
    D_8009D2E4 -= c;
    return D_8009D2E4;
}
