/* VRAM 0x8002F658 / size 0x114. Copy the default records through a
 * stack image, then publish the two trailing zeros. era -O2 -G8.
 * D_8009D1B0 is a gp scalar; D_8009D1B4 is outside the small-data
 * window (3-word object). */

typedef struct {
    int w[28];
} Rec70;

typedef struct {
    int w[6];
} Rec18;

typedef struct {
    Rec70 b;
    Rec18 t;
    int z0;
    int z1;
} Image;

extern Rec70 D_80010928;
extern Rec18 D_80010998;
extern Rec70 D_800B8A20;
extern Rec18 D_800B0CB0;
extern int D_8009D1B0;
extern int D_8009D1B4[3];

void func_8002F658(void) {
    Image img;

    img.b = D_80010928;
    img.t = D_80010998;
    img.z0 = 0;
    img.z1 = 0;
    D_800B8A20 = img.b;
    D_800B0CB0 = img.t;
    /* Keep the trailing loads below the 0x18 copy (retail order). */
    asm volatile("" : : : "memory");
    D_8009D1B0 = img.z0;
    D_8009D1B4[0] = img.z1;
}
