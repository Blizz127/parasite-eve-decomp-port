void func_80050D18(void);
void func_800638D8(int slot, void (*callback)(void));

void func_8004FFD0(int slot) {
    func_800638D8(slot, func_80050D18);
}
