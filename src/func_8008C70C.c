typedef struct {
    char pad[0x56];
    short value;
} State;

extern State *D_8009D2C8;

void func_8008C70C(int *arg0) {
    D_8009D2C8->value = arg0[1];
}
