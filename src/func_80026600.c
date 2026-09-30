typedef struct {
    int f0;
    short f4;
    short f6;
} Slot;

typedef struct {
    int f0;
    int f4;
    short f8;
    short padA;
} Entry;

typedef struct {
    unsigned char pad0[0x10];
    unsigned int f10;
} Obj;

typedef struct {
    unsigned char pad0[0x68];
    Obj *f68;
} Rec;

extern Rec *D_8009D278;
extern Entry D_8009E000[];
extern Slot D_800BE830[];
extern unsigned char D_8009D2D8;
extern unsigned char D_8009CE3C;
extern unsigned char D_8009D1DC;
extern signed char D_8009D2B0[16];

extern int func_80071A54(void);

void func_80026600(int *a0) {
    unsigned int fl = D_8009D278->f68->f10;
    unsigned char i;
    unsigned char j;
    unsigned char k;
    int r;
    short two;

    if ((fl & 0xC0) == 0xC0) {
        i = 0;
        while (D_8009E000[i].f0 != 0) {
            int v;
            Slot *s;

            short g = (signed char)D_8009D2D8;
            v = D_8009E000[i].f0;
            i++;
            s = &D_800BE830[D_8009CE3C];
            s->f0 = v;
            s->f4 = 2;
            s->f6 = g;
            D_8009CE3C++;
        }
    } else if ((fl & 0xC0) == 0x40) {
        j = 0;
        if (((int)(fl & 0xF) * 3) >> 1) {
            two = 2;
            do {
                Slot *s;
                short g;
                int v;

                k = D_8009CE3C;
                j++;
                r = func_80071A54() % D_8009D2B0[0];
                g = (signed char)D_8009D2D8;
                v = D_8009E000[r].f0;
                s = &D_800BE830[k];
                s->f0 = v;
                s->f4 = two;
                s->f6 = g;
                D_8009CE3C++;
            } while (j < (((int)(D_8009D278->f68->f10 & 0xF) * 3) >> 1));
        }
    } else {
        goto other;
    }
    D_8009D1DC = 0;
    return;
other:
    {
        Slot *s = &D_800BE830[D_8009CE3C];

        s->f0 = *a0;
        s->f4 = 1;
        s->f6 = (signed char)D_8009D2D8;
        D_8009CE3C++;
        D_8009D1DC--;
    }
}
