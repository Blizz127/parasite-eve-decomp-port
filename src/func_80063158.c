typedef struct {
    unsigned char pad[0x18];
    int x;
    int y;
} PositionNode;

extern int D_8009D124;
extern int D_8009D128;

void func_80063158(PositionNode *node, int x, int y) {
    if (node != 0) {
        node->x += x;
        node->y += y;
        D_8009D124 += x;
        D_8009D128 += y;
    }
}
