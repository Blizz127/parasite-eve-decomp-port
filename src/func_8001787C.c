typedef struct Task {
    unsigned char pad0[0x8];
    unsigned short flags;
    unsigned short id;
    unsigned char padC[0x18];
    struct Task *next;
} Task;

typedef struct {
    unsigned char pad0[0xA0];
    Task *lists[3];
} State;

extern Task *D_8009D300;
extern State *D_8009D2F0;

int func_8001787C(unsigned short **a0) {
    Task *self = D_8009D300;
    unsigned short id = **a0;
    unsigned char i;
    Task *p;

    if (self->id == id) {
        self->flags |= 0x10;
        return 0;
    }
    for (i = 0; i < 3; i++) {
        for (p = D_8009D2F0->lists[i]; p != 0; p = p->next) {
            if (p->id == id) {
                p->flags |= 0x10;
                return 1;
            }
        }
    }
    return 1;
}
