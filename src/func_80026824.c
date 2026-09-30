typedef struct Q {
    void *who;
    short cmd;
    short n;
} Q;

typedef struct Pick {
    void *who;
    int a;
    int b;
} Pick;

typedef struct Act {
    void *f0;
    unsigned char pad4[0x2A - 4];
    short f2A;
    unsigned char pad2C[2];
    short f2E;
    unsigned char pad30[2];
    short f32;
} Act;

typedef struct St {
    unsigned char pad0[0x12];
    unsigned char f12;
} St;

extern short D_8009D2A4;
extern Act *D_8009D254;
extern St *D_8009D278;
extern Q D_800BE830[];
extern signed char D_8009D2B0;
extern Pick D_8009E000[];
extern unsigned int D_8009D1A0;
extern unsigned int D_8009D1F4;
extern int D_800B0E08[];
extern unsigned char D_8009CE3C;
extern signed char D_8009CE40;
extern signed char D_8009CE44;
extern unsigned short D_8009CE50;
extern unsigned char D_8009CE60;
extern unsigned char D_8009D25C;
extern unsigned char D_8009D2D8;

extern void func_80026FD0(void);
extern int func_80071A54(void);
extern void func_8006DE80(int, int, int, int, int);
extern void func_800218D8(void);
extern void func_800209F0(void);
extern void func_80021AF8(void);
extern void func_8006DF50(int, int, int, int, int);
extern void func_800258CC(int);
extern void func_8005112C(void);

signed char func_80026824(signed char mode)
{
    signed char r;
    int s;

    r = mode;
    if (mode == 1) {
        s = D_8009D2A4;
        D_8009CE60 = 0;
        if (s > 0) {
            if (s < 0x183) {
                D_8009CE40 = 0;
                {
                Q *q = &D_800BE830[D_8009CE3C];
                q->who = D_8009D254;
                q->cmd = D_8009D2A4;
                q->n = (signed char)D_8009D2D8;
                }
                D_8009CE3C++;
                D_8009D2D8--;
                r = 0;
            } else if (s < 0x197) {
                switch (s) {
                case 0x189:
                    func_80026FD0();
                    D_8009CE40 = 4;
                    r++;
                    break;
                case 0x18A:
                    func_80026FD0();
                    D_8009CE40 = 5;
                    r++;
                    break;
                case 0x18B:
                    func_80026FD0();
                    D_8009CE40 = 6;
                    r++;
                    break;
                case 0x18D:
                    func_80026FD0();
                    D_8009CE40 = 7;
                    r++;
                    break;
                case 0x196: {
                    unsigned char k;
                    unsigned char c;

                    func_80026FD0();
                    D_8009D25C = 0;
                    r = 0;
                loop: {
                        int rnd;
                        Q *q;
                        void *w;
                        short g;

                        k = D_8009CE3C;
                        r++;
                        rnd = func_80071A54() % D_8009D2B0;
                        c = D_8009D2D8;
                        g = (signed char)c;
                        D_8009CE3C++;
                        w = D_8009E000[rnd].who;
                        q = &D_800BE830[k];
                        q->n = g;
                        q->cmd = D_8009D2A4;
                        q->who = w;
                    }
                    if ((unsigned char)r < 7) {
                        goto loop;
                    }
                    D_8009D2D8 = c - 1;
                    D_8009CE40 = 0;
                    r = 0;
                    break;
                }
                default:
                    D_8009CE40 = 0;
                    {
                    Q *q = &D_800BE830[D_8009CE3C];
                    q->who = D_8009D254;
                    q->cmd = D_8009D2A4;
                    q->n = (signed char)D_8009D2D8;
                    }
                    D_8009D2D8--;
                    D_8009CE3C++;
                    r = 0;
                    break;
                }
            } else if (s < 0x199) {
                unsigned char k;

                r = -1;
                k = D_8009CE3C;
                (&D_800BE830[k])->who = D_8009D254;
                (&D_800BE830[k])->cmd = D_8009D2A4;
                (&D_800BE830[k])->n = (signed char)D_8009D2D8;
                if (!(D_8009D1A0 & 2)) {
                    D_8009D278 = D_8009D254->f0;
                    func_8006DE80(0x453, 1, D_8009D254->f2A, D_8009D254->f2E, D_8009D254->f32);
                }
                switch (s) {
                case 0x197:
                    func_800218D8();
                    func_800209F0();
                    D_8009D278->f12 = 0xD;
                    break;
                case 0x198:
                    func_80021AF8();
                    break;
                }
                D_8009D2D8 = 0;
                D_8009CE40 = 0;
                D_8009CE3C++;
                r = -1;
            } else {
                r = -1;
                {
                Q *q = &D_800BE830[D_8009CE3C];
                q->who = D_8009D254;
                q->cmd = D_8009D2A4;
                q->n = (signed char)D_8009D2D8;
                }
                D_8009D2D8 = 0;
                D_8009CE40 = 0;
                D_8009CE3C++;
            }
            if (D_8009CE40 == 0 && D_800B0E08[0] != 0) {
                func_8006DF50(D_800B0E08[0], 0x44C, 0, 0x80, 0x7F);
            }
            D_8009CE50 = D_8009D2A4;
        } else if (s == -1) {
            r = 0;
        }
    } else if (mode == 2) {
        func_800258CC(D_8009CE40);
        if (D_8009D1F4 & 0x200) {
            D_8009CE40 = 0;
            {
            void *w = D_8009E000[D_8009CE44].who;
            Q *q = &D_800BE830[D_8009CE3C];
            q->n = (signed char)D_8009D2D8;
            q->cmd = D_8009CE50;
            q->who = w;
            }
            D_8009D2D8--;
            D_8009CE3C++;
            r = 0;
            if (D_800B0E08[0] != 0) {
                func_8006DF50(D_800B0E08[0], 0x44C, 0, 0x80, 0x7F);
            }
        } else if (D_8009D1F4 & 0x400) {
            r = 0;
            func_8005112C();
            D_8009CE44 = 0;
            if (D_800B0E08[0] != 0) {
                func_8006DF50(D_800B0E08[0], 0x44D, 0, 0x80, 0x7F);
            }
        }
    }
    return r;
}
