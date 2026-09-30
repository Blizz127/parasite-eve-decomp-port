typedef struct {
    char pad[0xD];
    unsigned char value;
} State;

extern State *D_8009D2F0;

int func_80017948(unsigned int **arg0) {
    D_8009D2F0->value = *arg0[0];
    return 1;
}
