/* VRAM 0x800327D8 / file 0x22FD8 / size 0x104. */
typedef struct {
    unsigned int tag;
    unsigned int code;
    unsigned short x;
    unsigned short y;
    short w;
    short h;
} Tile;

typedef struct {
    unsigned int tag;
    unsigned int code;
    short x;
    short y;
    unsigned char u, v;
    unsigned short clut;
    short w;
    short h;
} Sprt;

typedef struct {
    unsigned int tag;
    unsigned int code;
    Sprt spr;
} Spr;

extern Tile D_8009E358[][3];
extern int D_8009CDDC;
extern unsigned int *D_800B0E38[];
extern Spr D_8009E880[];
extern unsigned char D_8009E8B8[][56];
extern void func_80077AC4(void *a0, void *a1);
extern void func_800328DC(void *a0, short a1, short a2, short a3, int a4);

void func_800327D8(int a0, int a1) {
    int x;
    int y;
    Sprt *s;
    int k = D_8009CDDC;

    y = D_8009E358[k][0].y + a1;
    s = &D_8009E880[k].spr;
    x = D_8009E358[k][0].x + 8;
    s->x = x;
    s->y = y;
    func_80077AC4(D_800B0E38[k] + 4, D_8009E880 + k);
    func_800328DC(D_8009E8B8[D_8009CDDC], D_8009E358[D_8009CDDC][0].x + 0x40, y - 1, a0, 1);
}
