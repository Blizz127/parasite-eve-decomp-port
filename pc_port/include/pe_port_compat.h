/* Phase 6D-S — PE_PORT compatibility layer.
 * All BOOTSTRAP_RET stubs now use centralized Bootstrap_* policy.
 * No (int)(uintptr_t) casts remain on the first-clear path. */
#ifndef PE_PORT_COMPAT_H
#define PE_PORT_COMPAT_H

#include "psx_compat.h"

int func_801216C4(int mode, pe_addr_t buffers);
void func_801223A8(int mode);
void func_80121A00(void);
void func_80121004(int buffer, int wide);
int func_80121270(pe_addr_t state);
void func_801214D4(void);
void func_80191DC8(void);
int func_80122040(void);
int func_80192934(void);
void func_80122354(void);
int func_80121C04(int id);
void func_8006E60C(void);

/* ── Boot Rung globals (guest-address backed) ─────────────────────── */
/* D_800B0CD8..D_800B0CEB, D_800B0DD4, D_80094488/D_8009448C are guest-RAM
 * lvalue macros defined in psx_compat.h — no externs here. */
/* D_8009D250 is also guest-backed: frame updates, RNG/menu readers and
 * captured original-code probes must observe the same counter word. */

/* ── Arena pointers — guest-RAM lvalue macros (Phase 6E-B16) ────────
 * The 19-slot retail arena pointer table lives INSIDE the D_800B0CD8
 * struct region in guest RAM (0x800B0E24..0x800B0E6C); retail func_8006A8D4
 * stores to it with sw and retail readers (func_8006A9E4: lw 0x130(s3),
 * lw 0x194(s3)) load from guest RAM.  6D-S modeled these as host scalar
 * globals, splitting them from the guest struct — func_8006A9E4 then read
 * zeros.  They are now guest-RAM lvalue macros like D_800B0CD8. */
#define D_800B0E24  PE_GUEST_U32(0x800B0E24u)
#define D_800B0E28  PE_GUEST_U32(0x800B0E28u)
#define D_800B0E2C  PE_GUEST_U32(0x800B0E2Cu)
#define D_800B0E30  PE_GUEST_U32(0x800B0E30u)
#define D_800B0E34  PE_GUEST_U32(0x800B0E34u)
#define D_800B0E38  PE_GUEST_U32(0x800B0E38u)
#define D_800B0E3C  PE_GUEST_U32(0x800B0E3Cu)
#define D_800B0E40  PE_GUEST_U32(0x800B0E40u)
#define D_800B0E44  PE_GUEST_U32(0x800B0E44u)
#define D_800B0E48  PE_GUEST_U32(0x800B0E48u)
#define D_800B0E4C  PE_GUEST_U32(0x800B0E4Cu)
#define D_800B0E50  PE_GUEST_U32(0x800B0E50u)
#define D_800B0E54  PE_GUEST_U32(0x800B0E54u)
#define D_800B0E58  PE_GUEST_U32(0x800B0E58u)
#define D_800B0E5C  PE_GUEST_U32(0x800B0E5Cu)
#define D_800B0E60  PE_GUEST_U32(0x800B0E60u)
#define D_800B0E64  PE_GUEST_U32(0x800B0E64u)
#define D_800B0E68  PE_GUEST_U32(0x800B0E68u)
#define D_800B0E6C  PE_GUEST_U32(0x800B0E6Cu)

/* ── func_8003E610 callees ─────────────────────────────────────────── */
/* All ten callees are now REAL implementations (Phase 6E-A batch 1):
 * func_80073C94/func_80072714/func_80072724  — pe_libetc.c
 * func_80074924/749D8/74A44/74BB8            — pe_libgpu.c
 * func_8007D054                              — pe_libsnd.c
 * func_80077F7C/func_80079004/func_80079024  — pe_gte.c
 * func_800409B4                              — pe_libcard.c
 * func_800844E4/func_80082534                — pe_save.c
 * func_8007EC14/func_8007ED58/func_8007F72C/func_8007F778/
 * func_8007FBF0/func_80080CC8/func_8007F7A8  — pe_libcd.c
 * func_8003E754/func_8003E944                — game/boot/*.c          */
#include "pe_sdk.h"

/* ── func_8006A5BC callees ─────────────────────────────────────────── */
/* All eight callees are now real (Phase 6E-A batches 1-2):
 * func_80085644/func_80086FF8/func_80087024/func_8008682C — pe_stream.c
 * func_8007ED58/func_8007F72C/func_8007F7A8                     — pe_libcd.c */

/* ── func_8003E680 callees ─────────────────────────────────────────── */
/* func_80070D10/func_80070D6C are now REAL translations (Phase 6E-B1/B2):
 * game/boot/func_80070D10_port.c — lagged-Fibonacci RNG table init
 * game/boot/func_80070D6C_port.c — RNG advance (verbatim |= wrap)
 * game/boot/func_80070DD0_port.c — handwritten ranged-random wrapper
 * func_8003E974 is now a REAL translation (Phase 6E-B3):
 * game/boot/func_8003E974_port.c — bit-table init + 20 registrations;
 * func_8003EAC8 is now a REAL translation too (Phase 6E-B4):
 * game/boot/func_8003EAC8_port.c — GTE LZCS/LZCR-indexed table writer.
 * func_80036DC8 is now a REAL translation too (Phase 6E-B5):
 * game/boot/func_80036DC8_port.c — timer-record init dispatcher. */
extern void func_8003E91C(void);
extern void func_80036F7C(void);

/* func_80073D24 is now a REAL SDK implementation (Phase 6E-B6):
 * pe_libetc.c — VBlank callback slot-4 setter with previous-handler
 * return, guest-table backed (pe_callback.h).
 * func_800371A4 is now a REAL translation (Phase 6E-B7):
 * game/boot/func_800371A4_port.c — 3-word $gp-relative byte setter.
 * func_80029388 is now a REAL translation too (Phase 6E-B8):
 * game/boot/func_80029388_port.c — slot-table clear + default-record
 * init (with leaves func_8002F658 and func_80020EFC).
 * func_8005BCA8 is now a REAL translation too (Phase 6E-B9):
 * game/boot/func_8005BCA8_port.c — empty jr/nop stub (2 retail words).
 * func_80068D28 is now a REAL translation too (Phase 6E-B10):
 * game/boot/func_80068D28_port.c — double-buffered display-record
 * data initializer (63 retail words).
 * func_800124F8 is now a REAL translation too (Phase 6E-B11):
 * game/boot/func_800124F8_port.c — subsystem table/array/scalar clear
 * (31 retail words).
 * func_8001A890 is now a REAL translation too (Phase 6E-B12):
 * game/boot/func_8001A890_port.c — subsystem scalar/array clear
 * (34 retail words).
 * func_80034F10 is now a REAL translation too (Phase 6E-B13):
 * game/boot/func_80034F10_port.c — subsystem table clear +
 * D_800B0CD8 flag-bit clear (45 retail words).
 * func_8006536C is now a REAL translation too (Phase 6E-B14):
 * game/boot/func_8006536C_port.c — 28x3-word record-table clear +
 * index byte clear (19 retail words).
 * func_800653B8 is now a REAL translation too (PE-CH1):
 * game/boot/func_800653B8_port.c — 18-word mailbox append into
 * D_800A3180[count++] (12-byte record; no clamp at 28).
 * func_80017764 is now a REAL translation too (PE-CH1):
 * game/boot/func_80017764_port.c — opcode 0x1C send; jal 653B8
 * with extra 0; return 1.
 * func_80065400 is now a REAL translation too (PE-CH1):
 * game/boot/func_80065400_port.c — 117-word mailbox drain; jal
 * func_80012700.
 * func_80012700 is now a REAL translation too (PE-CH1):
 * game/boot/func_80012700_port.c — 29-word freelist pop + init.
 * func_800177AC is now a REAL translation too (PE-CH1):
 * game/boot/func_800177AC_port.c — 7-word opcode 0x1F poll;
 * copies current task+0x14 through *arg0; return 1; no ACK.
 * func_80019154 is now a REAL translation too (PE-CH1):
 * game/boot/func_80019154_port.c — 7-word opcode 0x94 mode
 * read; copies D_8009D28C through *arg0; return 1.
 * func_8002F7D8 is now a REAL translation too (PE-CH1):
 * game/boot/func_8002F7D8_port.c — 102-word opcode 0x6F slot
 * alloc; claims first free SlotRecord; jal 1A680 unresolved.
 * func_8002FA10 is now a REAL translation too (PE-CH1):
 * game/boot/func_8002FA10_port.c — 37-word opcode 0x70
 * formation write through *actor.
 * func_8002FAA4 is now a REAL translation too (PE-CH1):
 * game/boot/func_8002FAA4_port.c — 13-word opcode 0xB7
 * formation write through *actor (0x70 subset).
 * func_8002FF78 is now a REAL translation too (PE-CH1):
 * game/boot/func_8002FF78_port.c — 101-word opcode 0x5A
 * Aya tagged setter through *D_8009D254.
 * func_80030220 is now a REAL translation too (PE-CH1):
 * game/boot/func_80030220_port.c — 197-word opcode 0x5A
 * slot tagged setter via D_80010C90[tag-40].
 * func_8002FE78 is now a REAL translation too (PE-BTL46):
 * game/boot/func_8002FE78_port.c — 64-word opcode 0x59
 * Aya tagged reader through *(*D_8009D254).
 * func_8003010C is now a REAL translation too (PE-BTL46):
 * game/boot/func_8003010C_port.c — 69-word opcode 0x59
 * slot tagged reader via D_80010B28[tag-41].
 * func_800299CC_consume_cut is now a REAL translation too (PE-CH1):
 * game/boot/func_800299CC_port.c — 16-word named cut of the
 * battle tick consume edge; D_8009D28C 6→0 and sb gp+0x10C=6.
 * func_8002CF24_mode7_cut is now a REAL translation too (PE-BTL2):
 * game/boot/func_8002CF24_port.c — 2-word inlined D_8009D28C=7
 * store (addiu 7 / sw gp+0x51C).
 * func_800293F4_hp_cut is now a REAL translation too (PE-BTL2):
 * game/boot/func_800293F4_port.c — 21-word HP clamp/copy named cut
 * of func_800293F4 (record+0x0C/+0x0E/+0x1C). */

