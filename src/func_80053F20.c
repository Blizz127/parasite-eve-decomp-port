extern int D_8009D050;
extern int func_8005415C(int);

int func_80053F20(int a0)
{
    int unused[2];
    int i;
    int count;

    i = 0;
    count = 0;
    if (D_8009D050 > 0) {
        do {
            if (func_8005415C(i) == a0) {
                count++;
            }
            i++;
        } while (i < D_8009D050);
    }
    return count;
}
