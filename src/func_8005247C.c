extern void func_8005218C(void);
extern unsigned short **D_8009D254;

void func_8005247C(void)
{
    unsigned short **p;
    unsigned short *q;
    unsigned short v;

    func_8005218C();
    p = D_8009D254;
    if (p != 0) {
        q = *p;
        if (q != 0) {
            v = q[14];
            q[7] = v;
            q[6] = v;
        }
    }
}
