/*
 * func_8001A680 — activate a body's handler slot and recurse into the
 * 0x200000-linked children.
 *
 * VRAM 0x8001A680 / file 0xAE80 / size 0x104 (65 words).
 *
 * Retail shape (asm/disc1/AB74.s):
 *   tbl = D_800B0E98;                                     (0xC0-byte rows)
 *   handler = tbl[body->classId].entries[(u16)id];
 *   body->f0E = id; body->f14 = 0; body->f18 = 0; body->f1B0 = handler;
 *   body->f98 &= ~0x200;
 *   body->f0F = handlerByte[2] - 1;
 *   if (body->f98 & 0x100000)
 *       for (b = D_8009D20C[0]; b; b = b->next)
 *           if (b->f18C == body && (b->f98 & 0x200000))
 *               func_8001A680(b, id & 0xFFFF);
 *
 * Build: era -O2 -G0.
 * ROM: asm/disc1/AB74.s @ file 0xAE80, 65 words (0x104 bytes).
 */

typedef struct Body {
    unsigned char pad00[0x04];
    struct Body *next;              /* +0x04 */
    unsigned char pad08[0x04];
    unsigned char classId;          /* +0x0C */
    unsigned char pad0D;
    unsigned char handlerId;        /* +0x0E */
    unsigned char handlerCount;     /* +0x0F */
    unsigned char pad10[0x04];
    int value;                      /* +0x14 */
    int value2;                     /* +0x18 */
    unsigned char pad1C[0x98 - 0x1C];
    unsigned int flags;             /* +0x98 */
    unsigned char pad9C[0x18C - 0x9C];
    struct Body *link;              /* +0x18C */
    unsigned char pad190[0x1B0 - 0x190];
    void *handler;                  /* +0x1B0 */
} Body;

typedef struct HandlerClass {
    void *entries[0xC0 / 4];
} HandlerClass;

extern HandlerClass D_800B0E98[];   /* 0xC0-byte rows, indexed by classId */
extern void *D_8009D20C[];          /* absolute: body list head (gp+0x48C) */

void func_8001A680(Body *body, unsigned int id) {
    HandlerClass *tbl;
    void *h;
    Body *b;

    tbl = D_800B0E98;
    h = tbl[body->classId].entries[(unsigned short)id];
    body->handlerId = id;
    body->value = 0;
    body->value2 = 0;
    body->handler = h;
    body->flags &= ~0x200u;
    body->handlerCount = ((unsigned char *)body->handler)[2] - 1;

    if ((body->flags & 0x100000u) != 0) {
        b = (Body *)D_8009D20C[0];
        while (b != 0) {
            if (b->link == body && (b->flags & 0x200000u) != 0)
                func_8001A680(b, id & 0xFFFF);
            b = b->next;
        }
    }
}
