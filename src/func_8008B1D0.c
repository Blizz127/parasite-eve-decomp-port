extern void func_8008A400(void *arg0, void *arg1);

void func_8008B1D0(void *arg0) {
    func_8008A400(
        *(void **)((unsigned char *)arg0 + 4),
        *(void **)((unsigned char *)arg0 + 8)
    );
}
