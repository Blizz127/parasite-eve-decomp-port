typedef struct Node {
    struct Node *next;
    int f4;
    int f8;
} Node;

extern Node *D_8009D0DC;
extern Node *D_8009D0E0;
extern Node *D_8009D0E4;

void func_8005DF6C(int mask, Node *out)
{
    Node *p;
    Node *prev;

    if (out == 0) {
        return;
    }
    if (D_8009D0E0 != 0) {
        p = D_8009D0E0;
        prev = 0;
        while (!(p->f4 & mask)) {
            prev = p;
            p = p->next;
            if (p == 0) {
                break;
            }
        }
        if (p != 0) {
            if (prev != 0) {
                prev->next = p->next;
            } else {
                D_8009D0E0 = p->next;
            }
            if (p == D_8009D0E4) {
                D_8009D0E4 = prev;
            }
            p->next = D_8009D0DC;
            D_8009D0DC = p;
            *out = *p;
            return;
        }
    }
    out->f4 = 0;
    out->f8 = 0;
}
