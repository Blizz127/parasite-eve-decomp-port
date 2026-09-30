typedef struct {
    unsigned char pad0[0xC];
    short hC;
    unsigned char padE[0x6A];
    int *f78;
} Obj;

typedef struct {
    unsigned char pad0[2];
    short pc;
    unsigned char pad4[2];
    signed char f6;
    signed char f7;
    int reg[16];
    int var[16];
} Vm;

extern Vm *D_800E2248;
extern unsigned char *D_800F34F4;
extern unsigned char *D_800F32A8;
extern unsigned char *D_800F3330;
extern int *D_800F33B0;

extern int func_800C2E08(void);
extern void func_800C2B90(Obj *, unsigned char, int, int);

int func_800C2758(Obj *obj, int arg1, int arg2)
{
    unsigned int op;
    unsigned short hi;
    short lo;
    int done;
    int ret;
    unsigned int h;
    signed char sd;
    int r;
    unsigned char u;

    done = 0;
    ret = 0;
    D_800E2248 = (Vm *)((unsigned char *)obj + 0xC);
    D_800F34F4 = (unsigned char *)obj + 0x80;
    D_800F32A8 = (unsigned char *)obj;
    D_800F3330 = (unsigned char *)obj + 0x200;
    D_800F33B0 = obj->f78;
    if (obj->hC == 0) {
        do {
            op = D_800F33B0[D_800E2248->pc];
            if (op == -3) {
                ret |= func_800C2E08();
            }
            if (op == -2) {
                op = 0;
            }
            if (op != -1) {
                h = op >> 16;
                hi = h;
                lo = op;
                if (hi == 1) {
                    func_800C2B90(obj, lo, arg2, arg1);
                }
                if (hi == 2) {
                    done = 1;
                    *(short *)D_800E2248 = lo;
                }
                if (h - 0x10 < 0x10) {
                    D_800E2248->reg[(unsigned char)(hi - 0x10)] = lo;
                }
                if (h - 0x20 < 0x10) {
                    r = (unsigned char)(hi - 0x20);
                    D_800E2248->reg[r] += lo;
                }
                if (h - 0x30 < 0x10) {
                    r = (unsigned char)(hi - 0x30);
                    D_800E2248->reg[r] -= lo;
                }
                if (h - 0x40 < 0x10) {
                    u = hi - 0x40;
                    D_800E2248->reg[(unsigned char)u] = D_800E2248->var[(unsigned short)lo];
                }
                if (h - 0x50 < 0x10) {
                    u = hi - 0x50;
                    D_800E2248->var[(unsigned short)lo] = D_800E2248->reg[(unsigned char)u];
                }
                if (h - 0x1000 < 0x1000) {
                    sd = hi;
                    if (D_800E2248->reg[(hi & 0xF00) >> 8] == lo) {
                        D_800E2248->pc += sd;
                    }
                }
                if ((unsigned short)(hi - 0x2000) < 0x1000) {
                    sd = hi;
                    if (D_800E2248->reg[(hi & 0xF00) >> 8] != lo) {
                        D_800E2248->pc += sd;
                    }
                }
                if ((unsigned short)(hi - 0x3000) < 0x1000) {
                    D_800E2248->pc += (signed char)hi;
                }
            } else {
                done = 1;
                D_800E2248->f7 = 1;
            }
            if (D_800E2248->f7 == 0) {
                D_800E2248->pc++;
            } else if (D_800E2248->f6 == 0) {
                ret = -1;
            }
        } while (!(unsigned char)done);
    } else {
        obj->hC -= 1;
    }
    return ret;
}
