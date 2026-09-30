typedef struct { short vx; short vy; short vz; short pad; } SVECTOR;

typedef struct {
    SVECTOR *v;
    int f4;
    int f8;
    unsigned short fC;
    unsigned short fE;
    unsigned short f10;
} Ring;

extern int func_80077CF4(int a0);
extern int func_80077DC4(int a0);

void func_800C4E50(Ring *r)
{
    SVECTOR *v;
    int a;
    int step;
    unsigned int i;

    a = 0;
    v = r->v;
    step = 0x1000 / r->fC;
    for (i = 0; i < r->fC; i++) {
        v->vx = (func_80077CF4(a) * r->f10) >> 12;
        v->vy = (func_80077DC4(a) * r->f10) >> 12;
        v->vz = 0;
        a += step;
        v++;
    }
    a = 0;
    for (i = 0; i < r->fC; i++) {
        v->vx = (func_80077CF4(a) * r->fE) >> 12;
        v->vy = (func_80077DC4(a) * r->fE) >> 12;
        v->vz = 0;
        a += step;
        v++;
    }
}
