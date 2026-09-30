extern int D_800BCD80;
extern int D_800BCD84;
extern int D_800BCD88;
extern int D_800BCD8C;
extern int D_800BCD90;
extern int D_8009D268;
extern int D_8009CDF0;
extern unsigned char *D_8009D2C8;

extern int func_80085084(int);
extern void func_8008CB54(int);
extern void func_8008CB08(int **);

#define Q(o) (*(int *)((char *)rec + (o)))
#define R2(o) (*(int *)((char *)r2 + (o)))
#define R(o) (*(int *)((char *)r + (o)))

int func_8008CBA8(void)
{
    int ret;
    unsigned int cmd;
    unsigned char *s;
    int *rec;
    int *p;
    int a;
    int n;

    ret = 0;
    cmd = D_800BCD80;
    D_8009D268 = 1;
    switch (cmd) {
    case 0x10:
    case 0x12:
    case 0x19:
        s = (unsigned char *)D_800BCD84;
        p = &D_800BCD80;
        if (func_80085084((int)s) != 0) {
            ret = -1;
            break;
        }
        s += 4;
        ret = *(unsigned short *)s;
        s += 4;
        a = *(unsigned short *)s;
        s += 8;
        if (*(unsigned short *)(D_8009D2C8 + 0x54) != ret) {
            func_8008CB54(a);
            func_8008CB08(&rec);
            Q(4) = (int)s;
            Q(12) = ret;
            if (*p == 0x12) {
                Q(16) = D_800BCD88;
            }
            Q(0) = *p;
        } else {
            ret = 0;
        }
        break;
    case 0x24:
    {
        int *r;
        func_8008CB08(&rec);
        r = rec;
        R(4) = D_800BCD84;
        R(8) = D_800BCD88;
        n = D_8009CDF0;
        R(12) = D_800BCD8C;
        D_8009CDF0 = ((n + 1) & 0x1FF) + 0x400;
        R(16) = D_800BCD90;
        R(20) = n;
        R(0) = cmd;
        ret = n;
        break;
    }
    case 0xD8:
    {
        int *r;
        int *r2;
        func_8008CB08(&rec);
        r = rec;
        R(4) = D_800BCD84;
        R(0) = 0xD0;
        func_8008CB08(&rec);
        r2 = rec;
        R2(4) = D_800BCD84;
        R2(0) = 0xD4;
        break;
    }
    case 0xD9:
    {
        int *r;
        int *r2;
        func_8008CB08(&rec);
        r = rec;
        R(4) = D_800BCD84;
        R(8) = D_800BCD88;
        R(0) = 0xD1;
        func_8008CB08(&rec);
        r2 = rec;
        R2(4) = D_800BCD84;
        R2(8) = D_800BCD88;
        R2(0) = 0xD5;
        break;
    }
    case 0xDA:
    {
        int *r;
        int *r2;
        func_8008CB08(&rec);
        r = rec;
        R(4) = D_800BCD84;
        R(8) = D_800BCD88;
        R(12) = D_800BCD8C;
        R(0) = 0xD2;
        func_8008CB08(&rec);
        r2 = rec;
        R2(4) = D_800BCD84;
        R2(8) = D_800BCD88;
        R2(12) = D_800BCD8C;
        R2(0) = 0xD6;
        break;
    }
    case 0x99:
    {
        int *r;
        int *r2;
        func_8008CB08(&rec);
        r = rec;
        R(0) = 0x9B;
        func_8008CB08(&rec);
        r2 = rec;
        R2(0) = 0x9D;
        break;
    }
    case 0x98:
    {
        int *r;
        int *r2;
        func_8008CB08(&rec);
        r = rec;
        R(0) = 0x9A;
        func_8008CB08(&rec);
        r2 = rec;
        R2(0) = 0x9C;
        break;
    }
    default:
    {
        int *r;
        func_8008CB08(&rec);
        r = rec;
        R(4) = D_800BCD84;
        R(8) = D_800BCD88;
        R(12) = D_800BCD8C;
        R(16) = D_800BCD90;
        R(0) = D_800BCD80;
        break;
    }
    }
    D_8009D268 = 0;
    return ret;
}
