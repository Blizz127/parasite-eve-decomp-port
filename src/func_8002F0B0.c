typedef struct Info {
    unsigned char pad0[6];
    signed char b6;
    unsigned char pad7[0xA8];
    unsigned char bAF;
} Info;

typedef struct Actor {
    Info *info;
    struct Actor *next;
    unsigned char pad8[0x60];
    int f68;
    int f6C;
    int f70;
    unsigned char pad74[0x24];
    unsigned int f98;
    unsigned char pad9C[0xF0];
    int f18C;
    unsigned char pad190[0x24];
    unsigned char sub[0x9C];
    unsigned short h250;
    unsigned char b252;
} Actor;

extern unsigned char D_8009CE74;
extern unsigned char D_8009D244;
extern int D_8009D28C;
extern Actor *D_8009D20C;
extern Actor *D_8009D254;
extern void func_8001A680(Actor *, unsigned short);
extern void func_8003C5D8(void *, int);
extern void func_800703F4(void);
extern int func_8006D60C(int);
extern void func_8002F9CC(void);
extern void func_800295E4(void);
extern void func_800293F4(int);

void func_8002F0B0(void)
{
    Actor *a;
    unsigned char ok;
    unsigned int v;

    switch (D_8009CE74) {
    case 0:
        D_8009D244 = 0;
        for (a = D_8009D20C; a != 0; a = a->next) {
            if (a == D_8009D254) {
                continue;
            }
            if (a->info == 0) {
                if (a->f18C == 0) {
                    continue;
                }
            } else {
                func_8001A680(a, a->info->b6);
                a->f68 = 0;
                a->f6C = 0;
                a->f70 = 0;
                a->f98 |= 0x1000;
                v = *(volatile unsigned int *)&a->f98;
                if ((v & 0x40000000) && a->info->bAF == 0) {
                    a->f98 = v | 0x10;
                    a->info = 0;
                }
            }
            func_8003C5D8(a->sub, 30);
            a->h250 |= 2;
        }
        D_8009CE74++;
        break;
    case 1:
        ok = 1;
        for (a = D_8009D20C; a != 0; a = a->next) {
            if (a == D_8009D254) {
                continue;
            }
            if (a->info == 0 && a->f18C == 0) {
                continue;
            }
            if (a->b252 == 0) {
                a->info = 0;
                a->f98 |= 0x10;
            } else if (a->f98 & 0x40) {
                a->f98 |= 0x10;
                a->info = 0;
            } else {
                ok = 0;
            }
        }
        if (ok) {
            func_800703F4();
            D_8009CE74++;
        }
        break;
    case 2:
        if (func_8006D60C(0) != 1) {
            func_8002F9CC();
            func_8001A680(D_8009D254, 21);
            func_800295E4();
            D_8009D28C = 11;
            func_800293F4(0);
        }
        break;
    }
}
