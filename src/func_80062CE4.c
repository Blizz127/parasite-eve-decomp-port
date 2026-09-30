extern void **D_8009D154;
extern void **D_8009D15C;
extern void **D_8009D160;

void func_80062CE4(void)
{
    void **p;

    p = D_8009D154;
    if (p != 0) {
        while (p != 0) {
            if (p == D_8009D160) {
                break;
            }
            p = (void **)*p;
        }
        if (p != 0) {
            D_8009D15C = p;
        }
    }
    D_8009D160 = 0;
}