/* ── REAL translated Boot Rung functions ────────────────────────────── */
extern void func_8003E610(void);
extern void func_8003E680(void);
extern int func_8003E974(void);
extern void func_8003EAC8(int, int);
extern void func_80036DC8(void);
extern void func_80036DF8(void);
extern void func_80036E34(void);
extern void func_80036E58(void);
extern void func_800371A4(int a0);
extern void func_80029388(void);
extern void func_8002F658(void);
extern void func_80020EFC(void);
extern void func_80071A64(uint32_t seed);
extern void func_8005BCA8(void);
extern int func_80068D28(void);
extern void func_800124F8(void);
extern void func_8001A890(void);
extern void func_80034F10(void);
extern void func_80034FC4(void);
extern void func_8006536C(void);
extern void func_800653B8(unsigned int payload, unsigned int dest_id, unsigned int dest_type, unsigned int sender, unsigned int extra);
extern int  func_80065954(unsigned int index, unsigned int enabled);
extern int  func_800659C8(unsigned int index, unsigned int value);
/* PE-CH2 opcode-0x9A / 0x71 callees: 16-byte slot fill and 56-byte pan clamp. */
extern int  func_80065AD4(unsigned int index, unsigned int value, unsigned int count);
extern int  func_800671C8(pe_addr_t rec, int32_t x, int32_t y, int32_t page);
/* SEW3 — game/boot/func_800659F8_port.c */
extern int  func_800659F8(uint32_t index, uint32_t value);
extern int  func_80065A60(uint32_t index, uint32_t entry, uint32_t value);
extern int  func_80065A9C(uint32_t index, uint32_t bits);
extern int  func_80067678(uint32_t index, uint32_t on);
extern int  func_800676CC(uint32_t index, uint32_t on, uint32_t a2, uint32_t a3);
extern int  func_80067730(uint32_t index, uint32_t on, uint32_t a2, uint32_t a3);
extern void func_8003746C(short a0);
extern void func_80037454(int a0, int a1, int a2, int a3);
extern void func_800375B4(void);
extern void func_800375C4(void);
extern int  func_8006F820(uint32_t index, uint32_t mode, uint32_t value_or_out);
extern void func_8001ACE0(pe_addr_t actor, uint32_t index);
/* SEW8 — game/boot/func_80013988_port.c */
extern int  func_8001787C(pe_addr_t args);
extern int  func_80014FD8(pe_addr_t args);
extern int  func_800155FC(pe_addr_t args);
extern int func_80019B08(void);
extern int  func_80013988(pe_addr_t args);
extern int  func_80013E84(pe_addr_t args);
extern int  func_800665A0(pe_addr_t pos, int duration, int mode);
extern int  func_80017CE8(pe_addr_t args);
extern void func_8003C2E0(pe_addr_t dest, int level, pe_addr_t view);
extern int  func_80066800(unsigned int index);
extern int func_800661EC(short a0, short a1, short a2, int a3);
extern int  func_80017C54(pe_addr_t args);
extern int  func_80017764(pe_addr_t args);
extern void func_80065400(void);
extern int func_80068E24(void);
extern void func_8003F3C4(void);
extern pe_addr_t func_80012700(pe_addr_t entry, unsigned int a1);
void func_80012774(void);
void func_80035F54(pe_addr_t actor);
void func_80036448(void);
void func_800360B4(void);
void func_8003601C(void);
void func_8001D268(pe_addr_t actor,int32_t own_part,pe_addr_t other,int32_t part);
extern int  func_80017BB4_btl1_cut(pe_addr_t args);
extern int func_800177AC(pe_addr_t pe_arg0);
extern int func_80019154(pe_addr_t pe_arg0);
extern void func_8002F7D8(pe_addr_t actor);
extern void func_8002FA10(pe_addr_t actor, unsigned int index,
                          unsigned int a2, unsigned int a3,
                          unsigned int b3, unsigned int half_c,
                          int b7c, int b7d, int b7e, int b7f,
                          unsigned int b_e, unsigned int b_f);
extern void func_8002FAA4(pe_addr_t actor, unsigned int index,
                          unsigned int a2, unsigned int a3,
                          unsigned int b3, unsigned int half_c);
extern void func_8002FAD8(pe_addr_t actor, unsigned int index, uint32_t first, uint32_t last);
extern int func_800198C4(pe_addr_t args);
extern void func_8002FF78(unsigned int tag, unsigned int value);
extern int  func_8002FE78(unsigned int tag);
extern int  func_8003010C(pe_addr_t actor, unsigned int tag);
extern void func_80030220(pe_addr_t actor, unsigned int tag,
                          unsigned int value);
extern void func_800299CC_consume_cut(void);
extern void func_800299CC_after_consume_cut(void);
extern int func_8005C498(pe_addr_t result);
void func_8004DCA4(uint32_t mode);
int func_80016F10(pe_addr_t args);
void func_80046334(void);
void func_8004F464(void);
void func_8005C488(void);
void func_80042B6C(void);
void func_800339A0(uint32_t style);
void func_80042D40(void);
void func_80042F44(void);
void func_8005E788(int32_t wait);
void PE_FieldMenuFrame(void);
unsigned char func_80033A20(void);
int func_8005B89C(void);
int func_8005257C(void);
void func_8004C594(void);
void func_800439D8(void);
void func_800438EC(void);
void func_8005C174(int32_t battle);
extern void func_800512AC(int cmd, pe_addr_t src);
void func_80051244(void);
void func_8005D970(void);
void func_800218D8(void);
void func_80021AF8(void);
void func_800209F0(void);
void func_800254BC(int32_t command);
extern void func_800512AC_cmd10_cut(void);
extern int func_80026824(int a0);
extern void func_80026FD0(void);
extern void func_80062CB8(int a0);
extern int func_80062CC4(void);
pe_addr_t func_8005DA8C(unsigned int index);
pe_addr_t func_8005DAB4(unsigned int index);
pe_addr_t func_80062A34(uint32_t kind,uint32_t id);
pe_addr_t func_800631DC(void);
pe_addr_t func_80062D2C(uint32_t id,pe_addr_t owner,pe_addr_t parent,uint32_t modal);
pe_addr_t func_8006322C(uint32_t id,pe_addr_t owner,pe_addr_t parent);
void func_80064AC0(pe_addr_t node);
void func_80064D08(pe_addr_t list);
void func_80064A54(pe_addr_t pe_a0);
void func_8006269C(pe_addr_t node);
void func_80062F3C(uint32_t id);
void func_80064E90(pe_addr_t node);
void func_800647D0(pe_addr_t node,int32_t items);
/* Field menu page constructor reached from func_80043DA4 command 5. */
void func_8004AD9C(pe_addr_t owner);
/* Tier-2 wrapper the page stores into the list callback slot +0x30; it enters
 * the generic list renderer func_800638D8 with the per-cell draw 0x80050C50. */
void func_8004FF30(int slot);
/* Field-menu input tree the 4AD9C page installs into window+0x2C (0x8004AE1C)
 * and its sub-page constructors / handlers / draws (jtbl_80011034 cases 0..5). */
int func_8004AE1C(int page, unsigned int event);
void func_8004AF3C(pe_addr_t owner);
int func_8004AFA4(int a0, int a1);
void func_8004B03C(pe_addr_t owner);
int func_8004B0A4(int a0, int a1);
void func_8004B13C(pe_addr_t owner);
void func_8004B584(pe_addr_t owner);
/* Tier-2 draw wrappers the sub-page constructors store in list+0x30; each
 * enters func_800638D8 with its own per-cell draw callback. */
void func_8004FF58(int slot);
void func_8004FF80(int slot);
void func_8004B534(int slot);
void func_8004B55C(int slot);
void func_80050C70(int arg0);
void func_80050CB4(int arg0);
/* Field-menu Equipment sub-page (0x8004AE1C case 2): window draw +0x30,
 * window input +0x2C, and the list1 per-cell draw reached through
 * func_8004B534 -> func_800638D8.  Also the modal sub-page (case 3) handler. */
void func_8004B214(int a0);
int func_8004B394(pe_addr_t node, uint32_t event);
int func_8004B650(int a0, int a1);
void func_80050438(int a0);
/* Field-menu close page: the 0x8004AE1C case-4/5 arm and its heal commit. */
void func_8005D994(int32_t index);
void func_8005247C(void);
/* Packed-colour lane helpers the equipment page reads/writes (D_8009D14C). */
int func_8005E54C(void);
int func_800614A0(void);
void func_800614AC(int a0);  /* result stored to D_8009D150; retail $v0 unread (port3 A1) */
/* Field-menu modal sub-page (0x8004AE1C case 3) window draw and its two
 * original font/sprite leaves. */
