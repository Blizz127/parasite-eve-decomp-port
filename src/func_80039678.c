typedef struct Font {
    unsigned char pad0[3];
    unsigned char cur;
    unsigned char pad4[8];
    unsigned char *data;
} Font;

extern Font D_80091A1C;
extern int func_80039310(void);
extern int func_80039184(void);

static inline unsigned char cur_attr(void)
{
    unsigned char *d = D_80091A1C.data;

    return (d + (d + D_80091A1C.cur)[29])[4];
}

unsigned char func_80039678(unsigned char key)
{
    unsigned char c;
    unsigned char *p;
    int i;
    unsigned char v;

    c = cur_attr();
    __asm__("" : "=r"(c) : "0"(c));
    if ((c < 2 || c == 24 || c == 31 || c == 38 || c == 45 || c == 52 || c == 59) && key == 20) {
        return func_80039310();
    }
    if (c == 2 && key == 22) {
        return func_80039184();
    }
    p = D_80091A1C.data + 28;
    switch (key) {
    case 20:
        v = (p + D_80091A1C.cur)[101];
        for (i = D_80091A1C.cur - 1; i >= 0; i--) {
            if ((p + i)[101] == v) {
                D_80091A1C.cur = i;
                i = 0;
            }
        }
        break;
    case 21:
        D_80091A1C.cur--;
        break;
    case 22:
        v = (p + D_80091A1C.cur)[101];
        for (i = D_80091A1C.cur + 1; i < p[0]; i++) {
            if ((p + i)[101] == v) {
                D_80091A1C.cur = i;
                i = p[0];
            }
        }
        break;
    case 23:
        D_80091A1C.cur++;
        break;
    }
    return cur_attr();
}
