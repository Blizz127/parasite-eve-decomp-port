/*
 * PE-BTL15 / SEW19 — 15DAC room sound and default cases,
 * op 0xA (173F4), op 0x1D (17E20).
 * Translated retail, not matching src/.
 *
 * Authority: build/disc1.candidate.exe SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 *
 * func_80015DAC — 727 words 0x80015DAC..0x80016908, SHA-256
 * bd05f480…a978. D_800910A0[0xEA]. Jump table on *arg0-100,
 * sltiu 311, table at 0x800101B0. Out-of-range and table
 * targets 0x800168F4 are the epilogue `v0=1` (no stores).
 * Live type-6 first 0xEA is key 0x194; type-1 first is 0x193.
 * Both are that nop. Key 0x190 → 0x80016658 is the overlay
 * 12-byte table walk (BTL18). SEW19 restores keys300 and350..353
 * with their complete native sound call graphs. DAY1-15 wires
 * EA200/201/203/204 through original 6D2B8 and 86464/86498/864F8/86770.
 * DAY2 audio dispatch restores EA301/303/401/402/405/409. Matched
 * 86948 and 80AC4 are linked from the generated decomp ports. Other
 * unhandled non-default EA cases still need recovery; this is not the
 * complete 727-word dispatcher.
 *
 * func_800173F4 — 7 words 0x800173F4..0x80017410. Zero jal.
 * *arg0 = *arg1; v0=1.
 *
 * func_80017E20 — 18 words 0x80017E20..0x80017E68. Zero jal.
 * if *arg0 != *arg1: gp+0x90 = *(D2F0)+0x9C + (*arg2)<<1.
 * Always v0=1.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "pe_plat/timing.h"

extern void func_80087024(void);
extern void func_800867E4(int);
extern void func_8008682C(int);
extern void func_80086DAC(int);
extern void func_80086DE4(int, int);
extern void func_80086E24(int, int, int);
extern void func_80086E70(int);
extern void func_80086EA8(int, int);
extern void func_80086EE8(int, int, int);
extern void func_80086F34(int);
extern void func_80086F6C(int, int);
extern void func_80086FAC(int, int, int);
extern void func_80086874(int);
extern void func_80086A80(int, int, int, int);
extern void func_800869AC(int);
extern void func_800869E4(int, int);
extern void func_80086B60(int, int, int);
extern void func_80086BB8(int, int, int, int);
extern void func_80086AE4(int);
extern void func_80086B1C(int, int);
extern void func_800867B0(int);

#define GA_EA_TABLE   0x800101B0u
#define GA_EA_NOP     0x800168F4u
#define GA_EA_190     0x80016658u
#define GA_D_8009D2F0 0x8009D2F0u
#define GA_D_8009CE00 0x8009CE00u
#define GA_D_800B0E64 0x800B0E64u
#define GA_D_800B0DB8 0x800B0DB8u
#define GA_D_800B0DB9 0x800B0DB9u
#define GA_D_800B0DFC 0x800B0DFCu
#define GA_D_800B0E00 0x800B0E00u
#define GA_D_8009D300 0x8009D300u
/* Host stand-in for the retail stack locals (loader slot at +0, op-409
 * CD gains at +4..+7).  It used to be 0x80120FC0, inside the 0x80120D00
 * overlay load window; now in the port stack-scratch block
 * 0x801FF400..0x801FF4EF next to the field-VM arg frame
 * (func_80017018_port.c, audited with PE_VMFRAME_AUDIT=1). */
#define GA_EA_SLOT    0x801FF4B0u

static uint32_t ea_value(pe_addr_t args,unsigned index)
{return PE_LoadU32(PE_LoadU32(args+index*4u));}

