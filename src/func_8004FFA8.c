void func_80050CF8(void);
void func_800638D8(int slot, void (*callback)(void));

void func_8004FFA8(int slot) {
    func_800638D8(slot, func_80050CF8);
}