void func_8004B5DC(pe_addr_t node);
void func_8005FCAC(int32_t value);
void func_8005ED18(uint32_t icon, uint32_t mode);
void func_800527C0(void);
extern void func_8004B90C(void);
extern void func_8004B70C(uint32_t a0, uint32_t a1, pe_addr_t a2);
extern int func_8004BB80(pe_addr_t obj, uint32_t a1);
extern void func_8005E30C(void);
extern void func_800299CC_mode_switch_cut(void);
extern void func_8002A7F8_join_cut(void);
extern void func_800299CC_damage_entry_cut(void);
extern void func_8002BC90_mode6_cut(void);
extern void func_8002A7F8_mode3_cut(void);
extern int func_8002AA98(void);
extern void func_8002B29C(void);
extern void func_8002B0E8(void);
void func_8002B94C(void);
void func_8002F0B0(void);
extern void func_8002F300_mode2_cut(void);
extern void func_800292EC_victory_ready_cut(void);
extern void func_80027D14(pe_addr_t actor);
extern void func_80028E94(pe_addr_t actor);
extern int func_80027A08(pe_addr_t actor);
extern void func_80036254(pe_addr_t actor);
extern void func_8002D1F0(void);
extern void func_8002BC90(void);
extern void func_80032B0C(unsigned int mode, pe_addr_t amount);
extern void func_8002DC58(void);
extern void func_800295E4(void);
extern void func_8002F9CC(void);
extern int func_80021054(void);
extern void func_80021DE0(void);
extern void func_80022210(void);
extern int8_t func_800255E4(void);
extern int func_8006346C(pe_addr_t pe_record);
extern void func_8005FDF0(int value);
void func_8005FF28(int32_t value);
void func_8005E988(int a0, int a1);
void func_8004551C(pe_addr_t item);
void func_80045670(pe_addr_t old_item,pe_addr_t new_item);
void func_80045A98(pe_addr_t window);
unsigned int func_800631C0(pe_addr_t pe_arg0);
int func_80045D0C(pe_addr_t window,uint32_t event);
int func_8004620C(pe_addr_t window,uint32_t event);
int func_800466C0(pe_addr_t window,uint32_t event);
void func_80046574(pe_addr_t window,uint32_t confirmed);
void func_80059534(int32_t selected);
int32_t func_8005968C(int32_t selected);
int32_t func_80054520(uint32_t mask);
void func_80063198(pe_addr_t pe_arg0);
void func_80050D18(void);
void func_8004FFD0(int slot);
void func_8004F978(int slot);
void func_800509E0(uint32_t unused);
void func_8004FA10(pe_addr_t list);
void func_8004FB48(pe_addr_t list);
void func_800430A0(int32_t row);
void func_800542A0(uint32_t mask);
void func_800543CC(uint32_t mask,int32_t excluded);
void func_80059EC8(uint32_t slot,int32_t index);
void func_80064C20(pe_addr_t pe_arg0);
void func_80064B74(pe_addr_t list,int32_t index);
void func_80045EE4(pe_addr_t parent);
void func_8004542C(pe_addr_t parent);
void func_80046378(pe_addr_t parent,uint32_t focus);
void func_8004F9A0(pe_addr_t list);
void func_80050AD8(int32_t index);
void func_80046DFC(pe_addr_t owner,uint32_t alternate);
void func_80046EAC(uint32_t arranged);
void func_80047040(uint32_t index);
void func_800471BC(int slot);
int func_800471E4(pe_addr_t window,uint32_t event);
void func_8004732C(int slot);
int func_80047354(pe_addr_t window,uint32_t event);
void func_800473E4(pe_addr_t owner,uint32_t selection);
void func_800474A8(int slot);
int func_800474D0(pe_addr_t window,uint32_t event);
void func_80050280(uint32_t index);
void func_80050308(uint32_t index);
int32_t func_8005AFFC(pe_addr_t first,pe_addr_t second);
int32_t func_8005B124(pe_addr_t first,pe_addr_t second);
void func_8005B248(void);
void func_8005B3A4(void);
void func_8005B500(uint32_t kind,uint32_t selection);
void func_8005B71C(uint32_t selection);
void func_8005B7D0(uint32_t kind,uint32_t selection);
void func_800723A4(pe_addr_t base,uint32_t count,uint32_t size,pe_addr_t compare);
void func_800724F4(pe_addr_t pe_a, pe_addr_t pe_b, unsigned int n);
void func_80046ABC(pe_addr_t owner);
extern int func_80046B58(int a0, int a1);
extern void func_80046C20(void);
extern void func_80046DBC(int a0, int a1);
extern void func_80057B70(int32_t ability);
extern int func_8004FC3C(int a0);
extern void func_80050B48(uint32_t index);
extern void func_80061044(int32_t value,int32_t maximum);
extern void func_8004FC80(pe_addr_t list);
extern int32_t PE_MenuPEValues(int32_t *maximum);
extern int32_t func_800515F8(pe_addr_t maximum);
extern pe_addr_t func_8005DC10(void);
extern int func_800524D0(void);
extern int32_t func_800579D4(int32_t ability,int32_t available);
extern int func_8004324C(int32_t ability);
extern void func_80022394(void);
extern int func_80024A3C(void);
extern void func_80024250(int index, pe_addr_t actor);
extern void func_80021F38(void);
extern int func_8002312C(pe_addr_t slot);
extern void func_80023008(void);
extern void func_800236E8(void);
extern void func_80021278(pe_addr_t actor, pe_addr_t result, int32_t previous);
extern int func_80066268(void);
extern pe_addr_t func_8005DC9C(uint32_t index);
extern pe_addr_t func_80054A88(int32_t item, int32_t kind);
extern void func_80028574(pe_addr_t actor);
extern void func_8002F970(pe_addr_t p);
extern int func_80053E6C(int id);
extern void func_8006A25C(void);
extern void func_80033A2C(void);
extern int func_80019D24(void);
extern int func_80019DB8(pe_addr_t args);
extern int func_80019DF4(pe_addr_t args);
extern int func_80069594(void);
extern int func_8006F8EC(unsigned int index);
extern int func_800D4704(pe_addr_t slot);
extern int func_800D413C(pe_addr_t slot);
extern int func_800D401C(int32_t index);
extern int32_t func_800D3FD8(void);
extern int func_8006F9F0(unsigned int index);
extern int func_8006DC18(uint32_t code);
extern int PE_EffectCallback(pe_addr_t fn, int32_t mode, pe_addr_t data, pe_addr_t extra);
extern void PE_EffectCallback_SetExtra1(uint32_t extra1);
extern int PE_MirrorOverlay(void);
extern int PE_MirrorInit(pe_addr_t slot);
extern int PE_MirrorCommand(pe_addr_t slot, uint32_t mode, uint32_t command, uint32_t x, uint32_t z);
extern void PE_MirrorBind(pe_addr_t dest, pe_addr_t actor, int32_t x0, int32_t z0, int32_t x1, int32_t z1);
extern void PE_MirrorPose(pe_addr_t dest);
extern void PE_MirrorPackets(pe_addr_t dest);
extern int PE_MirrorDraw(pe_addr_t slot);
extern int func_800D4C24(int32_t mode,pe_addr_t data);
extern int func_800D4928(int32_t mode,pe_addr_t data);
extern int func_800DF87C(int32_t mode,pe_addr_t data);
extern void PE_WeaponCallback(pe_addr_t fn,pe_addr_t slot,pe_addr_t rec,pe_addr_t data);
int PE_M34BossEffectOverlay(void);
void PE_M34BossProjectileInit(pe_addr_t data,const int32_t retained[6]);
int PE_M34BossEffectInit(pe_addr_t slot);
int PE_M0348iEffectOverlay(void);          /* M34 phase 2 room m0348i resident */
int PE_M0348iEffectInit(pe_addr_t slot);   /* room_m0348i func_8018F06C */
int PE_M0348iEffectCallback(pe_addr_t o);  /* room_m0348i func_8018F0C4 */
int PE_M0348iEffectUpdate(pe_addr_t o);    /* room_m0348i func_8018F128 */
int PE_M0348iEffectCleanup(pe_addr_t o);   /* room_m0348i func_8018F1B4 */
void PE_M0348iEffectF240(pe_addr_t slot, pe_addr_t data); /* room_m0348i func_8018F240 */
int PE_M0348iEffectChild(pe_addr_t fn, pe_addr_t slot, pe_addr_t rec, pe_addr_t data); /* m0348i VM callbacks */
int PE_M34BossEffectCommand(pe_addr_t slot,uint32_t mode,uint32_t index,uint32_t value,uint32_t extra0,uint32_t extra1);
void PE_M34StackField(int begin);
void PE_M34StackActor(pe_addr_t actor);
void PE_M34StackVm(int begin);
void PE_M34StackOpcode(pe_addr_t fn);
void PE_M34StackInvalidate(void);
void PE_M34StackClipSound(pe_addr_t body,pe_addr_t actor,uint32_t volume);
void PE_M34StackConstruct(uint32_t code);
void PE_M34StackCallback(pe_addr_t fn);
void PE_M34StackPointQuad(const int16_t vertices[4][4]);
int PE_M34StackRead(int32_t retained[6]);
int PE_M34BossEffectCleanup(pe_addr_t slot);
int PE_M34BossEffectDraw(pe_addr_t slot);
int PE_M34BossEffectUpdate(pe_addr_t slot);
int PE_M34BossEffectChild(pe_addr_t fn,pe_addr_t slot,pe_addr_t rec,pe_addr_t data);
extern int func_800C2414(pe_addr_t slot,pe_addr_t table);
extern int func_800C251C(pe_addr_t slot,pe_addr_t table);
extern int func_800C2758(pe_addr_t slot,pe_addr_t callbacks,pe_addr_t sizes);
extern pe_addr_t func_800C2B90(pe_addr_t slot,unsigned code,pe_addr_t sizes,pe_addr_t callbacks);
extern void func_800C2D0C(unsigned index,unsigned code,unsigned size);
extern int func_800C2DA0(int a0);
extern int func_800C2E08(void);
extern int func_800C9B68(int arg0);
extern int func_800CD8C8(int arg0);
extern int func_800C9B90(int a0);
extern int func_800CD8F0(int a0);
extern uint32_t func_800CE560(pe_addr_t pool, uint32_t size, int32_t count, pe_addr_t fn);
extern uint32_t func_800CE5AC(pe_addr_t out, uint32_t offset, uint32_t size, int32_t count, pe_addr_t fn);
extern pe_addr_t func_800CE610(pe_addr_t pool);
extern int func_800CE688(pe_addr_t pool);
extern int func_800CE78C(pe_addr_t pool);
extern void func_800CE870(pe_addr_t actor,int32_t mode,pe_addr_t out);
extern void func_800CE8F0(pe_addr_t actor,uint32_t joint,pe_addr_t local,pe_addr_t out);
extern void func_8002CF24_mode7_cut(void);
extern void func_800293F4_hp_cut(void);
extern void func_800209F0_cut(void);
extern void func_80030640_cut(void);
extern void func_800339A0_cut(unsigned int encounter);
extern void func_80029810_after_hp_cut(unsigned int encounter);
extern void func_80029810_remainder_cut(void);
extern void func_80029810_prologue_cut(void);
extern void func_80029810_cut(unsigned int encounter);
extern void func_8001A680_command_cut(pe_addr_t actor, unsigned int command);
extern void func_8001A784(pe_addr_t actor, unsigned int command);
extern void func_8006B4F8_bind_effects(pe_addr_t base);
extern void func_800716A4(pe_addr_t actor, pe_addr_t codep);
extern void func_8001A4AC(pe_addr_t actor);
extern int32_t func_8006A318(pe_addr_t actor);
extern void func_8006C140_type0_clip_bind(pe_addr_t package);
extern int func_800144FC_state3B_cut(void);
extern void func_800144FC_state3A_d1a0_cut(void);
extern int func_800144FC_state3A_cut(pe_addr_t arg0);
extern int func_800144FC_state0_cut(void);
extern int func_800144FC_state37_cut(void);
extern int func_800144FC_state38_cut(void);
extern int func_800144FC_state39_cut(void);
extern void func_80042EDC(void);
extern void func_80042F20(void);
extern void func_8006D24C(void);
extern int func_8006914C(int a0);
extern void func_8006D60C_after_6d078_cut(void);
extern int func_8006D60C_state3F_cut(void);
extern int func_8006D60C_state2F_cut(void);
extern int func_8006D60C_state30_cut(void);
extern int func_80086464(pe_addr_t a0);
extern void func_80086498(int a0);
extern int func_800864F8(int a0, int a1);
extern void func_80086770(int a0);
extern void func_80086C1C(int a0, int a1);
extern void func_80086C5C(int a0, int a1, int a2);
extern void func_80086CA4(int a0, int a1, int a2, int a3);
extern int func_8006DB48(int index, signed char value0, unsigned char value1, unsigned char value2);
extern int func_8006DB9C(int a0);
extern int func_8006DBE0(int a0);
extern void func_800866A4(unsigned int a0, unsigned int a1);
extern void func_80086948(int handle, int group, int duration, int volume);
extern int func_80080AC4(pe_addr_t gains);
extern unsigned int func_80085084(pe_addr_t pe_a0);
extern int32_t func_80086608(pe_addr_t sound, uint32_t key, uint32_t pan, uint32_t volume);
extern pe_addr_t func_8006E514(pe_addr_t package, uint32_t id);
extern int func_8006DF50(int a0, int a1, int a2, int a3, int a4);
extern int func_8006DFA8(pe_addr_t position, pe_addr_t pan, pe_addr_t volume);
extern uint32_t func_800299CC_ready_input(void);
extern void func_800299CC_player_animation(void);
extern void func_800306E0(pe_addr_t enemy);
extern void func_80025BD8(pe_addr_t enemy);
extern void func_8003CAEC(pe_addr_t model, uint32_t red, uint32_t green, uint32_t blue);
extern void func_800275CC(pe_addr_t targets, int8_t selected);
extern void func_80026FF8(pe_addr_t targets, int8_t selected, int8_t mode);
extern void func_80031D6C(int8_t height);
extern int32_t func_800328DC(pe_addr_t digits, int16_t x, int16_t y, int16_t number, int16_t tint);
extern unsigned int func_80056C14(unsigned int a0);
extern void func_8003205C(void);
extern void func_800323C8(int8_t mode);
extern void func_800325DC(void);
extern void func_800327D8(int16_t number, int32_t vertical);
extern void func_80031E68(void);
extern void func_80031760(pe_addr_t model, int8_t out_of_range);
extern void func_800314E4(pe_addr_t model, int8_t mode, int8_t out_of_range, int16_t number);
extern uint32_t func_8007041C(pe_addr_t geometry, pe_addr_t position, int32_t radius,
                             uint32_t angle, pe_addr_t ot, pe_addr_t output);
