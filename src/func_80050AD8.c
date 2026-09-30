extern unsigned char *D_8009CF20;
extern int D_8009CF18;
extern void func_8005EB64(int);

void func_80050AD8(int a0)
{
    unsigned char *base;
    unsigned char *q;
    int v;

    base = D_8009CF20;
    if (a0 < base[0x14]) {
        q = base + a0;
        v = q[0x15] & 0x1F;
        if (v == 0) {
            return;
        }
        if (D_8009CF18 != 0) {
            v += 0x22;
        } else {
            v += 0x36;
        }
    } else {
        v = 0x46;
    }
    func_8005EB64(v);
}
