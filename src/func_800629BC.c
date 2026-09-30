extern int *D_8009D154;

int *func_800629BC(int a0)
{
    int *node;
    int *p;
    int *found;
    int i;

    found = 0;
    node = D_8009D154;
    while (node != 0) {
        for (i = 0, p = node; i < 4; i++, p++) {
            if (p[2] == a0) {
                break;
            }
        }
        if (i < 4) {
            found = node;
            break;
        }
        node = (int *)node[0];
    }
    return found;
}