extern void func_800347B4(void);
extern void func_800258CC(int8_t mode);
extern void func_80026600(pe_addr_t target);
extern void func_80026CF0_attack_cut(void);
extern int8_t func_80025EE8_attack_cut(void);
void func_800299CC(void);   /* battle tick, func_800299CC_port.c (src/func_800299CC.c) */
extern void func_80021128(void);
extern int func_80030584(pe_addr_t pe_a0, pe_addr_t pe_a1);
extern void func_80022D7C(pe_addr_t target);
extern int32_t func_8006DD38(uint32_t index, uint32_t key, int32_t x, int32_t y, int32_t z);
extern int32_t func_800518A8(pe_addr_t out);
extern void func_80051510(void);
/* PE-CH3 field message subsystem (field_message_port.c). */
extern int func_800515C0(uint32_t value);
extern int func_80051684(uint32_t value);
extern int func_800629B0(void);
extern int32_t func_80057E14(pe_addr_t list);
extern int func_8005270C(void);
extern void func_8004BF08(void);
extern void func_8004BE4C(void);
int32_t func_8005B8A8(int32_t experience, pe_addr_t table);
int32_t func_80051DF8(int32_t index);
int func_80057ECC(void);
void func_80052764(void);
void func_8004B970(void);
void func_8004BC80(void);
void func_8004BCB4(void);
void func_8004C4B4(uint32_t ability);
void func_8004C520(void);
int func_80055668(uint32_t level);
int func_8004C1E0(pe_addr_t window, uint32_t event);
int32_t func_8005382C(int32_t count);
void func_80048654(void);
void func_8004BF40(void);
void func_800437B4(uint32_t index);
void func_8005BA78(int32_t table_number,int32_t key,pe_addr_t remaining_out,pe_addr_t fraction_out);
void func_8006062C(int32_t remaining,int32_t fraction);
void func_8004FFF8(int slot);
void func_80050D20(uint32_t index);
void func_8004FCF8(pe_addr_t list);
void func_8004FD68(pe_addr_t pe_a0);
void func_80050B94(int a0);
void func_80050BE8(uint32_t index);
void func_80057F14(uint32_t index);
int func_80058030(uint32_t first_list,int32_t first,uint32_t second_list,int32_t second);
void func_80058454(void);
void func_80058670(void);
int func_80048838(pe_addr_t window,uint32_t event);
void func_80063D78(pe_addr_t pe_a0, int a1, int a2);
extern void func_8005C144(void);
extern void func_8004BCE8(int32_t arg0);
extern int func_8005D2B4(int32_t cmd, int32_t a, int32_t b);
extern int func_80015C7C(pe_addr_t args);
extern int32_t func_800574A8(void);
extern int func_800C6CE0(pe_addr_t slot);
extern pe_addr_t func_800C22F8(pe_addr_t slot);
extern int func_800C9A70(pe_addr_t slot);
extern int func_800CD728(pe_addr_t slot);
extern int func_800C2AF0(pe_addr_t slot, int32_t unused, int32_t index, uint32_t value);
extern void func_8006D60C_state0_a0eq1_cut(void);
extern void func_8006D60C_state2C_cut(void);
extern int func_8006D60C(int a0);
extern void func_8006D078_state0_cut(void);
extern int func_8006D078_state2A_cut(void);
extern int func_8006D078_state2B_cut(void);
extern int func_8006D078(void);
extern int func_8006D2B8(int id, int load, int second, pe_addr_t out_slot, int blocking);
extern int func_80087198(void);
extern int func_800871AC(pe_addr_t buffer, uint32_t bytes);
extern int func_80087428(unsigned bank, pe_addr_t buffer, uint32_t bytes);
extern int func_800875FC(unsigned bank, pe_addr_t buffer);
extern int func_8006CDA4(int a0, int a1, int a2, pe_addr_t a3, int stack_len,
                         int stack_flag);
extern int func_8006C4C4(int arg0);
extern int func_8006C1CC(int a0);
extern int func_8006C5BC(void);
extern pe_addr_t func_8006C5BC_ee13_prefix_cut(void);
extern void func_8006C5BC_ee13_epilogue_cut(void);
extern int func_8003F074_6C4C4_6C5BC_cut(void);
extern int func_8003F074_poll_cut(void);
extern pe_addr_t func_8003F074_371b0_a0(void);
extern void func_8003F074_after_poll_cut(void);
extern void func_8001A918(void);
extern void func_800E0060(void);
extern void func_800371B0(pe_addr_t a0);
extern pe_addr_t func_80012574(pe_addr_t a0);
extern void func_8001266C(void);
extern void func_800125E0(void);
extern pe_addr_t func_80035038(pe_addr_t desc, pe_addr_t parent,
                               unsigned int a2);
extern void func_8003F074_pool_cut(void);
extern void func_800361F4(pe_addr_t actor);
extern void func_80035E04(pe_addr_t actor);
extern void func_80035C84(pe_addr_t actor);
extern void func_8003999C(pe_addr_t actor, pe_addr_t table, pe_addr_t codep);
extern void func_8007136C(pe_addr_t actor, pe_addr_t codep);
extern void func_800710A4(pe_addr_t actor, pe_addr_t codep);
extern void func_80078934(pe_addr_t matrix, pe_addr_t src, pe_addr_t dst);
extern void func_8002F76C(pe_addr_t actor);
extern void func_80035558_walk_cut(void);
extern void func_8001AE40(pe_addr_t actor);
extern void func_8001A9F8_floor_cut(void);
extern int func_80065E48_position(int32_t x, int32_t y, int32_t z);
extern void func_80017018(void);
int func_80018300(pe_addr_t args);
int func_80018364(pe_addr_t args);
extern int func_80017294(pe_addr_t pe_a0);
extern int func_800172BC(void);
extern int func_800172E0(pe_addr_t pe_arg0);
extern int func_800172FC(void);
extern int func_800181CC(pe_addr_t args);
extern int func_80015DAC_default_cut(pe_addr_t args);
extern void func_80191DE8(int a0);
extern int32_t func_80191E30(uint32_t id);
extern void func_80190998(void);
extern void func_80191580(pe_addr_t object);
extern int32_t func_801915DC(void);
extern void func_80191678(uint32_t id);
extern void func_80191740(void);
extern pe_addr_t func_80191754(void);
extern void func_801917BC(pe_addr_t object, pe_addr_t parent);
extern void func_80191834(pe_addr_t object);
extern uint32_t func_8019959C(pe_addr_t resource, uint32_t index);
extern pe_addr_t func_80190AEC(pe_addr_t parent, uint32_t value);
extern pe_addr_t func_80190B78(pe_addr_t parent, uint32_t index, pe_addr_t resource);
extern pe_addr_t func_80190C1C(pe_addr_t parent, uint32_t index, uint32_t a2,
                            uint32_t a3, uint32_t a4, uint32_t a5, pe_addr_t resource);
extern void func_80190D08(pe_addr_t object);
extern void func_80191EFC(uint32_t handle, pe_addr_t position);
extern void func_800868F0(uint32_t handle, uint32_t group, uint32_t volume);  /* retail $v0 unread (port3 A1) */
extern void func_80086A28(uint32_t handle, uint32_t group, uint32_t pan);  /* retail $v0 unread (port3 A1) */
extern void func_8019BF8C(pe_addr_t dest);
extern void func_8019BD78(void);
extern void func_80191C94(void);
extern void func_80191854(void);
extern void func_80196498(void);
extern void func_80192030(void);
extern int func_80038D48(void);
extern void func_800868AC(int a0, int a1);
extern void func_80195994(uint32_t variant,uint32_t eye_shift,uint32_t target_shift,uint32_t time);
extern void func_80195D3C(void);
extern void func_80195BC8(pe_addr_t target,pe_addr_t eye,uint32_t eye_shift,uint32_t target_shift);
extern void func_80195E4C(uint32_t id,uint32_t unused1,uint32_t unused2,uint32_t time);
extern int func_8018F55C(uint32_t time, uint32_t id, pe_addr_t package,
                        pe_addr_t position, pe_addr_t rotation);
