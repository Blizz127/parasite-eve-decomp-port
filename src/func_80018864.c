extern void func_8006F820(void *arg0, int arg1, void *arg2);

int func_80018864(void *arg0) {
    void **first;
    void **second;

    first = *(void ***)arg0;
    second = *(void ***)((unsigned char *)arg0 + 4);
    func_8006F820(*first, 0, *second);
    return 1;
}
