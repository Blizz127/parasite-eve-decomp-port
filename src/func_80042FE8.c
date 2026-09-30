typedef struct {
    short x, y, w, h;
} RECT;

extern int D_8009CED8;
extern int D_8009CEDC;
extern void *D_800B0E54[];

int func_8007506C(RECT *rect, void *data);

void func_80042FE8(void) {
    RECT rect;

    if (D_8009CED8 == 6) {
        rect.x = 0;
        rect.y = 0x1E0;
        rect.w = 0x100;
        rect.h = D_8009CEDC;
        func_8007506C(&rect, D_800B0E54[0]);
    }
}
