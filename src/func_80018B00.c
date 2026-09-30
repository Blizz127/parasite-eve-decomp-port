extern void func_80067678(int, int);

int func_80018B00(int **args) {
    func_80067678(**args, **(args + 1));
    return 1;
}
