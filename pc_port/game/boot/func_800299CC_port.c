/*
 * PE-CH1 — func_800299CC_consume_cut: BTL1 D_8009D28C 6→0 consume
 * (named cut of the battle/field tick; not the 456-byte dispatcher).
 *
 * Full retail body starts at 0x800299CC (addiu $sp, -456) and runs
 * far past this cut. This translation is ONLY the consume-edge prefix:
 *
 *   0x800299CC..0x80029A0C exclusive (16 words / 0x40, file 0x1A1CC).
 *   First excluded word: lw $v0, 0x4C($a0) at 0x80029A0C (reload of
 *   the same flag word; not a consume store).
 *
 * Words dumped from SHA-1-exact build/disc1.candidate.exe
 * (452fb033f2eaa4b18aa20a5bca60b8125af3a37b). yaml [0x11718, asm]
 * (before matching 2F970 @ file 0x20170). This worktree has no
 * era/asm split, so the body cannot yet be a matching src/ unit.
 *
 * ROM (gp = 0x8009CD70):
 *
 *   lw    a0, 0x508($gp)          D_8009D278 current record
 *   addiu sp, -456                (frame; not guest-observable here)
 *   ... save s1/ra/s0 ...
 *   lw    v0, 0x4C(a0)
 *   lui   a1, 0x8                 0x00080000
 *   and   v0, a1
 *   beqz  v0, 0x80029A20          skip consume (past exclusive end)
 *   addiu s1, 1                   delay (register only)
 *   lw    v1, 0x51C($gp)          D_8009D28C
 *   addiu v0, 6
 *   bne   v1, v0, 0x80029A0C      skip consume (lands on exclusive end)
 *   addiu v0, 6                   delay: value stored by sb
 *   sb    v0, 0x10C($gp)          D_8009CE7C = 6
 *   sw    zero, 0x51C($gp)        D_8009D28C = 0
 *
 * Both guards required: record+0x4C & 0x00080000, and mode == 6.
 * Skip paths in this cut leave mode and gp+0x10C untouched.
 *
 * Inverse handshake at 0x80029A20 (lbu gp+0x10C==6 → sb 0 / restore
 * mode 6) is AFTER the exclusive end; not this cut. Later dispatcher
 * stores (Aya reload, mode!=0 branch @0x80029A64) are out of scope.
 *
 * Sole jal of the FULL tick: 0x800355E8. Opcode 0x89 matching leaf
 * src/func_80017FF0.c writes 6; opcode 0x94 native func_80019154
 * reads it. This cut is the consumer.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game_port.h"

#define GA_D_8009D278 0x8009D278u /* gp+0x508 current record */
#define GA_D_8009D28C 0x8009D28Cu /* gp+0x51C mode word */
#define GA_D_8009CE7C 0x8009CE7Cu /* gp+0x10C consume-edge byte */
#define RECORD_FLAG   0x00080000u

void func_800299CC_consume_cut(void)
{
    pe_addr_t record;
    unsigned int flags;

    record = PE_LoadU32(GA_D_8009D278);
    flags = PE_LoadU32(record + 0x4Cu);
    if ((flags & RECORD_FLAG) == 0u) {
        return;
    }
    if (PE_LoadU32(GA_D_8009D28C) != 6u) {
        return;
    }
    PE_StoreU8(GA_D_8009CE7C, 6u);
    PE_StoreU32(GA_D_8009D28C, 0u);
}

/*
 * PE-BTL53 — 299CC after the consume exclusive end, through the
 * 4D4 idle gate. Not the 2A470/1D340 clip body.
 *
 *   0x80029A0C  reload record+0x4C & 0x00080000
 *   0x80029A20  inverse: flag clear && edge==6 → edge=0, mode=6
 *   0x80029A38  *D254 → D278; sw 0 → D_8009D230
 *   0x80029A5C  jal 5C498 not this cut
 *   0x80029A6C  mode!=0 → 2A7F8 (mode switch not this cut)
 *   0x80029A7C  4D4==0 → 2A7F8
 *
 * Live after 293F4(0): 4D4 is 0, so 1D340/27D14 (rec=4) are
 * not reached. Do not invent 4D4 or rec=4.
 */