extern void func_8019BF50(void);
extern void func_80193AB0(void);
extern void func_801941A4(uint32_t intensity);
extern void func_80195F6C(void);
extern void func_80078E34(pe_addr_t matrix);
extern void func_80078E64(pe_addr_t matrix);
extern void func_80078FC4(uint32_t r, uint32_t g, uint32_t b);
extern void func_80078FE4(uint32_t r, uint32_t g, uint32_t b);
extern void func_80077E64(uint32_t near_z, uint32_t far_z, int32_t h);
extern int func_80194108(int level);
extern void func_80192740(void);
extern int func_80193B5C(int level);
extern int func_80015DAC_key190_cut(pe_addr_t args);
extern int func_800173F4(pe_addr_t args);
extern int func_80017E20(pe_addr_t pe_a0);
extern uint32_t func_8003708C(uint32_t a, uint32_t b);
extern uint32_t func_800370A8(uint32_t a, uint32_t b);
extern int func_80012850(pe_addr_t args);
extern int func_8001731C(pe_addr_t args);
extern int func_80017588(pe_addr_t args);
extern int func_80017D7C(void);
extern int func_80017D5C(void);
extern int func_80016910_key2900_cut(pe_addr_t args);
extern int func_8001A374(pe_addr_t pe_arg0);
extern int func_80018E84(pe_addr_t pe_arg0);
extern int func_800655D4(void);
extern int func_80018F54(void);
extern int func_80066C7C(short arg0);
extern void func_80066CE8(void);
extern void func_80065674(void);
extern void func_80067E1C(void);
extern int func_80067294(pe_addr_t rec);
extern void func_80067A78(void);
extern int func_80067B74(void);
extern int func_80067D18(void);
extern int func_80068CE0(void);
extern int func_80066F60(pe_addr_t rec, pe_addr_t prim_base, pe_addr_t *out);
extern int func_800677FC(pe_addr_t prim_base, pe_addr_t *cursor);
extern int func_80068B94(void);
extern int func_80077CF4(int a0);
extern int func_80077DC4(int a0);
extern int func_8001A214(pe_addr_t pe_args);
extern int PE_M28MovementOverlay(void);
extern int PE_M28MovementInit(pe_addr_t slot);
extern int PE_M28MovementCommand(pe_addr_t slot, uint32_t mode, uint32_t command,
                                 uint32_t a, uint32_t b, uint32_t c);
extern int PE_M28MovementUpdate(pe_addr_t slot);
extern int PE_M28MovementCleanup(pe_addr_t slot);
extern int PE_M32MovementOverlay(void);
extern int PE_M32MovementInit(pe_addr_t slot);
extern int PE_M32MovementCommand(pe_addr_t slot, uint32_t mode, uint32_t command,
                               uint32_t a, uint32_t b, uint32_t c);
extern int PE_M32MovementUpdate(pe_addr_t slot);
extern int func_80069660(void);
extern int PE_M28ProjectileMain(int32_t mode,pe_addr_t data,pe_addr_t extra);
extern int func_80018EE0(pe_addr_t pe_args);
extern int func_80017988(pe_addr_t args);
extern int func_80019618(void);
extern int func_8001856C(void);
extern int func_80018E58(pe_addr_t pe_args);
extern int func_80019410(void);
extern int func_80019638(void);
extern int func_80019658(void);
extern int func_80018BEC(void);
extern int func_80019AC0(void);
extern int func_80066B60(int a0);
extern int func_80018EB4(pe_addr_t pe_args);
extern int func_8001A1F0(void);
extern int func_800176FC(pe_addr_t args);
extern int func_80018954(void);
extern int func_80018164(pe_addr_t pe_a0);
extern int func_80018A48(pe_addr_t pe_a0);
extern int func_8001897C(pe_addr_t args);
extern int func_80018004(pe_addr_t pe_a0);
extern int func_800131E8(pe_addr_t args);
extern int func_80018774(pe_addr_t args);
extern int func_80013C34(pe_addr_t args);
extern int func_80013514(pe_addr_t pe_a0);
extern int func_800136C0(pe_addr_t pe_a0);
extern int func_800143B0(pe_addr_t args);
extern int func_800187C0(pe_addr_t args);
extern int func_800184EC(pe_addr_t pe_a0);
extern int func_8002FAF8(pe_addr_t actor, unsigned int code);
extern int func_80014228(pe_addr_t args);
extern int func_80017410(pe_addr_t args);
extern int func_80019D84(pe_addr_t args);
extern int func_800177C8(pe_addr_t args);
extern signed char func_80037548(short needle);
extern void func_800375E0(int id, unsigned int mode, pe_addr_t list);
extern void func_80037870(void);
extern void func_8003EB04(void);
extern int func_80037864(void);
extern int func_80017DE4(pe_addr_t args);
extern int func_80017F88(pe_addr_t pe_source);
extern int func_80017FB0(pe_addr_t pe_source);
extern int func_80019484(pe_addr_t pe_arg0);
extern int func_80017A50(pe_addr_t args);
extern int func_80066BD8(short arg0, unsigned short arg1, unsigned short arg2, unsigned short arg3, unsigned short arg4);
extern int func_80018F0C(pe_addr_t pe_args);
extern int func_80019BE4(void);
extern int func_80019748(void);
extern int func_80019798(pe_addr_t args);
extern unsigned char func_800392EC(void);
extern int func_80019728(void);
extern int func_800196E8(pe_addr_t args);
extern int func_8001A3FC(pe_addr_t args);
extern int func_80018080(pe_addr_t args);
extern int func_80017FF0(void);
extern int func_800192B8(void);
extern int func_800192C8(void);
extern int func_800192DC(pe_addr_t args);
extern int func_800193D8(pe_addr_t args);
extern int func_80018A9C(pe_addr_t pe_a0);
extern void func_8001D340(unsigned int a0);
extern void func_8001F9C4(void);
extern void func_800201DC(void);
extern void func_80020288(pe_addr_t enemy);
extern void func_80020CE4(void);
extern void func_80023E14(int32_t item);
extern void func_800523F8(pe_addr_t rate, pe_addr_t delay);
extern void func_8003335C(pe_addr_t actor, unsigned unused, unsigned icon);
extern int32_t func_8005409C(int32_t item);
extern int32_t func_8005485C(void);
extern void func_800553A4(pe_addr_t category, pe_addr_t amount);
extern int32_t func_80057D30(int32_t index);
extern int32_t func_80059A40(pe_addr_t out);
extern int32_t PE_ItemArmorCapacity59A40(uint32_t *extra);
extern int32_t func_80054E4C(int32_t extra);
extern void func_80054CF8(void);
extern void func_80062F9C(void);
extern int func_80067CBC(void);
void func_8004D18C(void);
int func_8004D2DC(pe_addr_t window,uint32_t event);
void func_80040F80(pe_addr_t record);
void func_80042228(void);
void func_8004D5CC(uint32_t index);
/* Explicit original caller stack and incoming s0..s7/ra; stops at unresolved BIOS. */
uint32_t PE_FormatterFrame(pe_addr_t destination, pe_addr_t format, uint32_t arg0, uint32_t arg1, pe_addr_t caller_sp, const uint32_t saved[9]);
pe_addr_t func_80072334(pe_addr_t destination,pe_addr_t source,uint32_t count);
void PE_CardCleanupFrame(pe_addr_t record, pe_addr_t caller_sp, const uint32_t incoming[32]);
void PE_CardOperationFrame(uint32_t index, pe_addr_t caller_sp, const uint32_t incoming[32]);
void func_80041108(uint32_t index);
void func_800405A4(uint32_t index);
void func_800425DC(void);
void func_8004D9D8(void);
void func_8004DC84(void);
void func_8004CDAC(void);
void func_80042928(void);
int func_8004D4A0(void);
void func_800428D4(void);
void func_80042B50(pe_addr_t cb);   /* matched: func_800A1870_t (a guest code address) */
int func_8004DA9C(void);
int32_t func_80042464(void);
void func_8004DA04(void);
void func_8004CFD4(void);
void func_8004CE28(uint32_t first,uint32_t second);
void func_80050580(pe_addr_t unused,uint32_t confirmed);
int func_80042770(int a0);
int func_8003FFCC(void);
void func_80190064(void);
int func_800428C4(void);
void func_8004DAA4(void);
int func_80042848(int a0);
void func_80042910(void);
int func_80042B28(void);
int func_80042AD8(int a0);
void func_80062CD0(int a0);
void func_8004298C(int a0, int a1);
void func_80042A10(void);
int func_8004FDA4(uint32_t index);
int func_8004FE58(int a0);   /* decomp_hand/absent_lo_menu_port.c: save-slot cell predicate */
void func_80050C08(int arg0);
/* Per-cell draw of the field-menu list (generated decomp port, 0x80050C50);
 * reached only through func_8004FF30 -> func_800638D8. */
void func_80050C50(int32_t value);
void func_8004FDE8(pe_addr_t list);
int func_80015AF0(pe_addr_t args);
extern void func_8001F4D4(pe_addr_t actor);
extern short func_8001F814(pe_addr_t actor);   /* src/func_8001F814.c returns short (retail sign-extends $v0) */
extern int func_800305C8(pe_addr_t pe_a0, pe_addr_t pe_a1);
extern int func_8006DDCC(int a0, int a1, short a2, short a3, short a4);
extern void func_80051770(int32_t ability);
extern int func_8006DE80(int a0, int a1, short a2, short a3, short a4);
extern int32_t func_8006DED4(pe_addr_t dest, int id, int a1, int x, int y, int z);
int32_t PE_SpatialSoundRequest(pe_addr_t dest,int id,int key,int x,int y,int z,uint32_t *computed_volume);
extern int func_8006DCE4(int a0, int a1, short a2, short a3, short a4);
extern unsigned int func_80071A54(void);
extern int32_t func_8002156C(void);
extern void func_800216E4(pe_addr_t records, int8_t first, int8_t last);
extern void func_80021850(pe_addr_t pe_records, signed char first, signed char second);
extern int func_80030534(pe_addr_t pe_a0, pe_addr_t pe_a1);
extern int func_8005186C(int value);
extern void func_80020F18(void);
extern void func_800374E8(void);
extern void func_80043240(int a0);
extern int32_t func_80070064(void);
extern int32_t func_8006FC18(uint32_t index, pe_addr_t owner, uint32_t force);
extern int32_t func_800702DC(void);
extern int32_t func_800701B4(void);
extern void func_800703F4(void);
extern int32_t func_8006FE14(pe_addr_t owner);
extern int func_8001930C(pe_addr_t args);
extern void func_80028C48(pe_addr_t actor);
extern void func_800293F4(int a0);
extern void func_80020D50(void);
extern void func_80021D4C(void);
extern int func_8006F6D4(unsigned int index, unsigned int mode,
                         unsigned int a2, pe_addr_t out0, pe_addr_t out1,
                         pe_addr_t out2);
