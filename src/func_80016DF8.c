typedef struct {
    char pad[0x98];
    unsigned int flags;
} State;

extern State *D_8009D2F0;

int func_80016DF8(void) {
    D_8009D2F0->flags |= 0x20000000u;
    return 1;
}
