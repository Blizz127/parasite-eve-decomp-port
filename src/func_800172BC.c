typedef struct {
    char pad[0x98];
    unsigned int flags;
} State;

extern State *D_8009D2F0;

int func_800172BC(void) {
    register State *state asm("$3") = D_8009D2F0;

    state->flags |= 0x10;
    return 0;
}
