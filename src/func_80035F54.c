/*
 * func_80035F54 — VRAM 0x80035F54 / file 0x26754 / size 0xC8 (50 words).
 * Actor contact/pose restore. When actor+0x18C (the parent link) is set the
 * routine recurses on it, then walks the D_8009D20C list (gp+0x49C) and
 * restores every entry whose parent link matches this actor's. When the
 * parent link is null the actor itself is restored and its +0x98 status
 * word gets bit 0x40000.
 * Build: era -O2 -G8.
 */

typedef struct Actor Actor;
struct Actor {
    char pad00[4];            /* 0x00..0x03 */
    Actor *next;              /* +0x04 */
    char pad08[0x20];         /* 0x08..0x27 */
    int f28;                  /* +0x28 */
    int f2C;                  /* +0x2C */
    int f30;                  /* +0x30 */
    char pad34[0x0C];         /* 0x34..0x3F */
    int f40;                  /* +0x40 */
    int f44;                  /* +0x44 */
    int f48;                  /* +0x48 */
    char pad4C[0x4C];         /* 0x4C..0x97 */
    unsigned int f98;         /* +0x98 */
    char pad9C[0xF0];         /* 0x9C..0x18B */
    Actor *f18C;              /* +0x18C */
    char pad190[0x14];        /* 0x190..0x1A3 */
    int f1A4;                 /* +0x1A4 */
    int f1A8;                 /* +0x1A8 */
};

extern Actor *D_8009D20C;

void func_80035F54(Actor *a) {
    Actor *p;

    if (a->f18C != 0) {
        func_80035F54(a->f18C);
        for (p = D_8009D20C; p != 0; p = p->next) {
            if (p->f18C == a->f18C) {
                p->f28 = p->f40;
                p->f2C = p->f44;
                p->f30 = p->f48;
                p->f1A4 = p->f1A8;
            }
        }
    } else {
        a->f28 = a->f40;
        a->f2C = a->f44;
        a->f30 = a->f48;
        a->f1A4 = a->f1A8;
        a->f98 |= 0x40000;
    }
}
