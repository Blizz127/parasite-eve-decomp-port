typedef struct {
    char pad0[0xF];
    unsigned char limit;
    char pad1[2];
    unsigned short value;
    char pad2[0x84];
    unsigned int flags;
} State;

extern State *D_8009D2F0;

int func_80017B34(unsigned short **arg0) {
    unsigned short value = *arg0[0];

    if (value > D_8009D2F0->limit) {
        value = D_8009D2F0->limit;
    }
    D_8009D2F0->value = value;
    D_8009D2F0->flags |= 0x200U;
    return 1;
}
