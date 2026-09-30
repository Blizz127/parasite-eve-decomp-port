/* ovl_03C5 (PE.IMG movie-controller overlay, VRAM 0x80120D00)
 * func_80121270 — blob offset 0x570, 0x264 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the overlay VMA (docs/evidence/ovl_03C5-func_80121270/REPORT.md).
 * MDEC frame fetch: wait for the next stream frame (2000 tries), fade the XA volume near the
 * end, resize/clear on a size change, and publish width/height into `out`; returns the frame address. */

typedef struct { short x, y, w, h; } RECT;
typedef struct {
    unsigned short id, type;
    unsigned short secCount, nSectors;
    unsigned int frameCount;
    unsigned int frameSize;
    unsigned short width, height;
} StHeader;
typedef struct { unsigned char pad[8]; short last; } MovieInfo;
extern int func_8007F72C();
extern int func_8007F7A8();
extern void func_800719E4() __attribute__((noreturn));
extern int func_8007C484();
extern void func_800870F0();
extern void func_80074F44();
extern unsigned short D_800B0DD4;
extern MovieInfo *D_801227E4;
extern short D_801227E8;
extern unsigned char D_800B0DBE;
extern unsigned char D_801223F5;
extern short D_80122418[2];
extern signed char D_800B0DBB;
void *func_80121270(short *out)
{
    int n = 2000;
    void *addr;
    StHeader *hdr;
    int v;
    int t;
    volatile short *p;

    if (func_8007F72C() == 1 && func_8007F7A8() != D_800B0DD4) {
        func_800719E4(1);
    }
    while (func_8007C484(&addr, &hdr) != 0) {
        if (--n == 0) {
            return 0;
        }
    }
    if (hdr->frameCount >= D_801227E4->last - 16) {
        t = hdr->frameCount + 16;
        v = t - D_801227E4->last;
        v = 14 - v;
        if (v < 0) {
            v = 0;
        }
        func_800870F0(D_800B0DBE * v / 14);
    }
    if (hdr->frameCount < D_801227E8 || hdr->frameCount >= D_801227E4->last) {
        D_801223F5 = 1;
    }
    D_801227E8 = hdr->frameCount;
    if (D_80122418[0] != hdr->width || D_80122418[1] != hdr->height) {
        RECT r;

        if (D_800B0DBB != 0) {
            r.x = 0;
            r.y = 0;
            r.w = 480;
            r.h = 480;
        } else {
            r.w = 320;
            r.x = 0;
            r.y = 0;
            r.h = 480;
        }
        func_80074F44(&r, 0, 0, 0);
        D_80122418[0] = hdr->width;
        D_80122418[1] = hdr->height;
    }
    if (D_800B0DBB != 0) {
        out[13] = out[17] = D_80122418[0] * 3 / 2;
    } else {
        out[13] = out[17] = D_80122418[0];
    }
    p = &D_80122418[1];
    out[14] = out[18] = *p;
    out[23] = *p;
    return addr;
}