#define GA_D_8009D254 0x8009D254u
#define GA_D_8009D230 0x8009D230u
#define GA_D_8009D244 0x8009D244u /* gp+0x4D4 */
#define GA_D_8009D20C 0x8009D20Cu
#define GA_D_8009D294 0x8009D294u
#define GA_D_8009CE3C 0x8009CE3Cu
#define GA_D_8009D1A0 0x8009D1A0u

void func_800299CC_after_consume_cut(void)
{
    pe_addr_t record;
    pe_addr_t aya;
    unsigned int flags;

    record = PE_LoadU32(GA_D_8009D278);
    flags = PE_LoadU32(record + 0x4Cu);
    if ((flags & RECORD_FLAG) == 0u) {
        if (PE_LoadU8(GA_D_8009CE7C) == 6u) {
            PE_StoreU8(GA_D_8009CE7C, 0u);
            PE_StoreU32(GA_D_8009D28C, 6u);
        }
    }

    aya = PE_LoadU32(GA_D_8009D254);
    /* Kuseg *D254 is the KSEG0 word (BTL-RAM-LOW). */
    if (aya < 0x80000000u)
        aya |= 0x80000000u;
    PE_StoreU32(GA_D_8009D230, 0u);
    PE_StoreU32(GA_D_8009D278, PE_LoadU32(aya));

    /* 29A5C jal 5C498; 29A68 sh v0, gp+0x534. Before mode!=0. */
    PE_StoreU16(0x8009D2A4u, (uint16_t)func_8005C498(0x800A76D8u));

    if (PE_LoadU32(GA_D_8009D28C) != 0u)
        return;
    if (PE_LoadU8(GA_D_8009D244) == 0u)
        return;
}

/* func_80062CB8: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80062CB8_port.c (src/func_80062CB8.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_80062CC4: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80062CC4_port.c (src/func_80062CC4.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

void func_800512AC_cmd10_cut(void)
{
    func_800512AC(10, 0u);
}

/* Compatibility entry now runs the complete original mode6 handler. */
void func_8002BC90_mode6_cut(void)
{
    func_8002BC90();
}

/* Original 34DE0..34F10 timed message and background border. */
static void pe_controller_status_message(void)
{
    unsigned state=PE_LoadU8(0x8009D1CEu);
    if(state==1u) {
        const pe_addr_t terminator=0x80122388u; /* Original stack-local -1 list. */
        PE_StoreU16(terminator,65535u);
        func_800374E8();
        func_80037454(func_8005BCB0()?20:97,PE_LoadU8(0x8009CE80u)<2u?15:195,0,0);
        func_800375E0(0,2u,terminator);
        state=PE_LoadU8(0x8009D1CEu);
        PE_StoreU8(0x800BCEA8u,2u);PE_StoreU8(0x8009CE88u,75u);
        PE_StoreU32(0x800BCEACu,PE_LoadU32(0x8009D1F8u));
        PE_StoreU8(0x8009D1CEu,(uint8_t)(state+1u));
        PE_StoreU32(0x800BCEB4u,PE_LoadU32(0x800BCEB4u)|0x02000000u);
    } else if(state==2u && !PE_LoadU8(0x8009CE88u)) {
        func_800374E8();PE_StoreU8(0x8009D1CEu,0u);
    }
    PE_StoreU8(0x8009CE88u,(uint8_t)(PE_LoadU8(0x8009CE88u)-1u));
    /* 5E894 stores the menu drawing origin. */
    PE_StoreU32(0x8009D124u,0u);PE_StoreU32(0x8009D128u,PE_LoadU8(0x8009CE80u)<2u?11u:191u);
    func_80061C34(320u,20u,0u,0u);
}

/* Original 2AA24..2AA80 join, with all three conditional calls. */
void func_8002A7F8_join_cut(void)
{
    if(PE_LoadU8(0x8009D1CEu) && !PE_LoadU32(GA_D_8009D28C))pe_controller_status_message();
    if(PE_LoadU8(GA_D_8009D244))func_80033A40();
    if((int16_t)PE_LoadU16(0x8009D2A4u))func_80067CBC();
}

