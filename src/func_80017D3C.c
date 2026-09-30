typedef struct {
    char pad[0x224];
    short value;
} State;

extern State *D_8009D2F0;

int func_80017D3C(short **arg0) {
    register State *state asm("$3") = D_8009D2F0;
    int value = (*arg0)[1];

    asm volatile("" : "=r"(value) : "0"(value));
    state->value = value;
    return 1;
}
