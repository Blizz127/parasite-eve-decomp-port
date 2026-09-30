typedef struct {
    char pad[4];
    unsigned int value;
} State;

extern State *D_800E2248;

int func_800C2B68(void) {
    return (D_800E2248->value & 0xFFFF0000U) == 0x01010000U;
}
