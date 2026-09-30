typedef struct {
    short vx;
    short vy;
    short vz;
    short pad;
} SVec;

typedef struct {
    unsigned char pad0[0x58];
    unsigned char *mats;
    unsigned char pad5C[0x28];
    unsigned char *clipMats;
} Obj;

extern void func_80079754(SVec *sv, unsigned char *m);

void func_80039D24(Obj *obj, unsigned char *src, short arg)
{
    unsigned char *mats;
    unsigned char *cursor;
    unsigned char *p;
    unsigned char *bytep;
    int i;
    int stride;
    register int half asm("$2");
    register int stride1 asm("$3");
    int off;
    int frame;
    int value;
    int n;
    int zero;
    unsigned int t;
    int sum;
    SVec *sv;
    short *ang;
    register int *tp asm("$5");

    sv = (SVec *)0x1F800000;
    cursor = src + 12;
    i = 0;
    mats = obj->clipMats;
    obj->mats = mats;
    off = (int)(short)arg << 1;
    half = src[2] >> 1;
    stride1 = half + 1;
    stride1 = stride1 << 2;
    tp = (int *)(mats + 20);
    for (; i < 3; i++) {
        p = cursor + 2;
        if (*(short *)cursor != 0) {
            value = *(short *)(cursor + 2);
            cursor += 4;
        } else {
            sum = off;
            sum = sum + (int)p;
            value = *(short *)sum;
            cursor += stride1;
        }
        *tp = value;
        tp += 1;
    }
    i = 0;
    t = src[2];
    n = src[1];
    t = t >> 2;
    t = t + 1;
    zero = 0;
    if (n < zero) {
        return;
    }
    frame = (short)arg;
    stride = t << 2;
    ang = (short *)((unsigned int)sv + 4);
    do {
        bytep = cursor + 1;
        if (cursor[0] != 0) {
            value = cursor[1];
            cursor += 4;
        } else {
            value = bytep[frame];
            cursor += stride;
        }
        sv->vx = (short)(value << 4);
        bytep = cursor + 1;
        if (cursor[0] != 0) {
            value = cursor[1];
            cursor += 4;
        } else {
            value = bytep[frame];
            cursor += stride;
        }
        ang[-1] = (short)(value << 4);
        bytep = cursor + 1;
        if (cursor[0] != 0) {
            value = cursor[1];
            cursor += 4;
        } else {
            value = bytep[frame];
            cursor += stride;
        }
        value = value << 4;
        ang[0] = (short)value;
        asm volatile("" : "=r"(ang) : "0"(ang));
        func_80079754(sv, mats);
        mats += 32;
        ang += 4;
        n = src[1];
        asm volatile("" : "=r"(n) : "0"(n));
        i += 1;
        sv += 1;
    } while (i <= n);
}
