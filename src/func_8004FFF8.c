void func_80050D20(void);
void func_800638D8(int slot, void (*callback)(void));

void func_8004FFF8(int slot) {
    func_800638D8(slot, func_80050D20);
}
