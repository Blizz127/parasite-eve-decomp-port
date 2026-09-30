void func_800D3AFC(unsigned char *dst, int n, unsigned char *src, int flag)
{
    int pad[4];
    register unsigned char *saved asm("$8");
    int i;

    (void)pad;
    saved = dst;
    if (flag != 0) {
        register int lim asm("$3");
        register unsigned char *p asm("$4");

        {
            register int cv asm("$2");

            cv = (short)(n + 1);
            if (cv <= 0) {
                goto done;
            }
            i = 0;
            lim = cv;
        }
        p = dst + 4;
        do {
            *(unsigned short *)saved = *(unsigned short *)src;
            *(unsigned short *)(p - 2) = *(unsigned short *)(src + 2);
            *(unsigned short *)p = *(unsigned short *)(src + 4);
            i++;
            saved += 8;
            p += 8;
        } while (i < lim);
    } else {
        register int cv asm("$3");
        register unsigned char *end asm("$5");
        register int lim asm("$9");
        register unsigned char *p asm("$4");

        {
            register int hi asm("$2");

            hi = n << 16;
            cv = hi >> 16;
        }
        {
            register int off asm("$2");

            off = (cv << 3) - 8;
            end = saved + off;
        }
        i = 0;
        if (cv <= 0) {
            goto store;
        }
        lim = cv;
        p = end + 0xC;
        do {
            int w0;
            int w1;

            i++;
            w0 = *(int *)end;
            w1 = *(int *)(p - 8);
            end -= 8;
            *(int *)(p - 4) = w0;
            *(int *)p = w1;
            p -= 8;
        } while (i < lim);
    store:
        *(int *)saved = *(int *)src;
        *(int *)(saved + 4) = *(int *)(src + 4);
    }
done:
    ;
}
