typedef struct {
    char pad[0x250];
    unsigned short flags;
} State;

extern State *D_8009D2F0;

int func_800199F8(void) {
    register State *state asm("$3") = D_8009D2F0;

    state->flags &= ~0x10U;
    return 1;
}
