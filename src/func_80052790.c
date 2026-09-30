extern int D_8009D020;

void func_80086728(int enabled);

void func_80052790(int value) {
    D_8009D020 = value;
    func_80086728(value == 0);
}
