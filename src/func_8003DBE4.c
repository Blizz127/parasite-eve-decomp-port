/* VRAM 0x8003DBE4 / file 0x2E3E4 / size 0x124. */
struct Src {
    unsigned short h[9];
    unsigned short pad;
    int w[3];
};

struct Dst {
    unsigned char pad[0x34];
    unsigned short h[9];
    int w[3];
};

struct Ctx {
    unsigned char pad[0x84];
    struct Src *table;
};

void func_8003DBE4(struct Dst *a0, struct Ctx *a1, short idx) {
    a0->h[0] = a1->table[idx].h[0];
    a0->h[1] = a1->table[idx].h[1];
    a0->h[2] = a1->table[idx].h[2];
    a0->h[3] = a1->table[idx].h[3];
    a0->h[4] = a1->table[idx].h[4];
    a0->h[5] = a1->table[idx].h[5];
    a0->h[6] = a1->table[idx].h[6];
    a0->h[7] = a1->table[idx].h[7];
    a0->h[8] = a1->table[idx].h[8];
    a0->w[0] = a1->table[idx].w[0];
    a0->w[1] = a1->table[idx].w[1];
    a0->w[2] = a1->table[idx].w[2];
}
