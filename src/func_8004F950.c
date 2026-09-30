void func_800509A8(void);
void func_800638D8(int slot, void (*callback)(void));

void func_8004F950(int slot) {
    func_800638D8(slot, func_800509A8);
}
