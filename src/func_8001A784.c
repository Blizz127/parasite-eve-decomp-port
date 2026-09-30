/* VRAM 0x8001A784 / size 0x10C. Sibling of func_8001A680: skip the
 * handler reload when +0xE already holds the id, then the same
 * ~0x200 clear and 0x200000 child walk. era -O2 -G0. */

typedef struct Body {
    unsigned char pad00[0x04];
    struct Body *next;
    unsigned char pad08[0x04];
    unsigned char classId;
    unsigned char pad0D;
    unsigned char handlerId;
    unsigned char handlerCount;
    unsigned char pad10[0x04];
    int value;
    int value2;
    unsigned char pad1C[0x98 - 0x1C];
    unsigned int flags;
    unsigned char pad9C[0x18C - 0x9C];
    struct Body *link;
    unsigned char pad190[0x1B0 - 0x190];
    void *handler;
} Body;

typedef struct HandlerClass {
    void *entries[0xC0 / 4];
} HandlerClass;

extern HandlerClass D_800B0E98[];
extern void *D_8009D20C[];

void func_8001A784(Body *body, unsigned int id) {
    unsigned int masked;
    unsigned int f;
    Body *b;

    masked = id & 0xFFFF;
    if (body->handlerId != masked) {
        unsigned char cls = body->classId;
        HandlerClass *tbl = D_800B0E98;
        void *h;

        body->value = 0;
        body->value2 = 0;
        body->handlerId = id;
        h = tbl[cls].entries[masked];
        body->handler = h;
        body->handlerCount = ((unsigned char *)h)[2] - 1;
    }

    f = body->flags & ~0x200u;
    body->flags = f;
    if ((f & 0x100000u) != 0) {
        b = (Body *)D_8009D20C[0];
        while (b != 0) {
            if (b->link == body && (b->flags & 0x200000u) != 0) {
                func_8001A784(b, id & 0xFFFF);
            }
            b = b->next;
        }
    }
}
