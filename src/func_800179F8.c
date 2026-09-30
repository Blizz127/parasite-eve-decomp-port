typedef struct {
    unsigned int *source;
    unsigned int *bit;
} Arguments;

int func_800179F8(Arguments *arg0) {
    *arg0->source &= ~(1U << *arg0->bit);
    return 1;
}