void func_800299CC_mode_switch_cut(void)
{
    uint32_t mode;

    mode = PE_LoadU32(GA_D_8009D28C);
    if (mode == 1u) {
        int8_t next=func_80025EE8_attack_cut();
        PE_StoreU32(GA_D_8009D28C,(uint32_t)(int32_t)next);
        if (!next) D_8009D1A0&=~4u;
    } else if (mode == 6u)
        func_8002BC90_mode6_cut();
    else if (mode == 3u)
        func_8002A7F8_mode3_cut();
    else if (mode == 2u)
        func_8002B0E8();
    else if (mode == 4u)
        func_8002B94C();
    else if (mode == 5u)
        func_8002F0B0();
    else if (mode == 7u)
        func_8002D1F0();
    else if (mode == 8u)
        func_8002DC58();
    if (PE_Port_ShouldStop()) return;
    /* mode 9 and the rest fall through to 2AA24. */
    func_8002A7F8_join_cut();
}

/*
 * PE-BTL98 — 299CC mode==0 && 4D4!=0 body reaches jal 1D340
 * at 0x8002A4FC. s1 is 1 from the consume delay at 0x800299F0
 * unless 2A444/2A4BC overwrite it (mode-1 / bit-0x4000 arms,
 * not first retail entry). a0 = s1. Do not store 4D4 or HP.
 */
/*
 * PE-BTL109 — 21054. 9 words. rec+0x4C bit 0x10000 → -1,
 * else lb gp+0xCC. 299CC @ 2A470: blez skips 236E8.
 */
int func_80021054(void)
{
    pe_addr_t rec = PE_LoadU32(GA_D_8009D278);

    if (rec != 0u && (PE_LoadU32(rec + 0x4Cu) & 0x10000u) != 0u)
        return -1;
    return (int)(int8_t)PE_LoadU8(GA_D_8009CE3C);
}

/* 29A84..2A470: ready cue, AT pulse and command-entry input. The return
 * value is the retail s1 argument subsequently passed to 1D340. */
uint32_t func_800299CC_ready_input(void)
{
    pe_addr_t rec=PE_LoadU32(GA_D_8009D278);
    pe_addr_t gauge, cursor;
    uint32_t pressed, bank, phase;
    static const uint8_t colors[4][2][3]={
        {{0,70,130},{159,255,249}},{{80,163,190},{80,163,190}},
        {{159,255,249},{0,70,130}},{{80,163,190},{80,163,190}}};
    unsigned i;
    if (!rec || PE_LoadU16(rec+0x10u)<9000u) return 1u;
    PE_StoreU16(rec+0x10u,9000u);
    if (!PE_LoadU8(0x8009D288u)) {
        pe_addr_t sound=PE_LoadU32(0x800B0E08u);
        func_80071A64(D_8009D250);
        if (sound) (void)func_8006DF50(sound,0x454u,0u,0x80u,0x7Fu);
        PE_StoreU8(0x8009D288u,1u);
    }
    rec=PE_LoadU32(GA_D_8009D278);
    if ((PE_LoadU32(rec+0x4Cu)&0x2000u) || !PE_LoadU8(0x8009D2A0u)) return 1u;
    bank=PE_LoadU32(0x8009CDDCu); phase=D_8009D250&3u;
    gauge=0x800B00E8u+bank*36u; cursor=0x800B6920u+bank*28u;
    for (i=0;i<3;i++) {
        PE_StoreU8(gauge+4u+i,colors[phase][0][i]);
        PE_StoreU8(gauge+12u+i,colors[phase][1][i]);
        PE_StoreU8(gauge+20u+i,colors[phase][0][i]);
        PE_StoreU8(gauge+28u+i,colors[phase][1][i]);
        PE_StoreU8(cursor+12u+i,colors[phase][1][i]);
    }
    if ((int8_t)func_80021054()) return 1u;
    pressed=PE_LoadU32(0x8009D1F4u);
    if (pressed&0x200u) PE_StoreU8(0x8009D1F0u,0u);
    else if (pressed&0x80u) {
        PE_StoreU8(0x8009D1F0u,1u);func_8005C174(1);func_80067CBC();
    } else return 1u;
    func_800866A4(0u,0xFFu);
    PE_StoreU32(GA_D_8009D28C,1u); PE_StoreU32(0x8009D290u,0u);
    D_8009D1A0|=4u;
    PE_StoreU8(0x8009D2B0u,(uint8_t)func_8002156C());
    func_80020F18();
    func_80043240(PE_LoadU8(0x8009D2B0u)?0:1);
    return 0u;
}

