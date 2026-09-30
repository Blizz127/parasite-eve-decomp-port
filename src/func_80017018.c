typedef struct Obj {
    unsigned int *pc;
    int f4;
    unsigned int f8;
    int fC;
    int wait;
    unsigned char pad14[0x10];
    struct Obj *next;
} Obj;

typedef struct State {
    unsigned char pad0[0x98];
    unsigned int f98;
    unsigned char pad9C[0x10];
    int vars[1];
} State;

extern Obj *D_8009D300;
extern unsigned int *D_8009CE00;
extern State *D_8009D2F0;
extern unsigned int D_8009D1A0;
extern State *D_8009D254;
extern int (*D_800910A0[])(int **);
extern int D_8009DF70[];
extern int D_800A77F0[];
extern int D_800B6A80[];

void func_80017018(void)
{
    int *args[16];
    register unsigned int w asm("$6");
    unsigned int ext;
    unsigned int modes;
    unsigned int op;
    unsigned int n;
    unsigned int *ops;
    unsigned short i;
    register unsigned int *p asm("$2");

    do {
        if (!(D_8009D300->f8 & 0x50)
            && (!(D_8009D2F0->f98 & 0x1000) || (*(unsigned short *)&D_8009D300->f8 & 0x80))
            && (!(D_8009D1A0 & 0x100) || D_8009D2F0 == D_8009D254
                || (*(unsigned short *)&D_8009D300->f8 & 0x80))
            && D_8009D300->wait != 0 && --D_8009D300->wait == 0) {
            D_8009CE00 = D_8009D300->pc;
            do {
                p = D_8009CE00;
                w = *p;
                D_8009CE00 = p + 1;
                ext = *D_8009CE00;
                ops = D_8009CE00 + 1;
                modes = w >> 17;
                op = w & 0x1FFF;
                w = (w >> 13) & 0xF;
                D_8009CE00 = ops + w;
                for (i = 0; i < w; i++) {
                    switch (modes & 7) {
                    case 0:
                        args[i] = (int *)&ops[i];
                        break;
                    /* Case labels follow the retail jump table jtbl_80010690
                     * {0:imm, 1:actor vars, 2:D_800A77F0, 3:D_8009DF70,
                     * 4:D_800B6A80}; the bodies stay in retail code order. */
                    case 3:
                        args[i] = &D_8009DF70[ops[i]];
                        break;
                    case 1:
                        args[i] = &D_8009D2F0->vars[ops[i]];
                        break;
                    case 2:
                        args[i] = &D_800A77F0[ops[i]];
                        break;
                    case 4:
                        args[i] = &D_800B6A80[ops[i]];
                        break;
                    }
                    modes >>= 3;
                    if (i == 4) {
                        modes = ext;
                    }
                }
            } while (D_800910A0[op](args));
            D_8009D300->pc = D_8009CE00;
        }
        D_8009D300 = D_8009D300->next;
    } while (D_8009D300 != 0);
}