/* func_8006DB48: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8006DB48_port.c (src/func_8006DB48.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_8006DB9C: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8006DB9C_port.c (src/func_8006DB9C.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_8006DBE0: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8006DBE0_port.c (src/func_8006DBE0.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* Original EA205/206/207 share the channel-record update at16150. */
static int ea_music_volume(pe_addr_t args,uint32_t key)
{
    int handle=func_8006DB9C((int32_t)ea_value(args,1));
    int slot;
    if (handle==-1) return 1;
    if (key==205u)
        func_80086C1C(handle,(int)ea_value(args,2));
    else if (key==206u)
        func_80086C5C(handle,ea_value(args,2)<<1u,ea_value(args,3));
    else
        func_80086CA4(handle,ea_value(args,2)<<1u,ea_value(args,3),ea_value(args,4));
    slot=func_8006DBE0((int32_t)ea_value(args,1));
    handle=func_8006DB9C((int32_t)ea_value(args,1));
    func_8006DB48((uint32_t)slot,ea_value(args,1),(uint32_t)handle,
                  ea_value(args,key-203u));
    return 1;
}

/* SEW19: original sound requests at 162B0 and 164DC..16658.
 * The result goes through the script's output argument, including -1 when
 * the room package has no sound with the requested ID. */
static int ea_sound(pe_addr_t args,uint32_t key)
{
    pe_addr_t actor=0;
    int32_t result;
    if (key==350u) {
        PE_StoreU16(0x800B0DD0u,(uint16_t)ea_value(args,1));
        PE_StoreU8(0x800B0DCEu,(uint8_t)ea_value(args,2));
        PE_StoreU16(0x800B0DD2u,(uint16_t)ea_value(args,3));
        PE_StoreU8(0x800B0DCFu,(uint8_t)ea_value(args,4));
        return 1;
    }
    if (key==300u) {
        result=func_8006DF50(PE_LoadU32(GA_D_800B0E64),ea_value(args,1),
                            ea_value(args,2),ea_value(args,3),ea_value(args,4));
    } else {
        int x,y,z;
        if (key==353u) {
            x=(int16_t)(ea_value(args,2)>>16u);
            y=(int16_t)(ea_value(args,3)>>16u);
            z=(int16_t)(ea_value(args,4)>>16u);
        } else {
            if (key==351u) actor=PE_LoadU32(GA_D_8009D2F0);
            else if (ea_value(args,2)==0u) {
                actor=PE_LoadU32(0x8009D254u);
                if (!actor) return 1;
            } else {
                for (actor=PE_LoadU32(0x8009D20Cu);actor;actor=PE_LoadU32(actor+4u))
                    if (PE_LoadU8(actor+12u)==ea_value(args,2) &&
                        PE_LoadU8(actor+13u)==ea_value(args,3) &&
                        !(PE_LoadU32(actor+0x98u)&16u)) break;
                if (!actor) return 1;
            }
            x=(int16_t)PE_LoadU16(actor+42u);
            y=(int16_t)PE_LoadU16(actor+46u);
            z=(int16_t)PE_LoadU16(actor+50u);
        }
        result=func_8006DCE4(ea_value(args,1),0u,x,y,z);
    }
    PE_StoreU32(PE_LoadU32(args+20u),(uint32_t)result);
    return 1;
}

/* Original 16758: rewind gp+0x90 by 0x20 and store the loader
 * return at task+0x10 so the same EA word retries next tick. */
static int ea_music_yield(uint32_t loader_ret)
{
    PE_StoreU32(GA_D_8009CE00, PE_LoadU32(GA_D_8009CE00) - 0x20u);
    PE_StoreU32(PE_LoadU32(GA_D_8009D300) + 0x10u, loader_ret);
    return 0;
}

