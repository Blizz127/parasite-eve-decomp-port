/* VRAM 0x80031D6C / file 0x2256C / size 0xFC. */
typedef struct {
    unsigned int tag;
    unsigned int code;
    short x;
    short y;
    short w;
    short h;
} Tile;

extern Tile D_8009E358[][3];
extern int D_8009CDDC;
extern unsigned int *D_800B0E38[];
extern void func_80077AC4(void *a0, void *a1);

void func_80031D6C(int a0) {
    unsigned char i;
    for (i = 0; i < 3; i++) {
        (D_8009E358[D_8009CDDC] + i)->x = D_8009E358[D_8009CDDC][0].x + i;
        (D_8009E358[D_8009CDDC] + i)->y = D_8009E358[D_8009CDDC][0].y + i;
        (D_8009E358[D_8009CDDC] + i)->w = 0x50 - i * 2;
        (D_8009E358[D_8009CDDC] + i)->h = (signed char)a0 - i * 2;
        func_80077AC4(D_800B0E38[D_8009CDDC] + (7 - i), D_8009E358[D_8009CDDC] + i);
    }
}
