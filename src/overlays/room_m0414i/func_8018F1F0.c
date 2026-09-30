/* room_m0414i (PE.IMG room m0414i chunk 2, VRAM 0x8018EFE8)
 * func_8018F1F0 — blob offset 0x208, 0x244 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0141i func_8018F224; C re-targeted by symbol address
 * (docs/evidence/room_m0414i-ports-2026-09-23/REPORT.md). */

#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))

typedef struct { int w[8]; } S32;
extern unsigned char D_80192160, D_80192161, D_80192162, D_80192164, D_80192165, D_80192166;
extern unsigned char D_80192170, D_80192171, D_80192172, D_80192174, D_80192175, D_80192176;
extern unsigned char D_80192180, D_80192181, D_80192182, D_80192184, D_80192185, D_80192186;
extern unsigned char D_80192190, D_80192191, D_80192192, D_80192194, D_80192195, D_80192196;
extern short D_80192168, D_8019216A, D_80192178, D_8019217A, D_80192188, D_8019218A, D_80192198, D_8019219A;
extern short D_800942EC;
extern void func_800C2B40();
extern int *func_800C2B28();
extern int func_8006DC18();
extern void func_800C66C8();

void func_8018F1F0(void *a0, int a1, void *a2)
{
    void *x;

    func_800C2B40(a2);
    H(a2, 0x2A) = 0;
    H(a2, 0x2C) = 0;
    H(a2, 0x28) = *func_800C2B28(5);
    H(a2, 0x2E) = *func_800C2B28(4);
    x = P(a0, 8);
    P(a2, 0) = x;
    *(S32 *)((char *)a2 + 4) = *(S32 *)P(x, 0x238);
    W(a2, 0x18) = *func_800C2B28(1);
    W(a2, 0x1C) = *func_800C2B28(2);
    W(a2, 0x20) = *func_800C2B28(3);
    W(a2, 0x24) = func_8006DC18(0xA6);
    D_80192164 = 0x42;
    D_80192165 = 3;
    D_80192168 = -0x32;
    D_8019216A = 0x80;
    D_80192160 = 0x80;
    D_80192194 = 0x20;
    D_80192195 = 1;
    D_80192198 = -0x33;
    D_80192184 = 0x40;
    D_80192185 = 2;
    D_80192188 = -0x29;
    D_80192174 = 0x47;
    D_8019219A = 0x80;
    D_8019218A = 0x80;
    D_8019217A = 0x80;
    D_80192178 = -0x29;
    D_80192161 = 0x80;
    D_80192162 = 0x80;
    D_80192166 = 0;
    D_80192190 = 0x80;
    D_80192191 = 0x80;
    D_80192192 = 0x80;
    D_80192196 = 0;
    D_80192180 = 0x80;
    D_80192181 = 0x80;
    D_80192182 = 0x80;
    D_80192186 = 0;
    D_80192175 = 5;
    D_80192170 = 0x80;
    D_80192171 = 0x80;
    D_80192172 = 0x80;
    D_80192176 = 0;
    D_800942EC = 0;
    func_800C66C8(a0, 0x587, (char *)a2 + 4);
}
