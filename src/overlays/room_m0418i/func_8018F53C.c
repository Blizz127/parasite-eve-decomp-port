/* room_m0418i — func_8018F53C, blob offset 0x554, 0x104 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl 2026-09-27).
 * init: two emitter records (func_800C4E50) + phase tables; lever: (char *) base keeps base+(off+K) unreassociated */

typedef struct {
    int p;
    unsigned char b4, b5, b6, pad7;
    unsigned char b8, b9, bA, padB;
    short hC, hE, h10, h12, h14, pad16;
} E;
typedef struct {
    E e[2];
    unsigned char pad30[0x230 - 0x30];
    short h230[2];
    unsigned char pad234[0x254 - 0x234];
    short h254[4];
    short h25C[4];
    short h264;
} W;
extern void func_800C4E50();

void func_8018F53C(void *a0, void *a1, W *a2)
{
    unsigned int i;
    int v;
    E *e;

    a2->h264 = 0x80;
    for (i = 0, v = 0; i < 4; i++) {
        a2->h254[i] = v;
        v += 300;
        a2->h25C[i] = i << 10;
    }
    for (i = 0; i < 2; i++) {
        a2->h230[i] = (i + 1) * 700;
        e = &a2->e[i];
        e->hC = 0x10;
        e->h14 = 0x80;
        e->b8 = 200;
        e->b9 = 0xFF;
        e->hE = 1300;
        e->h10 = 800;
        e->h12 = 0;
        e->bA = 0;
        e->b4 = 0;
        e->b5 = 0;
        e->b6 = 0;
        e->p = (int)((char *)a2 + ((i << 8) + 0x30));
        func_800C4E50(e);
    }
}
