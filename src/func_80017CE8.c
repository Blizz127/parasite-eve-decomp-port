typedef struct {
    char pad[0x28];
    int member;
} State;

extern State *D_8009D254;
extern void func_800665A0(void *arg0, int arg1, int arg2);

int func_80017CE8(void) {
    func_800665A0(&D_8009D254->member, -1, -1);
    return 1;
}