/* 25BD8: enqueue the original automatic counterattack. */
void func_80025BD8(pe_addr_t enemy)
{
    pe_addr_t rec=PE_LoadU32(GA_D_8009D278),weapon=PE_LoadU32(rec+104u);
    pe_addr_t target=0u,aya=PE_LoadU32(GA_D_8009D254);
    uint32_t flags=PE_LoadU32(enemy+152u);
    unsigned i;
    PE_StoreU8(0x8009D294u,0u);
    PE_StoreU8(0x8009D1DCu,(uint8_t)(PE_LoadU32(weapon+16u)&15u));
    for (i=0;i<4;i++) PE_StoreU8(0x8009CE38u+i,PE_LoadU8(weapon+20u+i));
    if ((flags&0x40000000u) && !PE_LoadU8(PE_LoadU32(enemy)+175u)) {
        /* The original tests the source's flag again inside this walk. */
        for (target=PE_LoadU32(GA_D_8009D20C);target;target=PE_LoadU32(target+4u)) {
            pe_addr_t body=PE_LoadU32(target);
            if (target!=aya && target!=enemy && body && (int32_t)PE_LoadU32(body+16u)>0 &&
                PE_LoadU8(target+13u)==PE_LoadU8(enemy+13u) && !(PE_LoadU32(enemy+152u)&0x40000000u)) break;
        }
    } else if ((flags&0x6000u) && (int8_t)PE_LoadU8(PE_LoadU32(enemy)+5u)==3) {
        for (target=PE_LoadU32(GA_D_8009D20C);target;target=PE_LoadU32(target+4u)) {
            pe_addr_t body=PE_LoadU32(target);
            if (target!=aya && target!=enemy && body && (int32_t)PE_LoadU32(body+16u)>0 &&
                PE_LoadU32(target+396u)==enemy && (int8_t)PE_LoadU8(body+5u)==1) break;
        }
    } else if (!(flags&0x4000u)) target=enemy;
    if (!target) return;
    flags=PE_LoadU32(PE_LoadU32(PE_LoadU32(GA_D_8009D278)+104u)+16u)&0xC0u;
    if (flags==0x40u || flags==0xC0u) {
        pe_addr_t command=0x800BE830u+PE_LoadU8(0x8009CE3Cu)*8u;
        PE_StoreU8(0x8009D1DCu,0u);PE_StoreU32(command,target);PE_StoreU16(command+4u,2u);
        PE_StoreU16(command+6u,(uint16_t)(int16_t)(int8_t)PE_LoadU8(0x8009D2D8u));
        PE_StoreU8(0x8009CE3Cu,(uint8_t)(PE_LoadU8(0x8009CE3Cu)+1u));
    } else while (PE_LoadU8(0x8009D1DCu)) {
        pe_addr_t command=0x800BE830u+PE_LoadU8(0x8009CE3Cu)*8u;
        PE_StoreU8(0x8009D1DCu,(uint8_t)(PE_LoadU8(0x8009D1DCu)-1u));
        PE_StoreU32(command,target);PE_StoreU16(command+4u,1u);
        PE_StoreU16(command+6u,(uint16_t)(int16_t)(int8_t)PE_LoadU8(0x8009D2D8u));
        PE_StoreU8(0x8009CE3Cu,(uint8_t)(PE_LoadU8(0x8009CE3Cu)+1u));
    }
    func_80021128();rec=PE_LoadU32(GA_D_8009D278);
    PE_StoreU32(rec+76u,PE_LoadU32(rec+76u)|0x200000u);
}

void func_800306E0(pe_addr_t enemy)
{
    pe_addr_t body=PE_LoadU32(enemy),rec=PE_LoadU32(GA_D_8009D278);
    int difference;
    unsigned threshold,multiplier,roll;
    if ((int32_t)PE_LoadU32(body+16u)<=0) return;
    difference=(int8_t)(PE_LoadU8(rec+4u)-PE_LoadU8(body+4u));
    threshold=PE_LoadU16(0x800C0E28u);
    if (difference>0) multiplier=10u;
    else if (!difference) {threshold*=3u;multiplier=50u;}
    else if (difference==-1) {threshold*=3u;multiplier=100u;}
    else multiplier=50u;
    roll=func_80071A54()%100u;
    if (roll*multiplier<threshold) func_80025BD8(enemy);
}

/* 299CC's 2A5BC..2A7F8 continuation: finish hit/equip animations and
 * restore the interrupted action at its saved frame. */
