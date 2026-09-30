extern void func_80066BD8(unsigned short, unsigned short, unsigned short,
                           unsigned short, unsigned short);

int func_80018F0C(unsigned short **args) {
    func_80066BD8(**args, **(args + 1), **(args + 2), **(args + 3),
                  **(args + 4));
    return 1;
}
