extern unsigned short D_80172CA0;
extern unsigned short D_80172CA2;
void func_80077C84(unsigned char *p, int a, int b, int c);
void func_80077B04(unsigned char *p, int a);
void func_80077CB4(unsigned char *p, unsigned char *q);

void func_8012462C(unsigned char *p, short x, short y, unsigned char w, unsigned short h)
{
    unsigned char *q;

    func_80077C84(p, 0, 0, D_80172CA0);
    q = p + 8;
    p[0xB] = 4;
    p[0xF] = 0x64;
    func_80077B04(q, 1);
    func_80077CB4(p, q);
    *(short *)(p + 0x18) = 0x10;
    *(short *)(p + 0x1A) = 0x10;
    *(short *)(p + 0x10) = x;
    *(short *)(p + 0x12) = y;
    p[0x14] = w;
    p[0x15] = h;
    p[0xC] = 0xFF;
    p[0xD] = 0xFF;
    p[0xE] = 0xFF;
    *(short *)(p + 0x16) = D_80172CA2;
}
