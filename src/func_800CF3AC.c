typedef struct {
    unsigned char b0;
    unsigned char b1;
    unsigned char b2;
    unsigned char b3;
    unsigned short f4;
    unsigned short f6;
} EntCF3;

typedef struct {
    int total;
    int count;
    EntCF3 e[1];
} HdrCF3;

extern void func_80078554(EntCF3 *a, EntCF3 *b, int wa, int wb, int *out);

void func_800CF3AC(HdrCF3 *h, int *out, int t)
{
    int pad[2];
    register HdrCF3 *hh asm("$9");
    register int count asm("$8");
    int i;
    int frac;

    (void)pad;
    hh = h;
    count = h->count;
    asm volatile("" : "=r"(hh), "=r"(count) : "0"(hh), "1"(count));
    {
        register int total asm("$3");
        EntCF3 *e;

        h = (HdrCF3 *)((unsigned char *)h + 8);
        if (hh->total != 0) {
            goto join;
        }
        {
            register int acc asm("$3");
            register int d asm("$7");

            count = 0;
            acc = 0;
            h = (HdrCF3 *)((unsigned char *)h + 4);
            asm volatile("" : "=r"(acc) : "0"(acc));
            goto init_loop;
        init_loop:
            d = ((unsigned char *)h)[-1];
            if (d == 0) {
                goto init_done;
            }
            *(unsigned short *)((unsigned char *)h + 2) = (unsigned short)acc;
            acc += d;
            count++;
            *(unsigned short *)h = (unsigned short)d;
            h = (HdrCF3 *)((unsigned char *)h + 8);
            goto init_loop;
        init_done: ;
            h = (HdrCF3 *)((unsigned char *)hh + 8);
            hh->total = acc;
            hh->count = count;
        }
    join:
        e = (EntCF3 *)h;
        total = hh->total;
        if (total < t) {
            t = total;
        }
        e = (EntCF3 *)((unsigned char *)e + ((count << 3) - 8));
        i = 0;
        if (count <= 0) {
            goto divs;
        }
    loop:
        if (t >= e->f6) {
            goto divs;
        }
        i++;
        e--;
        if (i < count) {
            goto loop;
        }
    divs:
        {
            register int den asm("$3");
            register int diff asm("$7");

            {
                register int f6 asm("$2");

                f6 = (int)e->f6;
                den = (int)e->f4;
                diff = t - f6;
            }
            {
                register int num asm("$2");

                num = diff << 12;
                frac = num / den;
            }
        }
        func_80078554(e, e + 1, 0x1000 - frac, frac, out);
    }
}
