/* VRAM 0x8004AE1C / file 0x3B61C / size 0x120.
 * Save/load file-menu page input handler. era -O2 -G0 with the
 * jtbl_80011034 dispatch fold. */
int func_80062A20();
int func_80063428();
int func_8004AF3C();
int func_8004B03C();
int func_8004B13C();
int func_8004B584();
int func_8005D994();
int func_80062F1C();
int func_800439D8();
int func_800525EC();
int func_80052634();

int func_8004AE1C(int page, unsigned int event) {
    int list;
    int selected;

    list = func_80062A20(page, 0);
    if (event & 0x10000) {
        selected = func_80063428(list);
        if ((unsigned int)selected < 6) {
            switch (selected) {
            case 0:
                func_8004AF3C(list);
                break;
            case 1:
                func_8004B03C(list);
                break;
            case 2:
                func_8004B13C(list);
                break;
            case 3:
                func_8004B584(list);
                break;
            case 4:
            case 5:
            default:
                func_8005D994(func_80063428(list) - 4);
                func_80062F1C(page);
                func_800439D8();
                func_800525EC();
                break;
            }
        }
        func_800525EC();
        return 1;
    }
    if (event & 0x40) {
        func_80062F1C(page);
        func_800439D8();
        func_80052634();
    }
    return 1;
}