extern int func_800D4698(pe_addr_t slot, unsigned int mode, unsigned int a2,
                         unsigned int a3, unsigned int extra0,
                         unsigned int extra1);
extern int func_8006F39C(unsigned int code, pe_addr_t userdata);
extern int func_800CE49C(pe_addr_t slot, unsigned int extra);
extern void func_800D4620(pe_addr_t slot);
extern int func_8001735C(pe_addr_t args);
extern void func_8001AA78(pe_addr_t actor);
extern int func_8001C614(pe_addr_t rec, int a1, int a2);
extern int func_80012C20(pe_addr_t args);
extern int func_80017D9C(void);
extern int func_80017AE8(pe_addr_t pe_a0);
extern int func_80017EC4(pe_addr_t args);
extern int func_80017B34(pe_addr_t pe_arg0);
extern int func_80017B74(void);
extern int func_80014694(pe_addr_t args);
extern int func_8001CAB0(int px, int pz, pe_addr_t pe_v, unsigned short n);
extern int func_8001A390(pe_addr_t pe_a0);
extern int func_80014DA0(pe_addr_t args);
extern int func_80014BA0(pe_addr_t args);
extern int func_80012E7C(pe_addr_t args);
extern int32_t func_80079FB4(int32_t a0, int32_t a1);
extern int func_8001A15C(pe_addr_t pe_a0);
extern int func_8001784C(pe_addr_t pe_a0);
extern int func_800130B4(pe_addr_t args);
extern int func_80015240(pe_addr_t args);
extern void func_80039B74(pe_addr_t dest, pe_addr_t clip, int a2, int a3);
extern pe_addr_t func_800362B8(unsigned int size);
extern void func_8003A6A8(pe_addr_t dest, pe_addr_t unused);
extern void func_8003E188(pe_addr_t dest);
extern int func_8006E2D0(pe_addr_t pe_arg0, unsigned int arg1);
extern int func_8006E454(pe_addr_t pe_arg0);
extern void func_8006B4F8_12574_publish_cut(void);
extern int func_8006B4F8_dest_load_cut(uint32_t token);
extern void func_8006B35C(void);
extern int func_8006BE4C(void);
extern int func_8006BECC(void);
extern int func_8003F074_dest_ready_cut(uint32_t token);
extern int func_8006CC68(void);
extern void func_800661A4(void);
extern void func_800661CC(void);
extern void func_8006C5BC_clear_wait_cut(void);
extern void func_8003D050_prefix_cut(pe_addr_t dest, pe_addr_t obj,
                                     pe_addr_t stream, unsigned int stack_ba);
extern void func_8003D050_ptr14_cut(pe_addr_t dest, pe_addr_t obj);
extern pe_addr_t func_8003D050_packets(pe_addr_t dest);
extern void func_8003D94C(pe_addr_t dest, int x, int y, int unused, int palette_y);
extern int func_8003D050_post_3d94c_skip_cut(pe_addr_t dest, pe_addr_t stream);
extern void func_8003D050_epilogue_cut(pe_addr_t dest, int skipped);
extern int func_8003C638(pe_addr_t dest);
extern void func_8003CCB0(pe_addr_t dest, int enabled);
extern void func_8003CEF8(pe_addr_t dest, int mode);
extern void func_8003B708(pe_addr_t dest, int bank);
extern void func_8003C0B4(pe_addr_t dest, int level, uint32_t red, uint32_t green, uint32_t blue);
extern void func_8003C5D8(pe_addr_t pe_state, short scale);
int func_8003D050(pe_addr_t o, pe_addr_t c, pe_addr_t p0, short a3, short a4, short a5, short a6,
                  short a7, pe_addr_t out, int flag);   /* func_8003D050_port.c (src/func_8003D050.c) */
extern int func_8003C818(pe_addr_t dest);
extern void func_8003B144(pe_addr_t dest);
extern int func_8003AF14(pe_addr_t dest, pe_addr_t scratch);
extern void func_8006698C(pe_addr_t dest);
extern void func_8003DFD8(pe_addr_t pe_src, pe_addr_t pe_dst, short count);
extern void func_8003D834(pe_addr_t dest, pe_addr_t clip, int a2, pe_addr_t bea40);
extern void func_800794C4(pe_addr_t angles, pe_addr_t out);
extern void func_8003A088_mode0_empty_cut(pe_addr_t dest);
extern void func_8003AC90(pe_addr_t dest, pe_addr_t view);
extern void func_8003A088_mode0_walk_cut(pe_addr_t dest);
extern void func_8003B97C_empty_cut(pe_addr_t dest, pe_addr_t bea40);
extern void func_8003B97C_lighting_cut(pe_addr_t dest, pe_addr_t bea40);
extern void func_8003BCE0(pe_addr_t dest, int a1, int a2);
extern void func_80070D10(void);
extern unsigned int func_80070D6C(void);
extern int func_80070DD0(int, int);

/* func_8003EAC8 deterministic argument recording (test instrumentation) */
void PE_3EAC8_RecordReset(void);
void PE_3EAC8_RecordSetEnabled(int enabled);
int  PE_3EAC8_RecordCount(void);
int  PE_3EAC8_RecordAt(int index, int *a0, int *a1);
extern void func_8006A5BC(void);
extern void func_8006A64C(void);
extern void func_8006A674(void);
extern void func_8006A8D4(void);

/* func_80077B64/B A4/BC4/C44/C64 — Phase 6E-PE-GPU1: REAL translated
 * GPU packet-header setters (SetPolyF3 / SetPolyFT4 / SetPolyG4 / SetTile /
 * SetSprt).  game/boot/func_80077B{64,BA4,BC4,C44,C64}_port.c — each a
 * 5-word real outlined header-inline writing the primitive packet's byte
 * offset 3 (length) and byte offset 7 (code).  Called via jal from
 * func_80030894; B54K-A..B6 exercise the SET leaves through L2/L3 and the
 * complete L4-L9 packet groups. */
extern void func_80077B64(pe_addr_t pe_arg0);
extern void func_80077BA4(pe_addr_t pe_arg0);
extern void func_80077BC4(pe_addr_t pe_arg0);
extern void func_80077BE4(pe_addr_t pe_arg0);
extern void func_80077C44(pe_addr_t pe_arg0);
extern void func_80077C64(pe_addr_t pe_arg0);
extern void func_80077AC4(pe_addr_t ot, pe_addr_t packet);
extern void func_80033A40(void);
extern void func_80034104(int32_t maximum, int32_t current);
extern void func_800334AC(void);

/* Phase 6E-B54I: GPU primitive-builder leaves of func_80030894 and the
 * two add/sort wrappers, all REAL translations with full word-decode
 * headers (game/boot/func_80077*_port.c, func_8005DADC_port.c,
 * func_80037{0DC,140}_port.c, platform/func_800719E4_port.c):
 *   func_80077A64  GetTPage(tp, abr, x, y) value builder (pure)
 *   func_80077AA4  CLUT value builder, (y<<6)|((x>>a4)&0x3F), &0xFFFF
 *   func_80077B04  SetSemiTrans — code byte bit 1 set/clear
 *   func_80077B34  SetShadeTex — code byte bit 0 set/clear
 *   func_80077C04  setSprt header — len 4, code 0x64
 *   func_80077C84  DR draw-mode word — p[3]=1; p+4 word; returns word
 *   func_80077CB4  length-budget append — cap 17, fail -1, tail.tag=0
 *   func_8005DADC  *(u32*)0x800A8030 + 0x800A8028 + (a0<<3)
 *   func_800370DC  add/sort wrapper (draw-mode + setSprt + append)
 *   func_80037140  add/sort wrapper (draw-mode + SetTile + append)
 *   func_800719E4  BIOS B(38h) CD-mode trampoline (fail path; collapsed
 *                  with recorded justification, pe_libcd.c precedent) */
extern int func_80077A64(int tp, int abr, int x, int y);
extern int func_80077AA4(int a0, int a1);
extern void func_80077B04(pe_addr_t pe_a0, int a1);
extern void func_80077B34(pe_addr_t pe_a0, int a1);
extern void func_80077C04(pe_addr_t pe_arg0);
extern uint32_t func_80077C84(pe_addr_t p, uint32_t a1, uint32_t a2,
                              uint32_t a3);
extern int func_80077CB4(pe_addr_t pe_head, pe_addr_t pe_tail);
extern pe_addr_t func_8005DADC(uint32_t index);
extern void func_800370DC(pe_addr_t pe_head, int tag);
extern void func_80037140(pe_addr_t pe_head, int tag);
extern uint32_t func_800719E4(uint32_t mode);

/* Phase 6E-B54K-A / B45 siblings: declared here so tests and TUs that
 * include only the shared headers see the real ports instead of an
 * implicit declaration. */
extern void func_80030894(void);
extern int func_800870E0(void);

/* Host view of transient animation scratch angles (79754). */
void PE_RotMatrix79754(const int16_t angles[3], pe_addr_t out);
void PE_RotMatrix79754_values(const int16_t angles[3], int16_t out[9]);
void PE_RotMatrix794C4_values(const int16_t angles[3], int16_t out[9]);

