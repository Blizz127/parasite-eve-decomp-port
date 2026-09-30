typedef struct {
    char pad[0x1E6];
    unsigned short value;
} State;

extern State *D_8009D2F0;

int func_80019298(int **arg0) {
    register State *state asm("$3") = D_8009D2F0;

    state->value = *arg0[0];
    return 1;
}
