/* ovl_0457 (PE.IMG credits/XA overlay, VRAM 0x80120D00)
 * func_80125744 — blob offset 0x4A44, 0x150 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the overlay VMA (docs/evidence/ovl_0457-func_80125744/REPORT.md).
 * XA playback setup: wait for drive ready, CdControl(0xE setmode 0xC8) and CdControl(0xD setfilter {1,0}) with CdSync-style polling (retry on 0 / error), install the stream callback func_80125980 and VSync hook func_80125A40, start the fade (func_801258EC(200, 0x1B)), wait ready, return 0. Written with goto retries like func_8012562C. */

extern int func_8007F72C();
extern int func_8007F778();
extern int func_8007EE84();
extern int func_8007F418();
extern void func_800824C8();
extern void func_80073D24();
extern void func_801258EC();
extern void func_80125980();
extern void func_80125A40();
int func_80125744(void)
{
    unsigned char mode[8];
    unsigned char filter[8];
    unsigned char result[8];
    int id;
    int r;
    mode[0] = 0xC8;
retry_mode:
    while (func_8007F72C() != 1 || func_8007F778() != 0) {
    }
    id = func_8007EE84(0xE, mode, 0, -1);
    if (id == 0) goto retry_mode;
    for (;;) {
        r = func_8007F418(id, result);
        if (r == 2) break;
        if (r != 0) goto retry_mode;
    }
    filter[0] = 1;
    filter[1] = 0;
retry_filter:
    while (func_8007F72C() != 1 || func_8007F778() != 0) {
    }
    id = func_8007EE84(0xD, filter, 0, -1);
    if (id == 0) goto retry_filter;
    for (;;) {
        r = func_8007F418(id, result);
        if (r == 2) break;
        if (r != 0) goto retry_filter;
    }
    func_800824C8(func_80125980);
    func_80073D24(func_80125A40);
    func_801258EC(200, 0x1B);
    while (func_8007F72C() != 1 || func_8007F778() != 0) {
    }
    return 0;
}
