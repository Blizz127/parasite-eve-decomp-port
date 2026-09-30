extern void func_8005E8C4(void);
extern void func_8005EB64(int);
extern void func_8005E8A4(int, int);
extern void func_8005F27C(unsigned char *);
extern void func_8005FB74(int);
extern void func_8005E914(void);

void func_800534E4(unsigned char *record, unsigned char *name)
{
    unsigned int kind;
    int maximum;
    unsigned int current;
    int arg;
    int d;
    int lo;
    int hi;

    func_8005E8C4();
    if (record[6] < 8) {
        lo = record[9];
        hi = *(short *)(record + 0x12);
        maximum = lo + hi;
        asm volatile("" : "=r"(lo), "=r"(hi) : "0"(lo), "1"(hi));
        current = *(unsigned short *)(record + 0xA);
        if (maximum < 1000) {
            if (current == maximum) {
                func_8005EB64(0x69);
            }
        } else {
            if (current == 999) {
                func_8005EB64(0x69);
            }
        }
    }
    func_8005EB64(record[0] - 1);
    func_8005E8A4(0x12, 0);
    if (name != 0) {
        func_8005F27C(name);
    }
    kind = record[6];
    if (kind == 0) {
        goto high;
    }
    if (kind >= 8) {
        goto high;
    }
    if ((int)kind - 4 <= 0) {
        goto call;
    }
    if (kind != 4) {
        goto call;
    }
    goto end;
high:
    if (kind < 0x13) {
        goto end;
    }
    d = (int)kind - 0x12;
    if (d == 0) {
        goto end;
    }
call:
    func_8005E8A4(0x56, 0);
    kind = record[6];
    if (kind != 0 && kind < 8) {
        if ((int)kind - 4 > 0) {
            arg = kind + 0x1A;
        } else {
            arg = 0x1F;
        }
    } else if (kind >= 0x13) {
        arg = kind + 0xC;
    } else {
        arg = 0x1E;
    }
    func_8005EB64(arg);
    func_8005E8A4(0, 5);
    func_8005FB74(*(unsigned short *)(record + 0xA));
end:
    func_8005E914();
}
