typedef struct {
    char pad[4];
    signed char value;
} Arguments;

extern short D_8009D21E;
extern int D_8009D2CC;

void func_8008C270(Arguments *arg0) {
    register int value asm("$2") = arg0->value;

    D_8009D21E = 0;
    D_8009D2CC = value << 16;
}
