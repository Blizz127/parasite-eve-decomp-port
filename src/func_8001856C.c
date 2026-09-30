typedef struct {
    char pad[0x68];
    int value68;
    int value6C;
    int value70;
    char pad74[4];
    int value78;
    int value7C;
    int value80;
} State;

extern State *D_8009D2F0;

int func_8001856C(void) {
    State *state = D_8009D2F0;

    state->value68 = 0;
    state->value6C = 0;
    state->value70 = 0;
    state->value78 = 0;
    state->value7C = 0;
    state->value80 = 0;
    return 1;
}
