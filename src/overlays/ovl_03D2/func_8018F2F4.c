typedef struct {
    short x, y, w, h;
} RECT;

extern int D_801D11C8;
extern unsigned char *D_801D11C4;
extern unsigned char *D_801D11BC[];
extern int D_80193258;
extern unsigned char D_80193268[];
void func_8007506C(RECT *r, void *p);
void func_80074DC0(int a);
void func_80073A44(int a);
void func_80074A44(int a);
void func_80075424(void *p);
void func_800755F0(void *p);

void func_8018F2F4(void)
{
    RECT r;
    int i;

    i = 0;
    do {
        D_801D11C8 = D_801D11C8 == 0;
        D_801D11C4 = D_801D11BC[D_801D11C8];
        r.x = 0;
        r.y = D_801D11C8 == 0 ? 0x104 : 0x14;
        r.w = 0x1E0;
        r.h = 0xCC;
        func_8007506C(&r, D_80193268 + D_80193258);
        func_80074DC0(0);
        if (*(short *)(D_801D11C4 + 0x74) > 0) {
            RECT r2;

            r2 = *(RECT *)(D_801D11C4 + 0x70);
            r2.x = (r2.x * 3) >> 1;
            if (D_801D11C8 == 0) {
                r2.y += 0xF0;
            }
            r2.w = (r2.w * 3) >> 1;
            func_8007506C(&r2, D_801D11C4 + 0x8080);
        }
        func_80073A44(0);
        func_80074A44(1);
        i++;
        func_80075424(D_801D11C4);
        func_800755F0(D_801D11C4 + 0x5C);
    } while (i < 2);
}
