/* room_m0146i (PE.IMG room m0146i chunk 2, VRAM 0x8018EFE8)
 * func_8018F218 — blob offset 0x230, 0x244 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0141i func_8018F224; C re-targeted by symbol address
 * (docs/evidence/room_m0146i-ports-2026-09-23/REPORT.md). */

#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))

typedef struct { int w[8]; } S32;
extern unsigned char D_80192668, D_80192669, D_8019266A, D_8019266C, D_8019266D, D_8019266E;
extern unsigned char D_80192678, D_80192679, D_8019267A, D_8019267C, D_8019267D, D_8019267E;
extern unsigned char D_801926A8, D_801926A9, D_801926AA, D_801926AC, D_801926AD, D_801926AE;
extern unsigned char D_801926B8, D_801926B9, D_801926BA, D_801926BC, D_801926BD, D_801926BE;
extern short D_80192670, D_80192672, D_80192680, D_80192682, D_801926B0, D_801926B2, D_801926C0, D_801926C2;
extern short D_800942EC;
extern void func_800C2B40();
extern int *func_800C2B28();
extern int func_8006DC18();
extern void func_800C66C8();

void func_8018F218(void *a0, int a1, void *a2)
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
    D_8019266C = 0x42;
    D_8019266D = 3;
    D_80192670 = -0x32;
    D_80192672 = 0x80;
    D_80192668 = 0x80;
    D_801926BC = 0x20;
    D_801926BD = 1;
    D_801926C0 = -0x33;
    D_801926AC = 0x40;
    D_801926AD = 2;
    D_801926B0 = -0x29;
    D_8019267C = 0x47;
    D_801926C2 = 0x80;
    D_801926B2 = 0x80;
    D_80192682 = 0x80;
    D_80192680 = -0x29;
    D_80192669 = 0x80;
    D_8019266A = 0x80;
    D_8019266E = 0;
    D_801926B8 = 0x80;
    D_801926B9 = 0x80;
    D_801926BA = 0x80;
    D_801926BE = 0;
    D_801926A8 = 0x80;
    D_801926A9 = 0x80;
    D_801926AA = 0x80;
    D_801926AE = 0;
    D_8019267D = 5;
    D_80192678 = 0x80;
    D_80192679 = 0x80;
    D_8019267A = 0x80;
    D_8019267E = 0;
    D_800942EC = 0;
    func_800C66C8(a0, 0x587, (char *)a2 + 4);
}