void func_800783E4(pe_addr_t a, pe_addr_t b, int32_t wa, int32_t wb, pe_addr_t out);
void func_80078554(pe_addr_t a, pe_addr_t b, int32_t wa, int32_t wb, pe_addr_t out);
pe_addr_t func_80078CC4(pe_addr_t matrix, pe_addr_t scale);
void func_800786E4(pe_addr_t matrix);
void PE_EffectColorCF3AC(pe_addr_t curve, uint8_t out[3], int32_t time);
void func_800CF3AC(pe_addr_t curve, pe_addr_t out, int32_t time);
void PE_EffectSpriteCEE20(pe_addr_t position, const int16_t angles[4], int32_t sx,
    int32_t sy, int32_t texture, uint32_t clut, int32_t abr, int32_t brightness,
    const uint8_t color[3]);
void func_800CEE20(pe_addr_t position, pe_addr_t angles, int32_t sx, int32_t sy,
    int32_t texture, uint32_t clut, int32_t abr, int32_t brightness, pe_addr_t color);
int PE_M0023I_Main(int32_t mode, pe_addr_t data);
int PE_M0023I_Particle(int32_t mode, pe_addr_t data);
int PE_M0023I_Beam(int32_t mode, pe_addr_t data);
int PE_M0023I_Flash(int32_t mode, pe_addr_t data, int16_t retained_position[3]);
extern int g_pe_strict_effect_stack;   /* m0023i_effect_port.c: 1 = unresolved F710 stack read stops the port */
void PE_EffectStackBegin(void);
void PE_EffectStackEnd(void);
void PE_EffectStackInvalidate(void);
int PE_EffectStackWeaponDraw(pe_addr_t fn,pe_addr_t slot);
int PE_EffectStackWeaponUpdate(pe_addr_t fn,pe_addr_t slot);
void PE_EffectStackWeaponUpdateCall(void);
int func_800CE084(pe_addr_t slot);
int func_800CE144(int slot);
int func_800CE16C(int a0);
void func_800CE1FC(void);
void func_800CE2B4(int a0, int a1, pe_addr_t pe_a2);
void func_800CE3B4(int a0, int a1, pe_addr_t pe_a2);
void func_800CE464(unsigned int unused,pe_addr_t record);
void func_800CE470(pe_addr_t unused,pe_addr_t record,pe_addr_t data);
void PE_EffectStackWeaponCallback(pe_addr_t fn,pe_addr_t data);
int func_8018F330(int32_t mode, pe_addr_t data);
int func_8018F018(int32_t mode, pe_addr_t data);
int func_8018F614(int32_t mode, pe_addr_t data);
int func_8018FB84(int32_t mode, pe_addr_t data);
int func_8018FDC4(int32_t mode, pe_addr_t data);
int func_800D751C(int32_t mode, pe_addr_t data);
int func_800D71B8(int32_t mode, pe_addr_t data);
int func_800D70C0(int32_t mode, pe_addr_t data);
void func_800D1DEC(pe_addr_t position,pe_addr_t color,int32_t brightness,int32_t blend);
void func_8004DD64(int32_t id);
void func_8004DF74(pe_addr_t node);
void func_8004C608(pe_addr_t window);
void func_8005F874(int32_t value);
void func_8005FB74(int value);
void func_8006006C(int t, int mode);
uint32_t func_80052894(uint32_t index);
pe_addr_t func_8005DD3C(uint32_t id);
pe_addr_t func_8005DCEC(uint32_t id);
pe_addr_t func_8005DB44(unsigned int index);
unsigned int func_80052F70(void);
int32_t func_8005415C(int32_t index);
int func_80054288(void);
int func_800556E8(int a0);
int func_80058E08(int a0);
int func_80057ED8(int a0);
pe_addr_t func_80058BBC(int32_t index);
int32_t func_80059F08(uint32_t index);
void func_80055610(void);
int func_8004E970(void);
int func_8005E120(void);
pe_addr_t func_80051098(void);
int func_80056B24(int n);
void func_80057094(void);
void func_80056FB8(void);
void func_800453E8(pe_addr_t window);
int func_800452C0(int a0, int a1);
int func_80044B0C(pe_addr_t window,uint32_t event);
int func_80043DA4(pe_addr_t window,uint32_t event);
void func_80057834(int32_t index);
void func_800516B4(int32_t item);
void func_8005112C(void);
int32_t func_800533D4(pe_addr_t record);
void func_80044274(int32_t index);
void PE_CopyItemRecord(pe_addr_t destination,pe_addr_t source);
void PE_MenuApplyAmmo(pe_addr_t item);
void func_8005F354(pe_addr_t text,int32_t width);
void func_8005F594(int value);
void func_80064C30(int value);
void func_80064C54(uint32_t id);
void func_80062A7C(uint32_t id);
pe_addr_t func_80053068(int32_t index);
void func_80052BCC(pe_addr_t pe_dst, pe_addr_t pe_src);
void func_80052C08(pe_addr_t pe_dst, pe_addr_t pe_src);
void func_80050878(uint32_t index);
void func_8004F910(int a0);
void func_800509A8(int a0);
void func_8004F950(int slot);
void func_80050CF8(void);
void func_8004FFA8(int slot);
void func_800631AC(pe_addr_t pe_arg0);
void func_8004D024(int a0);
void func_8004CDD4(pe_addr_t window);
void func_8004CC50(uint32_t first,uint32_t second);
int func_8004D030(pe_addr_t window,uint32_t event);
int func_80044E14(pe_addr_t window);  /* retail $v0 = func_80062A7C variant (batch O) */
void func_80044F8C(void);
void func_80062CE4(void);
int32_t func_80058C4C(uint32_t categories);
void func_80045110(pe_addr_t window,uint32_t confirmed);
int func_80044E98(pe_addr_t window,uint32_t event);
int func_80053D2C(int arg);
pe_addr_t func_800532B4(uint32_t id);
int func_800194B0(pe_addr_t args);
int func_80019540(pe_addr_t pe_a0);
int func_8004C34C(uint32_t item);
void func_80055E14(void);
int func_80015BAC(pe_addr_t args);
void func_8004F490(uint32_t item);
void func_8004F644(void);
int func_8004F730(pe_addr_t window,uint32_t event);
void func_8004F798(void);
void func_8004F7D8(void);
void func_80051060(void);
void func_80050204(int slot);
void func_8005022C(pe_addr_t list);
int func_800210D4(void);
signed char func_80052558(void);
void func_80055724(void);
void func_80055FB4(int index);
int32_t func_800562A4(int32_t index);
void func_8005600C(pe_addr_t bank,pe_addr_t selected);
void func_80056C40(uint32_t first_bank,int32_t first,uint32_t second_bank,int32_t second);
int func_80057D18(int index);
int func_8005833C(int32_t index);
int func_80057654(int32_t index);
void func_80044924(pe_addr_t owner,uint32_t bank,int32_t index);
void func_800451D0(pe_addr_t owner);
int func_80044444(pe_addr_t window,uint32_t event);
void PE_MenuCommitResult(uint32_t result);
void func_80055760(void);
void func_80050260(void);
int func_80055FE0(int index);
int func_80057C54(uint32_t first_list,int32_t first,uint32_t second_list,int32_t second);
int32_t func_8005401C(void);
int func_80054240(int32_t index);
void func_8005FA3C(int value);
void func_80063158(pe_addr_t pe_node, int x, int y);
void func_80062F1C(pe_addr_t pe_node);
void func_80064C80(void);
void func_80050804(int a0);
void func_8004F8D0(int a0);
void func_800447F0(pe_addr_t window);
void func_80044174(pe_addr_t owner);
void func_80064EB4(pe_addr_t scrollbar);
void func_800534E4(pe_addr_t record,pe_addr_t name);
void func_80053648(pe_addr_t record);
void func_800536B8(int32_t index);
pe_addr_t func_80051E48(void);
pe_addr_t func_8005DBF8(void);
int func_80021080(void);
signed char func_80052534(void);
void func_800602D0(int32_t value,int32_t digits);
void func_80060528(int a);
void func_8006055C(int a);
void func_80060590(int a);
void func_800605C4(int a);
void func_800605F8(int a);
void func_80043B0C(pe_addr_t node);
void func_80043C64(pe_addr_t node);
void func_8004905C(void);
void func_80050748(uint32_t index);
void func_8004F838(pe_addr_t node);
int func_8004E074(pe_addr_t window,uint32_t event);
int func_8004E2E4(pe_addr_t window,uint32_t event);
int func_8005BCB0(void);
int func_8005BEDC(void);
pe_addr_t func_8005BEE8(void);
void func_8005BD10(int a0);
int func_8005BD94(void);
void func_8005BE1C(void);
int32_t func_8005BF08(void);
void func_80052FCC(pe_addr_t pe_a0);
pe_addr_t func_80062A20(pe_addr_t node,uint32_t index);
int func_80063428(pe_addr_t pe_record);
int func_8005E038(void);
int func_80063E0C(pe_addr_t node,uint32_t event);
int func_800650E0(pe_addr_t node,uint32_t event);
pe_addr_t PE_MenuPacketAlloc(uint32_t bytes);
void PE_MenuPacketLink(pe_addr_t packet);
void PE_MenuPacketColor(pe_addr_t packet,uint8_t words,uint8_t code);
void func_8006153C(uint32_t first,uint32_t second,int32_t thickness,uint32_t shade);
void func_80061878(pe_addr_t edges,uint32_t shade);
void func_80061A3C(uint32_t width,uint32_t height,uint32_t shade);
void func_80061C34(uint32_t width,uint32_t height,pe_addr_t shape,uint32_t textured);
void func_80062090(uint32_t width,uint32_t height,uint32_t pulse);
void func_800622BC(uint32_t width,uint32_t height,uint32_t pressed,uint32_t focused);
void func_800634D4(pe_addr_t node,pe_addr_t draw,int32_t row,uint32_t dim);
void func_8006374C(pe_addr_t node);
void func_80065260(pe_addr_t scrollbar);
void func_800638D8(pe_addr_t node,pe_addr_t draw);
void func_800500A8(pe_addr_t node);
void func_8005010C(pe_addr_t node);
void func_80050178(pe_addr_t node);
void func_800501C8(pe_addr_t node);
void func_80062830(pe_addr_t node);
void func_80062FEC(void);
/* Equipment sub-page list2 per-cell draw (generated matching port). */
void func_800504BC(int a0);
void func_8005E8A4(int a0, int a1);
void func_8005E8C4(void);
void func_8005E914(void);
void func_8005EB58(int a0);
unsigned char func_8005DC28(int index);
void func_8005EB64(uint32_t icon);
void func_8005EED4(uint32_t character);
int func_8005F1A0(pe_addr_t pe_p);
void func_8005F27C(pe_addr_t text);
void func_8005F5B8(uint32_t id);
void func_80050F10(int a0);
void func_80050F64(uint32_t index);
void func_80050FB8(uint32_t index);
void func_8005100C(uint32_t index);
void func_8005E6F0(void);
void func_8005E12C(int32_t fast_repeat);
void func_8005E114(int a0);
void func_800525EC(void);
void func_80052634(void);
void func_8005267C(void);
void func_800526C4(void);
void PE_EffectRibbonD2370(const int16_t position[3],const int16_t angles[3],
    int32_t length,int32_t width,int32_t u,int32_t v,int32_t uw,int32_t vh,
    uint32_t clut,const uint8_t start_color[3],const uint8_t end_color[3],
    int32_t brightness,int32_t abr);