/* EA200 at 15E30; EA203 at 15ED0. */
static int ea_music_start(pe_addr_t args, uint32_t key)
{
    uint32_t blocking = ea_value(args, key == 203u ? 3u : 2u) < 1u;
    int second = key == 203u;
    int ret;
    uint32_t slot;
    int handle;

    ret = func_8006D2B8((int)ea_value(args, 1), 1, second, GA_EA_SLOT,
                        (int)blocking);
    if (ret == 1)
        return ea_music_yield((uint32_t)ret);
    slot = PE_LoadU32(GA_EA_SLOT);
    if (slot + 2u < 2u)
        return 1;
    if (key == 200u) {
        handle = func_80086464(PE_LoadU32(GA_D_800B0E00 + slot * 4u));
        PE_StoreU32(PE_LoadU32(args + 20u), (uint32_t)handle);
        func_80086C1C(0, 0x7F);
        (void)func_8006DB48(slot, ea_value(args, 1), (uint32_t)handle, 0x7Fu);
    } else {
        handle = func_800864F8(PE_LoadU32(GA_D_800B0E00 + slot * 4u),
                               ea_value(args, 2));
        PE_StoreU32(PE_LoadU32(args + 20u), (uint32_t)handle);
        (void)func_8006DB48(slot, ea_value(args, 1), (uint32_t)handle,
                            ea_value(args, 2));
    }
    return 1;
}

/* EA201 at 15F78. */
static int ea_music_stop(pe_addr_t args)
{
    int handle = func_8006DB9C((int32_t)ea_value(args, 1));
    int ret;

    if (handle == -1)
        return 1;
    func_80086498((pe_addr_t)handle);
    ret = func_8006D2B8((int)ea_value(args, 1), 0, 0, GA_EA_SLOT, 1);
    if (ret == 1)
        return ea_music_yield((uint32_t)ret);
    return 1;
}

int func_80015DAC_key190_cut(pe_addr_t args)
{
    pe_addr_t base;
    uint32_t word;
    uint32_t count;
    uint32_t i;
    uint32_t want;
    pe_addr_t ent;

    base = PE_LoadU32(GA_D_800B0E64);
    word = PE_LoadU32(base + PE_LoadU32(base + 4u) + 0x30u);
    count = word >> 22;
    if (count == 0u)
        return 1;
    want = PE_LoadU32(PE_LoadU32(args + 4u));
    ent = base + (word & 0x3FFFFFu);
    for (i = 0u; i < count; i++) {
        if ((PE_LoadU8(ent + 3u) & 0x10u) != 0u &&
            (uint32_t)PE_LoadU16(ent + 10u) == want) {
            PE_StoreU8(GA_D_800B0DB8, (uint8_t)PE_LoadU16(ent + 10u));
            PE_StoreU8(GA_D_800B0DB9, PE_LoadU8(ent + 8u));
            PE_StoreU32(GA_D_800B0DFC,
                        base + (PE_LoadU32(ent + 4u) & 0x00FFFFFFu));
            return 1;
        }
        ent += 12u;
    }
    return 1;
}

/* Original16794..16870: EA406/407 append actor animation sound events.
 * Sixteen dynamic slots follow the four built-in Aya records. EA406 uses
 * the same sound in both banks; EA407 takes a separate second-bank sound. */
static int ea_animation_sound(pe_addr_t args, uint32_t key)
{
    uint32_t count = PE_LoadU8(0x800B0CE9u);
    pe_addr_t actor, record;
    if (count >= 16u)
        return 1;
    actor = PE_LoadU32(GA_D_8009D2F0);
    record = 0x800944A8u + count * 8u;
    PE_StoreU8(record, PE_LoadU8(actor + 12u));
    PE_StoreU8(record + 1u, PE_LoadU8(actor + 13u));
    PE_StoreU8(record + 2u, (uint8_t)ea_value(args, 1));
    PE_StoreU8(record + 3u, (uint8_t)ea_value(args, 2));
    PE_StoreU16(record + 4u, (uint16_t)ea_value(args, 3));
    PE_StoreU16(record + 6u, (uint16_t)ea_value(args, key == 406u ? 3u : 4u));
    PE_StoreU8(0x800B0CE9u, (uint8_t)(PE_LoadU8(0x800B0CE9u) + 1u));
    return 1;
}

