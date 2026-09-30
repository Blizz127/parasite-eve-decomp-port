void func_800509E0(void);
void func_800638D8(int slot, void (*callback)(void));

void func_8004F978(int slot) {
    func_800638D8(slot, func_800509E0);
}