void func_800299CC_player_animation(void)
{
    pe_addr_t aya=PE_LoadU32(GA_D_8009D254),rec=PE_LoadU32(GA_D_8009D278);
    if (PE_LoadU8(aya+15u)==PE_LoadU16(aya+26u) && PE_LoadU8(aya+14u)<4u) {
        unsigned restore=PE_LoadU16(0x8009D298u);
        if (!restore) {
            func_8001A680_command_cut(aya,PE_LoadU8(rec+18u));
            if (!(PE_LoadU8(0x800B0CE6u)&1u) && !(int8_t)func_80021054() &&
                !PE_LoadU32(GA_D_8009D28C) &&
                (PE_LoadU32(PE_LoadU32(PE_LoadU32(GA_D_8009D278)+104u)+16u)&0x8000u)) {
                pe_addr_t enemy=PE_LoadU32(0x8009D1D0u);
                if (enemy) {func_800306E0(enemy);PE_StoreU32(0x8009D1D0u,0u);}
            }
        } else {
            if (restore>=2u) PE_StoreU32(aya+152u,PE_LoadU32(aya+152u)|0x100u);
            PE_StoreU16(0x8009D298u,0u);
            func_8001A680_command_cut(PE_LoadU32(GA_D_8009D254),PE_LoadU8(0x8009D29Au));
            aya=PE_LoadU32(GA_D_8009D254);
            PE_StoreU32(aya+20u,PE_LoadU32(0x8009D29Cu));
            PE_StoreU32(aya+24u,PE_LoadU32(0x8009D29Cu)-65536u);
        }
    }
    aya=PE_LoadU32(GA_D_8009D254);
    if (PE_LoadU8(aya+14u)==13u && !(PE_LoadU8(0x800B0CE6u)&1u)) {
        PE_StoreU8(PE_LoadU32(GA_D_8009D278)+18u,4u);
        if (PE_LoadU32(aya+152u)&0x100u) {
            func_8006DE80(0x453,0,(int16_t)PE_LoadU16(aya+42u),
                (int16_t)PE_LoadU16(aya+46u),(int16_t)PE_LoadU16(aya+50u));
            aya=PE_LoadU32(GA_D_8009D254);PE_StoreU32(aya+152u,PE_LoadU32(aya+152u)&~0x100u);
        }
        aya=PE_LoadU32(GA_D_8009D254);
        if (PE_LoadU8(aya+15u)==PE_LoadU16(aya+26u)) {
            func_8001A680_command_cut(aya,PE_LoadU8(PE_LoadU32(GA_D_8009D278)+18u));
            PE_StoreU32(0x8009D2E8u,PE_LoadU32(0x8009D2E8u)&~1u);
        }
    }
    PE_StoreU32(0x8009D1E8u,PE_LoadU32(0x8009D1E8u)+1u);
}

void func_800299CC_damage_entry_cut(void)
{
    pe_addr_t actor;
    pe_addr_t aya;
    uint32_t update;

    if (PE_LoadU32(GA_D_8009D28C) != 0u)
        return;
    if (PE_LoadU8(GA_D_8009D244) == 0u)
        return;
    update=func_800299CC_ready_input();
    /* 2A470: 21054>0 → 21DE0 (unless 0x4000 && !D1A0.100), then 236E8. */
    if (func_80021054() > 0) {
        pe_addr_t rec = PE_LoadU32(GA_D_8009D278);
        uint32_t flags = (rec != 0u) ? PE_LoadU32(rec + 0x4Cu) : 0u;

        if ((flags & 0x4000u) == 0u ||
            ((D_8009D1A0|PE_LoadU32(GA_D_8009D1A0)) & 0x100u) != 0u) {
            func_80021DE0();
            update=2u;
        }
        if (PE_LoadU8(GA_D_8009D294) != 0u)
            func_800236E8();
    }
    func_8001D340(update);
    /* 2A504: walk D20C, jal 27D14 on non-Aya actors with a body. */
    aya = PE_LoadU32(GA_D_8009D254);
    actor = PE_LoadU32(GA_D_8009D20C);
    while (actor != 0u) {
        pe_addr_t next = PE_LoadU32(actor + 4u);

        if (actor != aya && PE_LoadU32(actor) != 0u)
            func_80027D14(actor);
        actor = next;
    }
    func_800299CC_player_animation();
    func_8002A7F8_join_cut();
}

