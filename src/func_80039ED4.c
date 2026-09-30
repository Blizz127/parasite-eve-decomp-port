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

void func_80039ED4(Obj *obj, unsigned char *src, short arg)
{
    unsigned char *cursor;
    unsigned char *p;
    unsigned char *bytep;
    short i;
    register int stride1 asm("$5");
    register int stride2 asm("$21");
    register int step asm("$7");
    register int t0v asm("$2");
    register int nb asm("$3");
    int off;
    int frame;
    int value;
    int n;
    int zero;
    int sum;
    register SVec *sv asm("$18");
    short *ang;
    int *tp;
    register unsigned char *mats asm("$20");

    sv = (SVec *)0x1F800000;
    cursor = src + 12;
    i = 0;
    mats = obj->clipMats;
    obj->mats = mats;
    off = (int)(short)arg << 1;
    t0v = src[2] >> 1;
    step = t0v + 1;
    stride1 = step << 2;
    tp = (int *)(mats + 20);
    do {
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
        i = (short)(i + 1);
    } while (i < 3);
    n = src[1];
    zero = 0;
    if (n < zero) {
        return;
    }
    i = 0;
    frame = (int)(short)arg << 1;
    stride2 = step << 2;
    ang = (short *)((unsigned int)sv + 4);
    do {
        bytep = cursor + 2;
        if (*(short *)cursor != 0) {
            value = *(unsigned short *)(cursor + 2);
            cursor += 4;
        } else {
            sum = frame;
            sum = sum + (int)bytep;
            value = *(unsigned short *)sum;
            cursor += stride2;
        }
        sv->vx = (short)value;
        bytep = cursor + 2;
        if (*(short *)cursor != 0) {
            value = *(unsigned short *)(cursor + 2);
            cursor += 4;
        } else {
            sum = frame;
            sum = sum + (int)bytep;
            value = *(unsigned short *)sum;
            cursor += stride2;
        }
        ang[-1] = (short)value;
        bytep = cursor + 2;
        if (*(short *)cursor != 0) {
            value = *(unsigned short *)(cursor + 2);
            cursor += 4;
        } else {
            sum = frame;
            sum = sum + (int)bytep;
            value = *(unsigned short *)sum;
            cursor += stride2;
        }
        ang[0] = (short)value;
        asm volatile("" : "=r"(ang) : "0"(ang));
        func_80079754(sv, mats);
        mats += 32;
        ang += 4;
        i = (short)(i + 1);
        nb = src[1];
        sv += 1;
    } while (i <= nb);
}
