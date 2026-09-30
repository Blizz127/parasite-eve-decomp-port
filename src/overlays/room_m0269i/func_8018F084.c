/* room_m0269i — func_8018F084, blob offset 0x9C, 0xAC bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl 2026-09-27).
 * message handler: 0x19 registers a3, 0x200 searches actor list for (a3,a4) ids */

typedef struct Obj { unsigned char pad[4]; struct Obj *next; unsigned char pad2[4]; unsigned char a, b; unsigned char pad3[0x98 - 0xE]; int flags; } Obj;
typedef struct { unsigned char pad[4]; int *ret; unsigned char pad2[8]; Obj *cur; } Sub;
extern Obj *D_8009D20C;

int func_8018F084(unsigned char *a0, int a1, int a2, int *a3, int a4)
{
    Sub *s = (Sub *)(a0 + 0xC);

    switch (a2) {
    case 0x19:
        if (a1 == 1) {
            *(int **)(a0 + 0x10) = a3;
            *a3 = a1;
        }
        break;
    case 0x200:
        s->cur = D_8009D20C;
        if (s->cur != 0) {
            for (; s->cur != 0; s->cur = s->cur->next) {
                if (s->cur->a == (int)a3 && s->cur->b == a4 && !(s->cur->flags & 0x10)) {
                    break;
                }
            }
        }
        break;
    }
    return 0;
}
