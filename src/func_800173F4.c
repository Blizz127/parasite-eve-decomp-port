typedef struct {
    int *destination;
    int *source;
} CopyArguments;

int func_800173F4(CopyArguments *arg0) {
    *arg0->destination = *arg0->source;
    return 1;
}
