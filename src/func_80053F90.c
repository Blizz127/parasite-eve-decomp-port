extern short D_800C1EB8[];
extern unsigned char *func_8005DB44(int);

int func_80053F90(int a0)
{
    short *p;
    int i;
    int count;
    int v;

    count = 0;
    i = 0;
    p = D_800C1EB8;
    do {
        v = *p;
        if (v != 0) {
            if (func_8005DB44(v - 1)[6] == a0) {
                count++;
            }
        }
        i++;
        p++;
    } while (i < 0x64);
    return count;
}
