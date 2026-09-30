typedef struct Task {
    struct Task *next;  /* 0x00 */
    short x, y;         /* 0x04 */
    short w, h;         /* 0x08 */
    void *f0C;
    void *f10;
    int f14;
    unsigned char *img; /* 0x18 */
    int f1C;
    int f20;
    int f24;
    int f28;
    int f2C;
    int f30;
} Task;

extern Task *D_801D136C;
extern Task *D_801D137C;
extern Task *D_801D1378;
extern int D_801D0DB0[];
extern int D_801D0DA4[];
extern int D_80193274;
extern int D_80193270;
extern unsigned char D_80193254[];
void func_801930D8();
void func_8018F958();
void func_80192F98();
Task *func_8018FBC0(int k);
void func_8018FD04(void);
void func_8018F7F0();

static inline Task *new_task(int kind)
{
    Task *t;

    t = D_801D136C;
    D_801D136C = t->next;
    t->next = 0;
    if (D_801D137C != 0) {
        D_801D137C->next = t;
        D_801D137C = t;
    } else {
        D_801D137C = t;
        D_801D1378 = t;
    }
    *(int *)((char *)t + 12) = 0;
    *(int *)((char *)t + 16) = 0;
    *(int *)((char *)t + 20) = 0;
    *(int *)((char *)t + 24) = 0;
    *(short *)((char *)t + 4) = 0;
    *(short *)((char *)t + 6) = 0;
    *(short *)((char *)t + 8) = 0;
    *(short *)((char *)t + 10) = 0;
    *(int *)((char *)t + 28) = 0;
    *(int *)((char *)t + 32) = 0;
    *(int *)((char *)t + 36) = 0;
    *(int *)((char *)t + 40) = 0;
    *(int *)((char *)t + 44) = 0;
    *(int *)((char *)t + 48) = 0;
    return t;
}

void func_8018FE1C(void)
{
    Task *t;
    int *p;
    int k;
    Task *t2;
    int *p2;
    int k2;

    k = 7;
    p = D_801D0DB0;
    t = new_task(k);
    t->img = D_80193254 + D_80193274;
    t->f2C = k;
    t->x = p[0];
    t->y = p[1];
    t->w = *(short *)(t->img + 0x10) * 2 / 3;
    t->h = *(short *)(t->img + 0x12);
    t->f28 = p[2];
    t->f10 = func_8018F7F0;
    func_8018FBC0(3);
    func_8018FD04();
    func_8018FBC0(5)->f14 = (int)func_80192F98;
    k2 = 6;
    p2 = D_801D0DA4;
    t2 = new_task(k2);
    t2->img = D_80193254 + D_80193270;
    t2->f2C = k2;
    t2->x = p2[0];
    t2->y = p2[1];
    t2->w = *(short *)(t2->img + 0x10) * 2 / 3;
    t2->h = *(short *)(t2->img + 0x12);
    t2->f28 = p2[2];
    t2->f0C = func_801930D8;
    t2->f10 = func_8018F958;
}
