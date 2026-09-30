extern short *D_8009D048;
extern int D_8009D03C;
extern unsigned char D_800BEEAC[];
extern unsigned char *func_8005DC9C(int);

unsigned char *func_80053068(int a0)
{
    int id;
    unsigned char *base;
    unsigned char *rec;
    unsigned char *res;
    int off;

    id = D_8009D048[a0];
    res = 0;
    if ((unsigned int)(id - 0x100) < 0x80) {
        off = id << 5;
        base = D_800BEEAC;
        rec = base + off;
        if (rec[5] & 0x10) {
            if (rec[6] == 9) {
                res = base + 0x3208;
            } else {
                res = base + 0x31F8;
            }
        } else {
            res = func_8005DC9C(rec[4] - 1);
        }
    } else if ((unsigned int)(id - 1) < 0xFF) {
        res = func_8005DC9C(id - 1);
    } else if ((unsigned int)(id - 0x200) < 9) {
        res = func_8005DC9C(D_8009D03C + id - 0x201);
    }
    return res;
}
