extern unsigned char *D_800E2800;
extern short D_800E21A4;
extern void func_800E026C(unsigned char *a0);
extern void func_800E03A0(unsigned char *a0);

void func_800E01BC(void)
{
    unsigned char *p;
    int i;

    p = D_800E2800;
    i = 0;
    while (i < D_800E21A4) {
        if (p[0] != 0) {
            switch (p[0xA]) {
            case 0:
                func_800E026C(p);
                break;
            case 1:
                func_800E03A0(p);
                break;
            }
            i++;
        }
        p -= 0x14;
    }
}
