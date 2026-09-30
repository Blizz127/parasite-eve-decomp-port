extern void func_80065954(void *arg0, void *arg1);

int func_80018B98(void *arg0) {
    void **first;
    void **second;

    first = *(void ***)arg0;
    second = *(void ***)((unsigned char *)arg0 + 4);
    func_80065954(*first, *second);
    return 1;
}
