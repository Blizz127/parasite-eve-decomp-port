typedef struct {
    unsigned char pad00[0x34];
    int *state;
} Func80064E90Object;

void func_8006269C(Func80064E90Object *object);

void func_80064E90(Func80064E90Object *object) {
    object->state[0x20] = 0;
    func_8006269C(object);
}
