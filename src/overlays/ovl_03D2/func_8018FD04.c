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
extern int D_801D0D8C[];
extern int D_80193268;
extern unsigned char D_80193254[];
void func_80193084();
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

void func_8018FD04(void)
{
    Task *t;
    int *p;
    int k;

    k = 4;
    p = D_801D0D8C;
    t = new_task(k);
    t->img = D_80193254 + D_80193268;
    t->f2C = k;
    t->x = p[0];
    t->y = p[1];
    t->w = *(short *)(t->img + 0x10) * 2 / 3;
    t->h = *(short *)(t->img + 0x12);
    t->f28 = p[2];
    t->f0C = func_80193084;
    t->f10 = func_8018F7F0;
}
