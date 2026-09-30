typedef struct {
    short *first;
    short *second;
    unsigned short *third;
} Arguments;

extern void func_800661EC(short first, short second, unsigned short third, int mode);

int func_80017C54(Arguments *arg0) {
    func_800661EC(*arg0->first, *arg0->second, *arg0->third, 0);
    return 1;
}