/* ── func_800299CC: complete battle tick, line-for-line from the matched
 * src/func_800299CC.c (audit items 1-3).  Replaces the composed cuts
 * (consume / after_consume / mode_switch / damage_entry) at the 355E8 call
 * site: restores the D_8009D1AC & 0x300 -> func_8003495C call, the
 * D_8009D23C script-command pair, the D_8009D235 -> func_80033430 call and
 * the full mode-3 body (item 0x12 consume, rec+0x56/+0x66 countdowns, enemy
 * walk).  func_80025EE8 is the complete port func_80025EE8_attack_cut. */
extern int func_80067CBC(void);
void func_8001A680(pe_addr_t body, unsigned int id);   /* decomp_hand/absent_lo_actor_port.c */
extern int func_8006DF50(int a0, int a1, int a2, int a3, int a4);
extern int func_8006DE80(int a0, int a1, short a2, short a3, short a4);
extern void func_8002B0E8(void);
extern void func_8002B29C(void);
extern void func_8002B94C(void);
extern void func_8002F0B0(void);
extern void func_8002BC90(void);
extern void func_8002D1F0(void);
extern void func_8002DC58(void);
extern void func_80034DE0(void);
extern void func_80033A40(void);
extern void func_80033430(void);
extern void func_8003495C(void);
extern void func_80021DE0(void);
extern void func_800236E8(void);
extern void func_80020F18(void);
extern void func_80043240(int a0);
extern void func_800866A4(unsigned int a0, unsigned int a1);
extern void func_8005C174(int32_t battle);
extern int32_t func_8002156C(void);

#define BT_REC()   PE_LoadU32(GA_D_8009D278)
#define BT_AYA()   PE_LoadU32(GA_D_8009D254)
#define BT_MODE()  PE_LoadU32(GA_D_8009D28C)

static void bt_colour(pe_addr_t base, unsigned slot, uint8_t r, uint8_t g, uint8_t b)
{
    PE_StoreU8(base + slot + 0u, r); PE_StoreU8(base + slot + 1u, g); PE_StoreU8(base + slot + 2u, b);
}

/* for (e = D_8009D20C; e; e = e->next) if (e != D_8009D254 && e->f0) func_80027D14(e); */
static void bt_enemy_walk(void)
{
    pe_addr_t e;
    for (e = PE_LoadU32(GA_D_8009D20C); e != 0u; e = PE_LoadU32(e + 4u)) {
        if (e != BT_AYA() && PE_LoadU32(e) != 0u)
            func_80027D14(e);
        if (PE_Port_ShouldStop()) return;
    }
}