void func_800D2370(pe_addr_t position,pe_addr_t angles,int32_t length,int32_t width,
    int32_t u,int32_t v,int32_t uw,int32_t vh,uint32_t clut,pe_addr_t start_color,
    pe_addr_t end_color,int32_t brightness,int32_t abr);
void PE_ActorJointPositionCE8F0(pe_addr_t actor,uint32_t joint,const int16_t local[3],int16_t out[3]);
void PE_EffectSpriteValuesCEE20(const int16_t position[3],const int16_t angles[4],int32_t sx,
    int32_t sy,int32_t texture,uint32_t clut,int32_t abr,int32_t brightness,const uint8_t color[3]);
void PE_EffectFanD004C(const int16_t position[3],int32_t radius0,int32_t radius1,int32_t segments,
    const int16_t angles[4],int32_t sx,int32_t sy,const uint8_t center_color[3],
    const uint8_t edge_color[3],int32_t brightness,int32_t abr);
void func_800D004C(pe_addr_t position,int32_t radius0,int32_t radius1,int32_t segments,
    pe_addr_t angles,int32_t sx,int32_t sy,pe_addr_t center_color,pe_addr_t edge_color,
    int32_t brightness,int32_t abr);
void PE_EffectRingD0728(const int16_t position[3],int32_t inner,int32_t outer,int32_t segments,
    const int16_t angles[4],int32_t sx,int32_t sy,const uint8_t inner_color[3],
    const uint8_t outer_color[3],int32_t brightness,int32_t abr);
void func_800D0728(pe_addr_t position,int32_t inner,int32_t outer,int32_t segments,
    pe_addr_t angles,int32_t sx,int32_t sy,pe_addr_t inner_color,pe_addr_t outer_color,
    int32_t brightness,int32_t abr);
uint32_t func_80078004(uint32_t value);
uint32_t func_80078134(pe_addr_t input, pe_addr_t output);
/* Host view of 78134's stack-local input/output vectors. */
int PE_NormalizeVectorRetail(const int32_t in[3], int32_t out[3], uint32_t *length);
pe_addr_t func_800787D4(pe_addr_t a, pe_addr_t b, pe_addr_t out);
pe_addr_t func_800799E4(pe_addr_t angles, pe_addr_t out);
void func_8018F05C(void);
void func_8018F92C(pe_addr_t origin);
void func_80193478(void);
void func_801942FC(void);
int func_8018FFF4(pe_addr_t point);
int func_80190124(pe_addr_t point);
int func_80190254(pe_addr_t point,uint32_t radius);
int func_801904B0(pe_addr_t point,uint32_t radius);
void PE_TransitionUpdate(const uint16_t *menu_tail,const uint16_t *subtitle_tail);
int PE_TransitionSubtitle(uint32_t id,uint32_t action,const uint16_t *tail);
void func_80038940(int a0, int a1, int a2, int a3);
void func_801938E8(uint32_t id);
void func_801939B0(uint32_t id);
void func_80190D3C(pe_addr_t object, pe_addr_t context);
void func_80190E04(pe_addr_t object, pe_addr_t context, uint32_t fixed_depth, uint32_t force_draw, uint32_t color_pass);
void func_80191114(pe_addr_t object, pe_addr_t context, uint32_t force_draw, uint32_t mode);
void func_80197BA0(pe_addr_t unused, pe_addr_t model);
void func_8019A318(pe_addr_t unused, pe_addr_t model, uint32_t submodel);
void func_801995BC(pe_addr_t unused, pe_addr_t model, uint32_t submodel);
void func_8019B1D0(pe_addr_t unused, pe_addr_t model, uint32_t submodel);
int32_t func_80079384(pe_addr_t a, pe_addr_t b, pe_addr_t c, pe_addr_t xy0, pe_addr_t xy1, pe_addr_t xy2, pe_addr_t cue, pe_addr_t depth, pe_addr_t flags);
int32_t func_80079414(pe_addr_t a, pe_addr_t b, pe_addr_t c, pe_addr_t d, pe_addr_t xy0, pe_addr_t xy1, pe_addr_t xy2, pe_addr_t xy3, pe_addr_t cue, pe_addr_t depth, pe_addr_t flags);
uint32_t func_800792D4(pe_addr_t point, pe_addr_t output, pe_addr_t flags);
void func_800791D0(pe_addr_t a, pe_addr_t b, pe_addr_t output);
void func_8018F344(pe_addr_t matrix, pe_addr_t eye, pe_addr_t target, pe_addr_t up);
void PE_EffectDirectionCFAA8(const int16_t from[3],const int16_t to[3],pe_addr_t out);
void func_800CFAA8(pe_addr_t pe_a0, pe_addr_t pe_a1, pe_addr_t pe_a2);
void func_800CE9D4(pe_addr_t actor,uint32_t joint,pe_addr_t out);
void PE_ActorJointDirectionCE9D4(pe_addr_t actor,uint32_t joint,int16_t out[3]);
int32_t func_800D3F64(uint32_t id,uint32_t group);
int func_800C6B90(pe_addr_t pe_a0, int a1);
int PE_M0013I_Main(int32_t mode,pe_addr_t data,pe_addr_t extra);
int PE_M0013I_Particle(int32_t mode,pe_addr_t data);
void PE_EffectOffsetCFB7C(pe_addr_t angles,int32_t distance,int16_t out[3]);
void func_800CFB7C(pe_addr_t angles,int32_t distance,pe_addr_t out);
int func_800C62DC(pe_addr_t point,pe_addr_t triangle);
int func_800C6B20(pe_addr_t quad);
int func_800CEB8C(pe_addr_t from,pe_addr_t to,int32_t radius);
typedef struct { int16_t r[9],pad; int32_t t[3]; } PeEffectMatrix;
void func_800C2EAC(unsigned int a0);
void func_800C3098(int32_t colors);
int func_800C2FF0(unsigned char arg0, unsigned char arg1);
void func_800C3238(unsigned blend);
void func_800C608C(short a0, pe_addr_t pe_a1, pe_addr_t pe_a2);
int32_t func_80079244(pe_addr_t point,pe_addr_t screen,pe_addr_t ir0,pe_addr_t flags);
void func_80078E04(pe_addr_t matrix);
void func_80078E94(pe_addr_t matrix);
void func_80078A94(void);
void func_80078B38(void);
pe_addr_t func_8006EC6C(pe_addr_t base, int offset);
int32_t func_80079274(pe_addr_t v0, pe_addr_t v1, pe_addr_t v2, pe_addr_t sxy0,
                      pe_addr_t sxy1, pe_addr_t sxy2, pe_addr_t ir0, pe_addr_t flags);
void func_80079E14(int32_t angle,pe_addr_t dest);
pe_addr_t func_80078C34(pe_addr_t matrix,pe_addr_t vector,pe_addr_t out);
void func_800C3B04(pe_addr_t style);
void func_800CDD0C(int a0, int a1, pe_addr_t pe_a2);
void func_800CDE90(int a0, int a1, pe_addr_t pe_a2);
void PE_EffectQuadC42A4(pe_addr_t style,PeEffectMatrix *matrix,unsigned billboard);
int PE_EffectPointQuadC61A8(const int16_t point[3],const PeEffectMatrix *matrix);
void func_800C42A4(pe_addr_t style,pe_addr_t matrix,unsigned billboard);
void func_800CEDA8(int32_t texture);
void func_800C9C20(pe_addr_t slot,pe_addr_t rec,pe_addr_t data);
void func_800C9C8C(pe_addr_t slot,pe_addr_t rec,pe_addr_t data);
void func_800C9D9C(pe_addr_t slot,pe_addr_t rec,pe_addr_t data);
void func_800C9EA8(pe_addr_t slot,pe_addr_t rec,pe_addr_t data);
void func_800C9FD8(pe_addr_t slot,pe_addr_t rec,pe_addr_t data);
void PE_EffectReadMatrix(pe_addr_t address,PeEffectMatrix *out);
void PE_EffectMeshC71E4(pe_addr_t mesh,const PeEffectMatrix *matrix);
void func_800C71E4(pe_addr_t mesh,pe_addr_t matrix);
void func_800C6D5C(pe_addr_t mesh,uint32_t u,uint32_t v);
void func_800C6EC0(unsigned int first, unsigned int second);
void func_800C6ED8(unsigned int value);
void func_800C6EF8(pe_addr_t pe_record);
void func_800C6F4C(pe_addr_t pe_record);
void func_800C6FA0(pe_addr_t mesh,uint32_t level);
void func_800C70EC(pe_addr_t pe_a0, int a1, int a2, int a3);

/* Main transition scene renderer. */
extern void func_80192800(void);
extern uint32_t PE_TransitionCompactOT(uint32_t empty_start);
extern pe_addr_t func_80084B20(int arg0);
extern int func_80084F8C(pe_addr_t pe_record);
extern int func_800825C0(uint32_t port);
extern uint32_t func_80082680(uint32_t port,int32_t info,int32_t index);
extern void func_800835A4(pe_addr_t pe_arg0, unsigned int arg1, unsigned char arg2);
extern void func_80082974(uint32_t port,pe_addr_t motors,uint32_t count);
extern int func_80083BB8(pe_addr_t record,pe_addr_t alignment);
extern int func_800828F4(uint32_t port,pe_addr_t alignment);
extern int func_80083D04(pe_addr_t record,uint32_t mode,uint32_t lock);
extern int func_8008292C(uint32_t port,uint32_t mode,uint32_t lock);

#endif
