typedef struct {
    char pad[4];
    signed char value;
} Arguments;

extern short D_8009D220;
extern int D_8009D2D0;

void func_8008C16C(Arguments *arg0) {
    register int value asm("$2") = arg0->value;

    D_8009D220 = 0;
    D_8009D2D0 = value << 16;
}
