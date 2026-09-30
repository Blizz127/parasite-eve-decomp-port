extern int *D_8009D0DC;
extern int *D_8009D0E0;
extern int *D_8009D0E4;
extern void func_800527C0(int);

void func_8005DEE4(int a0, int a1)
{
    int *node;
    int *tail;
    int *next;

    node = D_8009D0DC;
    if (node != 0) {
        next = (int *)node[0];
        tail = D_8009D0E4;
        node[0] = 0;
        D_8009D0DC = next;
        if (tail != 0) {
            tail[0] = (int)node;
        } else {
            if (D_8009D0E0 != 0) {
                func_800527C0(0x1F);
            }
            D_8009D0E0 = node;
        }
        D_8009D0E4 = node;
        node[1] = a0;
        node[2] = a1;
    }
}
