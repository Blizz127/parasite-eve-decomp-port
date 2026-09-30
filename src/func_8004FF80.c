void func_80050CB4(void);
void func_800638D8(int slot, void (*callback)(void));

void func_8004FF80(int slot) {
    func_800638D8(slot, func_80050CB4);
}
