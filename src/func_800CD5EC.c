typedef struct { short x; short y; short z; short w; } V;

typedef struct {
    char c0;
    char c1;
    char c2;
    signed char f3;
    char c4[0xC];
    V pos[2];
    V vel[2];
} S;

void func_800CD5EC(int a0, char *a1, S *a2)
{
    unsigned short i;
    signed char c;

    for (i = 0; i < 2; i++) {
        a2->pos[i].x += a2->vel[i].x;
        a2->pos[i].y += a2->vel[i].y;
        a2->pos[i].z += a2->vel[i].z;
    }
    c = a2->f3 + 1;
    a2->f3 = c;
    if (c >= 8) {
        a1[1] = 2;
    }
}
