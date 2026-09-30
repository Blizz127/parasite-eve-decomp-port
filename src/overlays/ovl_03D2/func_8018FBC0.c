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
typedef struct { int x, y, z; } Pos;
extern Pos D_801D0D5C[];
extern int D_80193258[];
extern unsigned char D_80193254[];
void func_80192FE8();
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

Task *func_8018FBC0(int k)
{
    Task *t;
    Pos *p;
    int pad[2];

    p = &D_801D0D5C[k];
    t = new_task(k);
    t->img = D_80193254 + D_80193258[k];
    t->f2C = k;
    t->x = p->x;
    t->y = p->y;
    t->w = *(short *)(t->img + 0x10) * 2 / 3;
    t->h = *(short *)(t->img + 0x12);
    t->f28 = p->z;
    t->f0C = func_80192FE8;
    t->f10 = func_8018F7F0;
    t->f20 = t->y;
    t->f24 = -0x10;
    return t;
}
