/* ovl_0457 (PE.IMG credits/XA overlay, VRAM 0x80120D00)
 * func_801235DC — blob offset 0x28DC, 0x374 bytes. Profile ovl_0457_dispatch_80122FA4 (local: -O2 -G0,
 * three-word symbol store + MASPSX_DISPATCH_FOLD=jtbl_80122FA4); LINK_EXACT at the overlay VMA
 * (docs/evidence/ovl_0457-func_801235DC/REPORT.md). Credits main loop: choose the op table, start the
 * stream, then run the 5-state fade machine until D_80172CE0 is set. */

typedef struct {
    void (*init)();
    void (*update)();
    void (*update2)();
    int padC;
    void (*fade)();
    void (*draw)();
    void (*end)();
} CreditOps;
extern unsigned int D_8009D1A0;
extern unsigned int D_8009D1F4;
extern unsigned int D_8009D26C;
extern CreditOps *D_80172CD0;
extern CreditOps D_80125B68;
extern CreditOps D_80125B4C;
extern int D_80125B54;
extern int D_80172CD4;
extern int D_80172CE0;
extern int D_80172CE4;
extern void func_8012562C();
extern void func_80125744();
extern void func_800870F0();
extern void func_80125894();
extern int func_80123950();
extern void func_8003EB04();
extern void func_801255A4();
void func_801235DC(void)
{
    unsigned int state;
    int fade;

    if (D_8009D1A0 & 0x10000) {
        D_80172CD0 = &D_80125B68;
    } else {
        D_80172CD0 = &D_80125B4C;
        if (!(D_8009D1A0 & 0x20000)) {
            D_80125B54 = 0;
        }
    }
    D_80172CD0->init();
    if (!(D_8009D1A0 & 0x10000)) {
        func_8012562C();
        func_80125744();
        if (D_8009D1A0 & 0x20000) {
            func_800870F0(0x90);
            func_80125894(1);
        } else {
            func_800870F0(0xDA);
        }
    }
    state = 0;
    fade = 0;
    D_80172CD4 = 0x1A4;
    D_80172CE4 = 0;
    D_80172CE0 = 0;
    do {
        if (D_80172CD0->update != 0) {
            D_80172CD0->update();
        }
        if (D_80172CD0->update2 != 0) {
            D_80172CD0->update2();
        }
        switch (state) {
        case 0:
            if (func_80123950() != 0) {
                if (D_8009D1A0 & 0x20000) {
                    state = 1;
                } else {
                    state = 4;
                }
                fade = 0;
            }
            break;
        case 1:
            D_80172CD0->fade(fade & 0xFF);
            fade += 4;
            if (fade >= 0x100) {
                state = 2;
                fade = 0;
            }
            break;
        case 2:
            D_80172CD0->fade(0xFF);
            if (D_80172CE4 != 0 && (D_8009D26C & 0xF0000004)) {
                state = 3;
                fade = 0xFF;
            }
            break;
        case 3:
            D_80172CD0->fade(fade & 0xFF);
            fade -= 4;
            if (fade < 0) {
                state = 4;
            }
            break;
        case 4:
            if (D_8009D1A0 & 0x30000) {
                D_80172CE0 = 1;
            } else {
                D_80172CE0 = D_80172CE4;
            }
            break;
        }
        func_8003EB04();
        if (D_8009D1A0 & 0x10000) {
            if (D_8009D1F4 & 0x4000000) {
                func_80125894(0);
            }
            if (D_8009D1F4 & 0x8000000) {
                func_80125894(1);
            }
        }
        D_80172CD0->draw();
        D_80172CD4++;
    } while (D_80172CE0 == 0);
    func_801255A4();
    D_80172CD0->end();
    D_8009D1A0 &= ~0x10000;
}
