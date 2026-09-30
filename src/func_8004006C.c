/* VRAM 0x8004006C / file 0x3086C / size 0x1A4. */
extern int D_800A1708[];

void func_8004006C(unsigned char *dst, unsigned char *fmt) {
    int *arg;
    int v;
    int t;
    unsigned int w;
    unsigned char *s;

    arg = D_800A1708;
    while (*fmt != 0) {
        if (*fmt++ == '%') {
            switch (*fmt++) {
            case 'd':
                v = *arg++;
                t = v / 10;
                w = t % 10 + 0x824F;
                *dst++ = w >> 8;
                *dst++ = w;
                w = v % 10 + 0x824F;
                *dst++ = w >> 8;
                *dst++ = w;
                break;
            case 'D':
                v = *arg++;
                w = v % 10 + 0x824F;
                *dst++ = w >> 8;
                *dst++ = w;
                break;
            case 's':
                s = (unsigned char *)*arg++;
                if (s != 0) {
                    while (*s != 0) {
                        *dst++ = *s++;
                    }
                }
                break;
            }
        } else {
            *dst++ = fmt[-1];
        }
    }
    *dst = 0;
}