/* func_8006D24C: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8006D24C_port.c (src/func_8006D24C.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

int func_80015DAC_default_cut(pe_addr_t args)
{
    uint32_t key;
    uint32_t idx;
    pe_addr_t target;

    key = PE_LoadU32(PE_LoadU32(args));
    {
        /* audio2 diagnostics: PE_AUDIO_EA_LOG=path logs every EA key with
         * its first operands; keys that reach the silent fall-through are
         * marked DROPPED. */
        static FILE *ea_log;
        static int ea_init;
        if (!ea_init) {
            const char *p = getenv("PE_AUDIO_EA_LOG");
            ea_init = 1;
            if (p && *p) ea_log = fopen(p, "w");
        }
        if (ea_log) {
            uint32_t i2 = key - 100u;
            pe_addr_t tg = i2 < 311u ? PE_LoadU32(GA_EA_TABLE + i2 * 4u) : 0u;
            int handled = key==200u||key==203u||key==201u||key==204u||(key>=205u&&key<=207u)||
                key==301u||key==303u||key==305u||key==314u||key==300u||(key>=350u&&key<=353u)||
                key==217u||key==401u||key==402u||key==405u||key==409u||key==406u||key==407u||
                key==100u||key==101u||(key>=208u&&key<=216u)||key==302u||key==304u||
                (key>=306u&&key<=313u)||key==408u||key==410u||
                i2>=311u||tg==GA_EA_NOP||tg==GA_EA_190;
            uint32_t op[4];
            for (unsigned k = 0; k < 4u; k++) {
                pe_addr_t pp = PE_RangeIsRam(args + (k + 1u) * 4u, 4u) ?
                               PE_LoadU32(args + (k + 1u) * 4u) : 0u;
                op[k] = PE_RangeIsRam(pp, 4u) ? PE_LoadU32(pp) : 0xDEADu;
            }
            fprintf(ea_log, "vbl=%u EA %u (table %08X) ops %X %X %X %X%s\n",
                    pe_plat_timing_vblank_count(), key, tg, op[0], op[1], op[2], op[3],
                    handled ? "" : "  DROPPED");
            fflush(ea_log);
        }
    }
    if (key==200u || key==203u)
        return ea_music_start(args,key);
    /* audio2 lane: the remaining retail EA keys (src/func_80015DAC.c),
     * which the earlier cut dropped at the silent fall-through below.
     * 408 shares case 200's body (a second music-start label). */
    if (key==408u)
        return ea_music_start(args,200u);
    switch (key) {
    case 100u: func_800867E4((int)ea_value(args,1)); return 1;   /* AKAO pause  */
    case 101u: func_8008682C((int)ea_value(args,1)); return 1;   /* AKAO resume */
    case 208u: func_80086DAC((int)ea_value(args,1)); return 1;
    case 209u: func_80086DE4((int)(ea_value(args,1)*2u),(int)ea_value(args,2)); return 1;
    case 210u: func_80086E24((int)(ea_value(args,1)*2u),(int)ea_value(args,2),(int)ea_value(args,3)); return 1;
    case 211u: func_80086E70((int)ea_value(args,1)); return 1;
    case 212u: func_80086EA8((int)(ea_value(args,1)*2u),(int)ea_value(args,2)); return 1;
    case 213u: func_80086EE8((int)(ea_value(args,1)*2u),(int)ea_value(args,2),(int)ea_value(args,3)); return 1;
    case 214u: func_80086F34((int)ea_value(args,1)); return 1;
    case 215u: func_80086F6C((int)(ea_value(args,1)*2u),(int)ea_value(args,2)); return 1;
    case 216u: func_80086FAC((int)(ea_value(args,1)*2u),(int)ea_value(args,2),(int)ea_value(args,3)); return 1;
    case 302u: (void)func_800868F0(ea_value(args,1),ea_value(args,2),ea_value(args,3)); return 1;
    case 304u: func_80086874((int)ea_value(args,1)); return 1;
    case 306u: (void)func_80086A28(ea_value(args,1),ea_value(args,2),ea_value(args,3)); return 1;
    case 307u: func_80086A80((int)ea_value(args,1),(int)ea_value(args,2),(int)(ea_value(args,3)*8u),(int)ea_value(args,4)); return 1;
    case 308u: func_800869AC((int)ea_value(args,1)); return 1;
    case 309u: func_800869E4((int)(ea_value(args,1)*2u),(int)ea_value(args,2)); return 1;
    case 310u: func_80086B60((int)ea_value(args,1),(int)ea_value(args,2),(int)ea_value(args,3)); return 1;
    case 311u: func_80086BB8((int)ea_value(args,1),(int)ea_value(args,2),(int)(ea_value(args,3)*2u),(int)ea_value(args,4)); return 1;
    case 312u: func_80086AE4((int)ea_value(args,1)); return 1;
    case 313u: func_80086B1C((int)(ea_value(args,1)*2u),(int)ea_value(args,2)); return 1;
    case 410u: func_800867B0((int)ea_value(args,1)); return 1;
    default: break;
    }
    if (key==201u)
        return ea_music_stop(args);
    if (key==204u) {
        (void)func_80086770(ea_value(args,1));
        return 1;
    }
    if (key>=205u && key<=207u)
        return ea_music_volume(args,key);
    /* Original162F0/16338: stop an effect, or fade its volume. The
     * duration is doubled before the matching 86948 leaf masks it to8 bits. */
    if (key == 301u) {
        func_800866A4(ea_value(args, 1), ea_value(args, 2));
        return 1;
    }
    if (key == 303u) {
        func_80086948((int)ea_value(args, 1), (int)ea_value(args, 2),
                       (int)(ea_value(args, 3) << 1u), (int)ea_value(args, 4));
        return 1;
    }
    /* Original16384/164CC: EA305 scales duration before the audio leaf. */
    if (key==305u) {
        (void)func_800868AC(ea_value(args,1)<<1u,ea_value(args,2));
        return 1;
    }
    if (key==314u) {
        func_80087024();
        return 1;
    }
    if (key==300u || (key>=350u && key<=353u))
        return ea_sound(args,key);
    if (key == 217u) {
        func_8006D24C();
        return 1;
    }
    /* Original166E0..16780: load a bank without starting playback.
     * Busy retries leave the output operand untouched, rewind this EA word,
     * and delay its task. Completed loads publish the signed slot/status. */
    if (key == 401u || key == 402u) {
        int result = func_8006D2B8((int)ea_value(args, 1), 1, key == 402u,
                                    GA_EA_SLOT, ea_value(args, 2) == 0u);
        if (result == 1)
            return ea_music_yield((uint32_t)result);
        PE_StoreU32(PE_LoadU32(args + 20u), PE_LoadU32(GA_EA_SLOT));
        return 1;
    }
    if (key == 405u) {
        /* Original16780: select the second animation-sound bank this frame. */
        PE_StoreU8(0x800B0CEAu, 1u);
        return 1;
    }
    if (key == 409u) {
        /* Original16870: stack-local CD gains [volume,0,volume,0].
         * The host scratch is disjoint from the loader's slot above. */
        const pe_addr_t gains = GA_EA_SLOT + 4u;
        uint8_t volume = (uint8_t)ea_value(args, 1);
        PE_StoreU8(gains + 3u, 0u);
        PE_StoreU8(gains + 1u, 0u);
        PE_StoreU8(0x800B0DBEu, volume);
        PE_StoreU8(gains + 2u, volume);
        PE_StoreU8(gains, volume);
        (void)func_80080AC4(gains);
        return 1;
    }
    if (key == 406u || key == 407u)
        return ea_animation_sound(args, key);
    idx = key - 100u;
    if (idx >= 311u)
        return 1;
    target = PE_LoadU32(GA_EA_TABLE + idx * 4u);
    if (target == GA_EA_NOP)
        return 1;
    if (target == GA_EA_190)
        return func_80015DAC_key190_cut(args);
    /* Other 15DAC cases are not this cut. */
    return 1;
}

int func_800173F4(pe_addr_t args)
{
    PE_StoreU32(PE_LoadU32(args), PE_LoadU32(PE_LoadU32(args + 4u)));
    return 1;
}

/* func_80017E20: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80017E20_port.c (src/func_80017E20.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
