typedef struct {
    char pad[0x98];
    unsigned int flags;
} State;

extern State *D_8009D2F0;

int func_80017D9C(void) {
    register State *state asm("$3") = D_8009D2F0;

    state->flags |= 0x40;
    return 1;
}
