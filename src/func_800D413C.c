typedef struct P {
    unsigned int f0;
    int f4;
    int f8;
    int fC;
    int f10;
    int f14;
    unsigned char *f18;
} P;

typedef struct Sub {
    P *f0;
    int f4;
    int f8;
    short wC;
    unsigned char bE;
    unsigned char bF;
} Sub;

typedef struct Rec {
    unsigned short id;
    unsigned short cnt;
    int a1;
    int a0;
} Rec;

typedef struct VM {
    unsigned short *pc;
    int f4;
    void *f8;
    unsigned char bC;
    unsigned char bD;
    unsigned short wE;
    unsigned short w10;
    unsigned short w12[7];
    Rec recs[8];
    int *f80;
} VM;

typedef struct Obj {
    unsigned char b0;
    unsigned char b1;
    short w2;
    int f4;
    Sub *f8;
    VM vm;
} Obj;

extern Obj *D_800F32D0;
extern VM *D_800E2368;
extern int D_800F3428;
extern Rec *D_800F33E0;
extern int D_800E27EC;

extern void func_800D401C(int);
extern int func_8006DC18(int);
extern int func_800CE688(int);

int func_800D413C(Obj *a0)
{
    VM *vm;
    unsigned short *p;
    unsigned short op;
    unsigned short arg1;
    unsigned short arg2;
    int loop;
    P *u;
    Sub *t;
    P *p5;
    P *p5b;
    Sub *t5;
    P *pt;
    Sub *tt;
    int n;
    unsigned int w;
    unsigned int k;
    unsigned char *b;
    unsigned short *np;
    Rec *r;
    int i;
    int st;
    int (*h)();

    loop = 1;
    vm = &a0->vm;
    D_800F32D0 = a0;
    D_800E2368 = vm;

    while (loop) {
        if (vm->wE != 0) {
            vm->wE = vm->wE - 1;
            if (vm->wE != 0) {
                break;
            }
            vm->wE = 0;
        }
        p = vm->pc;
        op = *p;
        vm->pc = p + 1;
        arg1 = *vm->pc;
        vm->pc = p + 2;
        arg2 = *vm->pc;
        vm->pc = p + 3;

        switch ((short)op) {
        case 0:
            vm->pc = vm->pc - 3;
            if (vm->bC != 0) {
                goto stop;
            }
            a0->b0 = 4;
            loop = 0;
            if (D_800E2368->bD == 0) {
                break;
            }
            u = D_800F32D0->f8->f0;
            if (u == 0) {
                break;
            }
            u->f0 = u->f0 & 0xC0FFFFFF;
            D_800F32D0->f8->f0->f18[0] = 4;
            D_800E2368->bD = 0;
            break;
        case 1:
            vm->w12[(short)arg1] = arg2;
            break;
        case 2:
            func_800D401C((short)arg1);
            break;
        case 3:
            vm->w12[(short)arg2] = vm->w12[(short)arg2] - 1;
            if ((short)vm->w12[(short)arg2] <= 0) {
                vm->w12[(short)arg2] = 0;
stop:
                loop = 0;
            } else {
                np = (unsigned short *)(int)(short)arg1;
                goto setpc;
            }
            break;
        case 4:
            if ((short)arg2 != 1) {
                vm->wE = arg1;
                goto stop;
            }
            if ((short)vm->w12[(short)arg1] != 0) {
                break;
            }
            np = vm->pc;
            loop = 0;
setpc:
            vm->pc = np - 3;
            break;
        case 5:
            if (arg1 == 0) {
                p5 = D_800F32D0->f8->f0;
                p5->f0 = (p5->f0 & 0xC0FFFFFF) | 0x1000000;
                D_800E2368->bD = 1;
                break;
            }
            if (D_800E2368->bD == 0) {
                break;
            }
            t5 = D_800F32D0->f8;
            if (t5 == 0) {
                break;
            }
            p5b = t5->f0;
            if (p5b == 0) {
                break;
            }
            b = p5b->f18;
            if (b[0] != 1) {
                break;
            }
            b[0] = 2;
            break;
        }
    }

    if (D_800E2368->bD != 0) {
        tt = D_800F32D0->f8;
        if (tt != 0) {
            if (tt->f0 != 0) {
                pt = tt->f0;
                w = pt->f0;
                k = (w >> 24) & 0x3F;
                if ((int)k >= 2) {
                    pt->f0 = (w & 0xC0FFFFFF) | (((k - 1) & 0x3F) << 24);
                }
                n = ((unsigned char *)pt)[3] & 0x3F;
                if (n > 0) {
                    if (pt->f18[0] != 0) {
                        goto tail;
                    }
                    if (pt->f10 <= 0) {
                        a0->b0 = 4;
                    }
                }
                if (pt->f18[0] != 0) {
                    goto tail;
                }
                if ((pt->f0 & 0x180E) != 0) {
                    a0->b0 = 4;
                }
                if (D_800F32D0->f8->bE < 2) {
                    return 0;
                }
            }
        }
    }
tail:
    r = &vm->recs[0];
    if (vm->bD != 0) {
        D_800F3428 = func_8006DC18(a0->b1);
    }
    for (i = 0; i < 8; i++, r++) {
        if (r->id != 0xFFFF) {
            D_800E27EC = r->cnt;
            h = (int (*)())vm->f80[r->id];
            D_800F33E0 = r;
            st = h(1, r->a1, vm->f8);
            if (r->a0 != 0) {
                if (st == 2) {
                    if (func_800CE688(r->a0) == 0) {
                        st = 1;
                    }
                } else {
                    func_800CE688(r->a0);
                }
            }
            r->cnt = r->cnt + 1;
            if (st == 1) {
                r->id = 0xFFFF;
                vm->bC = vm->bC - 1;
            }
        }
    }
    return 0;
}
