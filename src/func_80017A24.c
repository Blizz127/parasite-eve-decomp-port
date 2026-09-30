typedef struct {
    unsigned int *source;
    unsigned int *bit;
    unsigned int *destination;
} Arguments;

int func_80017A24(Arguments *arg0) {
    *arg0->destination = *arg0->source & (1U << *arg0->bit);
    return 1;
}
