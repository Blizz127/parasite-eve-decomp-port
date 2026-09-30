typedef struct {
    char pad[0x98];
    unsigned int flags;
} State;

extern State *D_8009D2F0;

int func_80019AE4(void) {
    register State *state asm("$2") = D_8009D2F0;
    register unsigned int mask asm("$4") = ~0x400U;

    state->flags &= mask;
    return 1;
}
