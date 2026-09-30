/* VRAM 0x800360B4 / file 0x268B4 / size 0x140. */
typedef struct Actor {
    unsigned int f0;
    struct Actor *next;
    struct Actor *prev;
    unsigned char pad0C[0x8C];
    unsigned int flags;
    unsigned char pad9C[0xF0];
    struct Actor *parent;
    unsigned char pad190[0x1C];
    int f1AC;
    unsigned char pad1B0[4];
    unsigned char f1B4[0xC4];
    int f278;
} Actor;

extern Actor *D_8009D20C;
extern Actor *D_8009D254;
extern unsigned int D_8009D2E8;
extern Actor *D_8009D2AC;
extern unsigned short D_8009D2A6;

extern void func_8006FE14(Actor *a0);
extern void func_800363F4(int a0);
extern void func_8003D82C(void *a0);

void func_800360B4(void) {
    Actor *a;
    Actor *next;
    unsigned int f;
    Actor *t;
    Actor *n;
    Actor *u;

    a = D_8009D20C;
    while (a != 0) {
        f = a->flags;
        a->flags = f & 0xFF7FFFFF;
        if ((a->parent != 0 && (a->parent->flags & 0x10)) || (f & 0x10)) {
            func_8006FE14(a);
            next = a->next;
            if (a == D_8009D254) {
                D_8009D254 = 0;
                D_8009D2E8 &= ~0xD;
            }
            if (a->f1AC != 0) {
                func_800363F4(a->f278);
                func_8003D82C(a->f1B4);
            }
            if (a->prev == 0) {
                n = a->next;
                t = D_8009D2AC;
                D_8009D2AC = a;
                D_8009D20C = n;
                n->prev = 0;
                a->next = t;
            } else {
                a->prev->next = a->next;
                if (a->next != 0) {
                    a->next->prev = a->prev;
                }
                u = D_8009D2AC;
                D_8009D2AC = a;
                a->next = u;
            }
            D_8009D2A6--;
            a = next;
        } else {
            a = a->next;
        }
    }
}
