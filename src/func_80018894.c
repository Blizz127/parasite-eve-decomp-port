extern void func_8006F820(void *arg0, int arg1, void *arg2);

int func_80018894(void *arg0) {
    void **first;
    void *second;

    first = *(void ***)arg0;
    second = *(void ***)((unsigned char *)arg0 + 4);
    func_8006F820(
        *first,
        1,
        second
    );
    return 1;
}
