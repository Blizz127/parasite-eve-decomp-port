typedef struct {
    char pad[0x98];
    unsigned int flags;
} State;

extern State *D_8009D2F0;

int func_80019C04(void) {
    register State *state asm("$4") = D_8009D2F0;
    register unsigned int mask asm("$3") = ~0x20000U;

    state->flags &= mask;
    return 1;
}