void func_800299CC(void)
{
    uint32_t s = 1u;
    pe_addr_t rec, aya;

    if (PE_LoadU32(BT_REC() + 0x4Cu) & 0x80000u) {
        if (BT_MODE() == 6u) { PE_StoreU8(GA_D_8009CE7C, 6u); PE_StoreU32(GA_D_8009D28C, 0u); }
    }
    if (!(PE_LoadU32(BT_REC() + 0x4Cu) & 0x80000u)) {
        if (PE_LoadU8(GA_D_8009CE7C) == 6u) { PE_StoreU8(GA_D_8009CE7C, 0u); PE_StoreU32(GA_D_8009D28C, 6u); }
    }
    PE_StoreU32(GA_D_8009D230, 0u);
    PE_StoreU32(GA_D_8009D278, PE_LoadU32(BT_AYA()));
    PE_StoreU16(0x8009D2A4u, (uint16_t)func_8005C498(0x800A76D8u));
    if (BT_MODE() == 0u && PE_LoadU8(GA_D_8009D244) != 0u) {
        rec = BT_REC();
        if (PE_LoadU16(rec + 0x10u) >= 9000u) {
            PE_StoreU16(rec + 0x10u, 9000u);
            if (PE_LoadU8(0x8009D288u) == 0u) {
                func_80071A64(PE_LoadU32(0x8009D250u));
                if (PE_LoadU32(0x800B0E08u) != 0u)
                    (void)func_8006DF50((int)PE_LoadU32(0x800B0E08u), 0x454, 0, 0x80, 0x7F);
                PE_StoreU8(0x8009D288u, 1u);
            }
            if (!(PE_LoadU32(BT_REC() + 0x4Cu) & 0x2000u) && (int8_t)PE_LoadU8(0x8009D2A0u) != 0) {
                uint32_t ph = PE_LoadU32(0x8009D250u) & 3u;
                pe_addr_t g4 = 0x800B00E8u + PE_LoadU32(0x8009CDDCu) * 36u;
                pe_addr_t p28 = 0x800B6928u + PE_LoadU32(0x8009CDDCu) * 28u;
                if (ph == 0u) {
                    bt_colour(g4, 4u, 0, 0x46, 0x82);   bt_colour(g4, 12u, 0x9F, 0xFF, 0xF9);
                    bt_colour(g4, 20u, 0, 0x46, 0x82);  bt_colour(g4, 28u, 0x9F, 0xFF, 0xF9);
                    bt_colour(p28, 4u, 0x9F, 0xFF, 0xF9);
                } else if (ph == 1u || ph == 3u) {
                    bt_colour(g4, 4u, 0x50, 0xA3, 0xBE);  bt_colour(g4, 12u, 0x50, 0xA3, 0xBE);
                    bt_colour(g4, 20u, 0x50, 0xA3, 0xBE); bt_colour(g4, 28u, 0x50, 0xA3, 0xBE);
                    bt_colour(p28, 4u, 0x50, 0xA3, 0xBE);
                } else {
                    bt_colour(g4, 4u, 0x9F, 0xFF, 0xF9); bt_colour(g4, 12u, 0, 0x46, 0x82);
                    bt_colour(g4, 20u, 0x9F, 0xFF, 0xF9); bt_colour(g4, 28u, 0, 0x46, 0x82);
                    bt_colour(p28, 4u, 0, 0x46, 0x82);
                }
                if ((int8_t)func_80021054() == 0) {
                    int k = 0;
                    uint32_t pad = PE_LoadU32(0x8009D1F4u);
                    if (pad & 0x200u) {
                        PE_StoreU8(0x8009D1F0u, 0u);
                        k = 1;
                    } else if (pad & 0x80u) {
                        k = 1;
                        PE_StoreU8(0x8009D1F0u, 1u);
                        func_8005C174(1);
                        (void)func_80067CBC();
                    }
                    if (k) {
                        func_800866A4(0u, 0xFFu);
                        PE_StoreU32(GA_D_8009D28C, 1u);
                        PE_StoreU32(0x8009D290u, 0u);
                        D_8009D1A0 |= 4u;
                        s = 0u;
                        PE_StoreU8(0x8009D2B0u, (uint8_t)(int8_t)func_8002156C());
                        func_80020F18();
                        func_80043240((int8_t)PE_LoadU8(0x8009D2B0u) != 0 ? 0 : 1);
                    }
                }
            }
        }
        if ((int8_t)func_80021054() > 0) {
            if (!(PE_LoadU32(BT_REC() + 0x4Cu) & 0x4000u) || (D_8009D1A0 & 0x100u)) {
                func_80021DE0();
                s = 2u;
            }
            if (PE_LoadU8(GA_D_8009D294) != 0u)
                func_800236E8();
        }
        if (PE_LoadU32(0x8009D1ACu) & 0x300u)
            func_8003495C();
        func_8001D340(s);
        if (PE_Port_ShouldStop()) return;
        bt_enemy_walk();
        if (PE_Port_ShouldStop()) return;
        if (PE_LoadU8(0x8009D23Cu) != 0u) {
            (void)func_8006F6D4(PE_LoadU32(0x8009D2FCu), 0u, 1u, 0x800B8A90u, 0u, 0u);
            (void)func_8006F6D4(PE_LoadU32(0x8009D2FCu), 0u, 0u, 1u, 0u, 0u);
            PE_StoreU8(0x8009D23Cu, 0u);
        }
        if (PE_LoadU8(0x8009D235u) != 0u)
            func_80033430();
        aya = BT_AYA();
        if (PE_LoadU8(aya + 0x0Fu) == PE_LoadU16(aya + 0x1Au) && PE_LoadU8(aya + 0x0Eu) < 4u) {
            if (PE_LoadU16(0x8009D298u) == 0u) {
                func_8001A680(aya, PE_LoadU8(BT_REC() + 0x12u));
                if (!(PE_LoadU8(0x800B0CE6u) & 1u) && (int8_t)func_80021054() == 0 && BT_MODE() == 0u &&
                    (PE_LoadU32(PE_LoadU32(BT_REC() + 0x68u) + 0x10u) & 0x8000u) &&
                    PE_LoadU32(0x8009D1D0u) != 0u) {
                    func_800306E0(PE_LoadU32(0x8009D1D0u));
                    PE_StoreU32(0x8009D1D0u, 0u);
                }
            } else {
                if (PE_LoadU16(0x8009D298u) >= 2u)
                    PE_StoreU32(aya + 0x98u, PE_LoadU32(aya + 0x98u) | 0x100u);
                PE_StoreU16(0x8009D298u, 0u);
                func_8001A680(aya, PE_LoadU8(0x8009D29Au));
                PE_StoreU32(aya + 0x14u, PE_LoadU32(0x8009D29Cu));
                PE_StoreU32(aya + 0x18u, PE_LoadU32(0x8009D29Cu) - 0x10000u);
            }
        }
        aya = BT_AYA();
        if (PE_LoadU8(aya + 0x0Eu) == 0x0Du && !(PE_LoadU8(0x800B0CE6u) & 1u)) {
            PE_StoreU8(BT_REC() + 0x12u, 4u);
            if (PE_LoadU32(aya + 0x98u) & 0x100u) {
                (void)func_8006DE80(0x453, 0, (short)PE_LoadU16(aya + 0x2Au),
                                    (short)PE_LoadU16(aya + 0x2Eu), (short)PE_LoadU16(aya + 0x32u));
                PE_StoreU32(aya + 0x98u, PE_LoadU32(aya + 0x98u) & ~0x100u);
            }
            if (PE_LoadU8(aya + 0x0Fu) == PE_LoadU16(aya + 0x1Au)) {
                func_8001A680(aya, PE_LoadU8(BT_REC() + 0x12u));
                PE_StoreU32(0x8009D2E8u, PE_LoadU32(0x8009D2E8u) & ~1u);
            }
        }
        PE_StoreU32(0x8009D1E8u, PE_LoadU32(0x8009D1E8u) + 1u);
    } else if (BT_MODE() == 1u) {
        int8_t next = func_80025EE8_attack_cut();
        PE_StoreU32(GA_D_8009D28C, (uint32_t)(int32_t)next);
        if (next == 0) D_8009D1A0 &= ~4u;
    } else if (BT_MODE() == 2u) {
        func_8002B0E8();
    } else if (BT_MODE() == 3u) {
        rec = BT_REC();
        if (PE_LoadU32(rec + 0x4Cu) & 0x800u) {
            if ((uint8_t)func_8002AA98())
                PE_StoreU32(BT_REC() + 0x4Cu, PE_LoadU32(BT_REC() + 0x4Cu) & ~0x800u);
        } else if (func_80053E6C(0x12)) {
            if ((uint8_t)func_8002AA98())
                (void)func_8005409C(0x12);
        } else {
            func_8002B29C();
        }
        rec = BT_REC();
        if (PE_LoadU8(rec + 0x56u) != 0u) {
            func_80032B0C(0u, rec + 0x50u);
            PE_StoreU8(BT_REC() + 0x56u, (uint8_t)(PE_LoadU8(BT_REC() + 0x56u) - 1u));
        }
        rec = BT_REC();
        if (PE_LoadU8(rec + 0x66u) != 0u) {
            func_80032B0C(0u, rec + 0x60u);
            PE_StoreU8(BT_REC() + 0x66u, (uint8_t)(PE_LoadU8(BT_REC() + 0x66u) - 1u));
        }
        bt_enemy_walk();
    } else if (BT_MODE() == 4u) {
        func_8002B94C();
    } else if (BT_MODE() == 5u) {
        func_8002F0B0();
    } else if (BT_MODE() == 6u) {
        func_8002BC90();
    } else if (BT_MODE() == 7u) {
        func_8002D1F0();
    } else if (BT_MODE() == 8u) {
        func_8002DC58();
    }
    if (PE_Port_ShouldStop()) return;
    if (PE_LoadU8(0x8009D1CEu) != 0u && BT_MODE() == 0u)
        func_80034DE0();
    if (PE_LoadU8(GA_D_8009D244) != 0u)
        func_80033A40();
    if ((int16_t)PE_LoadU16(0x8009D2A4u) != 0)
        (void)func_80067CBC();
}
