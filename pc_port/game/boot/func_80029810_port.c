/*
 * PE-BTL3 — ROM-word cuts from func_80029810, func_8001A680, and the
 * type-0 command-table writer at 0x8006C140 (func_8006BECC state 6).
 *
 * These are native translations, not matching src/ C.  Authority is
 * pc_port/tools/pe_btl3_29810_tail_oracle.py and
 * pc_port/tools/pe_btl3_command_table_oracle.py against the SHA-1-exact EXE.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "pe_plat/timing.h"
#include "game_port.h"
#include "pe_sdk.h"
#include "pe_guestcode.h"

#define GA_RECORD_P       0x8009D278u
#define GA_ACTOR_P        0x8009D254u
#define GA_COMMAND_TABLE  0x800B0E98u
#define GA_VALUE_D27C     0x8009D27Cu
#define GA_OVERLAY        0x800B0CD8u
#define CALLBACK_1D268    0x8001D268u /* lui8002 + signed addiuD268 */

/*
 * ROM 0x8006C140..0x8006C174 (Writer B clip loop inside func_8006BECC
 * state 6). s4 = D_800B0CD8, s2 = overlay+0x154 package. Each 12-byte
 * directory record stores package+(rec+4 & 0x00FFFFFF) at
 * overlay+0x1C0+idB*4 = D_800B0E98[idB] (type-0 row). Directory setup
 * matches 0x8006C0E4/0xEC/0xF4 and 0x8006C11C..0x130 (section at
 * package+4, count = packed>>22, records at packed&0x3FFFFF). Model
 * bind 0x8006C118 (D_800B0E70) is not this cut.
 *
 * 0x55 / func_80029810 does not jal this writer. The table is a global
 * filled by 6BECC; 1A680 only reads it. Native tests invoke the writer
 * with a planted package so the consumer can use a ROM-shaped pointer.
 */
void func_8006C140_type0_clip_bind(pe_addr_t package)
{
    pe_addr_t dir;
    unsigned int packed;
    unsigned int count;
    unsigned int i;
    pe_addr_t rec;

    dir = package + (PE_LoadU32(package + 4u) & 0x3FFFFFu);
    if (dir < 0x80000000u)
        return;
    packed = PE_LoadU32(dir + 0x10u);
    count = packed >> 22;
    rec = package + (packed & 0x3FFFFFu);
    if (rec < 0x80000000u)
        return;
    for (i = 0; i < count; i++) {
        unsigned int idb = PE_LoadU8(rec + 7u);
        unsigned int ptr = PE_LoadU32(rec + 4u) & 0x00FFFFFFu;
        PE_StoreU32(GA_OVERLAY + 0x1C0u + idb * 4u, package + ptr);
        rec += 12u;
    }
}

extern int func_8006E6A8(int lba, pe_addr_t dest, int sectors);
extern int func_8006E7E8(void);

#define GA_D_800B0DD8  0x800B0DD8u
#define GA_D_800930D8  0x800930D8u
#define MASK_22_BECC   0x003FFFFFu
#define MASK_24_BECC   0x00FFFFFFu

/*
 * func_8006BECC — character texture and model loading, 6BECC..6C1CC.
 *
 * States 1/2 load and poll the texture package at overlay+0x194 from
 * D_800930D8[CE2+3]..[CE2+4]. State 3 uploads its counted +0x28 entries.
 * State 4 @ 0x8006C068: 6E6A8 into overlay+0x154 from
 * D_800930D8[CE2+8]..[CE2+9] + D_800B0DD8.
 * State 5 @ 0x8006C0AC: 6E7E8 poll; -1 retries state 4.
 * State 6 @ 0x8006C0E4: 6C118 B0E70[0] + 6C140 type-0 bind,
 * CE3=CE2, +0xEC=0, overlay &= ~0x200000, return 0.
 * State 0 selects reload on overlay&0x200000, otherwise binds the
 * retained model. State 2 also observes the retail actor-transition gate.
 */
int func_8006BECC(void)
{
    uint8_t state;
    uint8_t ce2;
    pe_addr_t dest;
    pe_addr_t package;
    pe_addr_t section;
    uint32_t packed;
    uint16_t start;
    uint16_t end;
    int status;
    int lba;

    state = PE_LoadU8(GA_OVERLAY + 0xECu);
    ce2 = PE_LoadU8(GA_OVERLAY + 0x0Au);
    dest = PE_LoadU32(GA_OVERLAY + 0x154u);

    /* State 0 @ 0x8006BF34: overlay bit 0x200000 selects state 1;
     * else fall through to state 6 (bind existing dest+0x154). */
    if (state == 0u) {
        if ((PE_LoadU32(GA_OVERLAY) & 0x200000u) != 0u) {
            /* State 1 reloads dest+0x194. No dest means nothing to do. */
            if (PE_LoadU32(GA_OVERLAY + 0x194u) < 0x80000000u)
                return 0;
            state = 1u;
        } else {
            state = 6u;
        }
        PE_StoreU8(GA_OVERLAY + 0xECu, state);
    }

    if (state == 1u) {
        pe_addr_t texture = PE_LoadU32(GA_OVERLAY + 0x194u);
        if (texture < 0x80000000u)
            return 0;
        start = PE_LoadU16(GA_D_800930D8 + (uint32_t)(ce2 + 3u) * 2u);
        end = PE_LoadU16(GA_D_800930D8 + (uint32_t)(ce2 + 4u) * 2u);
        lba = (int)(PE_LoadU32(GA_D_800B0DD8) + start);
        status = func_8006E6A8(lba, texture, (int)end - (int)start);
        if (status != -1)
            PE_StoreU8(GA_OVERLAY + 0xECu, 2u);
        return 1;
    }

    if (state == 2u) {
        uint32_t flags;
        status = func_8006E7E8();
        if (status == -1) {
            PE_StoreU8(GA_OVERLAY + 0xECu, 1u);
            return 1;
        }
        if (status != 0)
            return 1;
        flags = PE_LoadU32(GA_OVERLAY);
        if ((flags & 0x20000u) != 0u) {
            if ((flags & 0x80000u) == 0u && PE_LoadU8(0x8009D25Cu) < 2u)
                return 1;
            PE_StoreU32(GA_OVERLAY, flags | 0x40000u);
        }
        state = 3u;
        PE_StoreU8(GA_OVERLAY + 0xECu, state);
    }

    if (state == 3u) {
        pe_addr_t texture = PE_LoadU32(GA_OVERLAY + 0x194u);
        pe_addr_t metadata = texture + PE_LoadU32(texture + 4u);
        pe_addr_t entry;
        uint32_t issued = 0u;
        packed = PE_LoadU32(metadata + 0x28u);
        entry = texture + (packed & MASK_22_BECC);
        while (issued < (PE_LoadU32(metadata + 0x28u) >> 22)) {
            func_8006E1C0(entry, texture);
            entry += 0x14u;
            issued++;
        }
        /* The host CD adapter copies on issue. Finish these guest-backed
         * uploads before later loaders can reuse the staging package. */
        func_80074DC0(0);
        state = 4u;
        PE_StoreU8(GA_OVERLAY + 0xECu, state);
    }

    if (state == 4u) {
        if (dest < 0x80000000u)
            return 0;
        start = PE_LoadU16(GA_D_800930D8 + (uint32_t)(ce2 + 8u) * 2u);
        end = PE_LoadU16(GA_D_800930D8 + (uint32_t)(ce2 + 9u) * 2u);
        lba = (int)(PE_LoadU32(GA_D_800B0DD8) + start);
        status = func_8006E6A8(lba, dest, (int)end - (int)start);
        if (status == -1)
            return 1;
        PE_StoreU8(GA_OVERLAY + 0xECu, 5u);
        return 1;
    }

    if (state == 5u) {
        status = func_8006E7E8();
        if (status == -1) {
            PE_StoreU8(GA_OVERLAY + 0xECu, 4u);
            return 1;
        }
        if (status != 0)
            return 1;
        PE_StoreU8(GA_OVERLAY + 0xECu, 6u);
        state = 6u;
    }

    if (state != 6u)
        return 0;

    package = dest;
    if (package < 0x80000000u)
        return 0;
    section = package + (PE_LoadU32(package + 4u) & MASK_22_BECC);
    if (section < 0x80000000u)
        return 0;
    packed = PE_LoadU32(section + 0x0Cu);
    {
        pe_addr_t rec0 = package + (packed & MASK_22_BECC);

        if (rec0 >= 0x80000000u)
            PE_StoreU32(0x800B0E70u,
                        package + (PE_LoadU32(rec0 + 4u) & MASK_24_BECC));
    }
    func_8006C140_type0_clip_bind(package);
    PE_StoreU8(GA_OVERLAY + 0xECu, 0u);
    PE_StoreU8(GA_OVERLAY + 0x0Bu, ce2);
    PE_StoreU32(GA_OVERLAY, PE_LoadU32(GA_OVERLAY) & ~0x200000u);
    return 0;
}

/* Original command headers can use physical RAM addresses, including zero.
 * Preserve the published pointer and map only its byte read to native RAM. */
static uint8_t pe_command_frame_count(pe_addr_t resource)
{
    if (resource < 0x200000u) resource |= 0x80000000u;
    return PE_LoadU8(resource + 2u);
}

/* day1 diagnostics: PE_ANIM_SET_LOG=path logs every anim (actor+0xE) set by
 * 1A680/1A784 with actor key, old/new anim and the host caller address
 * (resolve with addr2line -f -e <binary>). Read-only; off by default. */
void pe_anim_set_log(const char *who, pe_addr_t actor, unsigned int command, void *caller)
{
    static FILE *f; static int init;
    if (!init) { const char *p = getenv("PE_ANIM_SET_LOG"); init = 1; if (p && *p) f = fopen(p, "w"); }
    if (!f) return;
    fprintf(f, "vbl=%u %s actor=%08X key=%02X/%02X old=%02X new=%02X token=%08X caller=pe_anim_set_log%+ld\n",
            pe_plat_timing_vblank_count(), who, actor, PE_LoadU8(actor + 0xCu), PE_LoadU8(actor + 0xDu),
            PE_LoadU8(actor + 0xEu), command & 0xFFu, PE_LoadU32(0x8009D280u),
            (long)((char *)caller - (char *)(void *)pe_anim_set_log));
    fflush(f);
}

/* Audit item 4: identical to the matched func_8001A680 (same D_800B0E98 row
 * lookup, stores and 0x200000-child cascade); kept as a name for callers. */
void func_8001A680(pe_addr_t body, unsigned int id);   /* decomp_hand/absent_lo_actor_port.c */
void func_8001A680_command_cut(pe_addr_t actor, unsigned int command)
{
    func_8001A680(actor, command);
}

/* 1A784..1A890: select a clip without restarting it when unchanged,
 * and propagate the selection to attached actors that share animation. */
void func_8001A784(pe_addr_t actor, unsigned int command)
{
    pe_addr_t child;
    uint32_t flags;
    if (PE_LoadU8(actor + 0xEu) != (command & 0xFFFFu)) {
        pe_anim_set_log("1A784", actor, command, __builtin_return_address(0));
        unsigned int type = PE_LoadU8(actor + 0xCu);
        pe_addr_t resource;
        /* Unlike 1A680, this local reset must not restart descendants.
         * Each descendant makes its own unchanged-command check below. */
        PE_StoreU32(actor + 0x14u, 0u);
        PE_StoreU32(actor + 0x18u, 0u);
        PE_StoreU8(actor + 0xEu, (uint8_t)command);
        resource = PE_LoadU32(GA_COMMAND_TABLE + type * 192u
                             + (command & 0xFFFFu) * 4u);
        PE_StoreU32(actor + 0x1B0u, resource);
        PE_StoreU8(actor + 0xFu, (uint8_t)(pe_command_frame_count(resource) - 1u));
    }
    flags = PE_LoadU32(actor + 0x98u) & ~0x200u;
    PE_StoreU32(actor + 0x98u, flags);
    if (!(flags & 0x100000u)) return;
    for (child = PE_LoadU32(0x8009D20Cu); child;
         child = PE_LoadU32(child + 4u)) {
        if (PE_LoadU32(child + 0x18Cu) == actor &&
            (PE_LoadU32(child + 0x98u) & 0x200000u))
            func_8001A784(child, command & 0xFFFFu);
    }
}

/* Original 6A318..6A5BC: emit sound records crossed since the previous tick.
 * First four records are Aya-only; later records match actor type and ID too.
 * The current endpoint is excluded, the previous endpoint is included. */
int32_t func_8006A318(pe_addr_t actor)
{
    int32_t current, previous, speed;
    unsigned i;
    if (!actor) return -1;
    current = PE_LoadU16(actor + 0x16u);
    previous = PE_LoadU16(actor + 0x1Au);
    speed = (int32_t)PE_LoadU32(actor + 0x1Cu);
    if (speed > 0 && current < previous)
        current += PE_LoadU8(actor + 0xFu) + 1;
    else if (speed < 0 && previous < current)
        current -= PE_LoadU8(actor + 0xFu) + 1;
    {
        /* audio2 diagnostics: PE_AUDIO_ANIM_LOG=path logs, per actor tick
         * with registered room triggers, the actor key and frame window. */
        static FILE *al;
        static int al_init;
        if (!al_init) {
            const char *p = getenv("PE_AUDIO_ANIM_LOG");
            al_init = 1;
            if (p && *p) al = fopen(p, "w");
        }
        {
            static unsigned last_n = 0xFFFFu;
            unsigned n = PE_LoadU8(0x800B0CE9u);
            if (al && n != last_n) {
                last_n = n;
                for (unsigned k = 0; k < 4u + n; k++) {
                    pe_addr_t r = 0x80094488u + k * 8u;
                    fprintf(al, "REC %u: key=%02X/%02X anim=%02X frame=%u snd=%X/%X\n", k,
                            PE_LoadU8(r), PE_LoadU8(r + 1u), PE_LoadU8(r + 2u),
                            PE_LoadU8(r + 3u), PE_LoadU16(r + 4u), PE_LoadU16(r + 6u));
                }
            }
        }
        if (al && PE_LoadU8(0x800B0CE9u)) {
            fprintf(al, "vbl=%u actor=%08X key=%02X/%02X anim=%02X win=%d..%d dir=%d aya=%d d1a0=%X cd8=%X n=%u\n",
                    pe_plat_timing_vblank_count(), actor, PE_LoadU8(actor + 0xCu), PE_LoadU8(actor + 0xDu),
                    PE_LoadU8(actor + 0xEu), previous, current, speed,
                    actor == PE_LoadU32(0x8009D254u), PE_LoadU32(0x8009D1A0u),
                    PE_LoadU32(0x800B0CD8u), PE_LoadU8(0x800B0CE9u));
        }
    }
    for (i = 0; i < 4u + PE_LoadU8(0x800B0CE9u); ++i) {
        pe_addr_t record = 0x80094488u + i * 8u;
        int32_t frame;
        uint32_t sound;
        if (i < 4u) {
            if (actor != PE_LoadU32(0x8009D254u) ||
                (PE_LoadU32(0x8009D1A0u) & 2u) ||
                (PE_LoadU32(0x800B0CD8u) & 0x800000u)) continue;
        } else if (PE_LoadU8(record) != PE_LoadU8(actor + 0xCu) ||
                   PE_LoadU8(record + 1u) != PE_LoadU8(actor + 0xDu)) continue;
        if (PE_LoadU8(record + 2u) != PE_LoadU8(actor + 0xEu)) continue;
        frame = PE_LoadU8(record + 3u);
        if (!((speed > 0 && previous <= frame && frame < current) ||
              (speed < 0 && current < frame && frame <= previous))) continue;
        sound = PE_LoadU16(record + 4u + PE_LoadU8(0x800B0CEAu) * 2u);
        {
            /* day1 diagnostics: PE_ANIM_SFX_LOG=path logs each anim-keyed sound fire. */
            static FILE *sf; static int sf_init;
            if (!sf_init) { const char *p = getenv("PE_ANIM_SFX_LOG"); sf_init = 1; if (p && *p) sf = fopen(p, "w"); }
            if (sf) {
                fprintf(sf, "vbl=%u FIRE rec=%u actor=%08X key=%02X/%02X anim=%02X frame=%d snd=%X token=%08X\n",
                        pe_plat_timing_vblank_count(), i, actor, PE_LoadU8(actor + 0xCu), PE_LoadU8(actor + 0xDu),
                        PE_LoadU8(actor + 0xEu), frame, sound, PE_LoadU32(0x8009D280u));
                fflush(sf);
            }
        }
        if (sound)
            func_8006DCE4(sound, 0u,
                (int16_t)PE_LoadU16(actor + 0x2Au),
                (int16_t)PE_LoadU16(actor + 0x2Eu),
                (int16_t)PE_LoadU16(actor + 0x32u));
    }
    return 0;
}

/*
 * PE-BTL66 — func_8001A4AC clip ticker.
 *
 * 117 words 0x8001A4AC..0x8001A680, SHA-256 53d93b57…2174.
 * Callers: self @ 1A4E4, 35558 @ 35B84 (D254) and 35BEC (D20C).
 * 6A318 processes sound events unconditionally before frame stores.
 * Linked actors select the parent command via 1A784 and copy its frame.
 * Arithmetic is checked against original execution by animation_tick_oracle.
 *
 * Clears +0x98 bit 3, sets bit 0x800000, copies +0x14 → +0x18.
 * +0x1A is the high half of that copy, so it lags +0x16 by one
 * tick. 2B0E8 phase 2 and 24A3C case 9 wait +0x0F==+0x1A.
 * Bit 0x100 returns. Bit 0x200 and +0x16==+0x12 returns.
 * Else +0x14 += +0x1C (35038 stores 0x10000) and clamps to
 * +0x12<<16. +0x16 is the high half of +0x14: 2B0E8 phase 0
 * waits that half == 10. +0x0F wrap / bit 8 is the no-0x200
 * path.
 */
void func_8001A4AC(pe_addr_t actor)
{
    pe_addr_t child;
    uint32_t flags;
    uint32_t cur;
    uint32_t speed;
    uint32_t next;
    uint32_t target;
    uint32_t dest;
    uint32_t cap;
    int32_t scur;
    int32_t snext;
    int32_t starget;

    if (actor == 0u)
        return;
    child = PE_LoadU32(actor + 0x18Cu);
    if (child != 0u) {
        if ((PE_LoadU32(child + 0x98u) & 0x800000u) == 0u)
            func_8001A4AC(child);

    }
    func_8006A318(actor);
    flags = (PE_LoadU32(actor + 0x98u) & ~8u) | 0x800000u;
    PE_StoreU32(actor + 0x98u, flags);
    cur = PE_LoadU32(actor + 0x14u);
    PE_StoreU32(actor + 0x18u, cur);
    if (flags & 0x100u)
        return;
    dest = PE_LoadU16(actor + 0x12u);
    if ((flags & 0x200u) != 0u && (cur >> 16) == dest)
        return;
    if (flags & 0x200000u) {
        child = PE_LoadU32(actor + 0x18Cu);
        func_8001A784(actor, PE_LoadU8(child + 0xEu));
        child = PE_LoadU32(actor + 0x18Cu);
        PE_StoreU32(actor + 0x14u, PE_LoadU32(child + 0x14u));
        return;
    }
    speed = PE_LoadU32(actor + 0x1Cu);
    next = cur + speed;
    target = dest << 16;
    /* Original checks strict target crossing before wrapping the clip. */
    scur = (int32_t)cur;
    snext = (int32_t)next;
    starget = (int32_t)target;
    if ((flags & 0x200u) &&
        ((scur < starget && starget < snext) ||
         (starget < scur && snext < starget))) {
        PE_StoreU32(actor + 0x14u, target);
        return;
    }
    cap = PE_LoadU8(actor + 0x0Fu);
    if ((int32_t)next >> 16 > (int32_t)cap) {
        uint32_t denom;

        denom = cap + 1u;
        if (denom == 0u)
            return;
        next = (uint32_t)(((int32_t)next >> 16) % (int32_t)denom) << 16;
        cur = 0u;
        flags |= 8u;
        PE_StoreU32(actor + 0x98u, flags);
    } else if ((int32_t)next < 0) {
        cur = cap << 16;
        next += (cap + 1u) << 16;
        flags |= 8u;
        PE_StoreU32(actor + 0x98u, flags);
    }
    flags = PE_LoadU32(actor + 0x98u);
    if ((flags & 0x200u) == 0u) {
        PE_StoreU32(actor + 0x14u, next);
        return;
    }
    scur = (int32_t)cur;
    snext = (int32_t)next;
    starget = (int32_t)target;
    if ((scur <= starget && starget < snext)
        || (starget <= scur && snext < starget))
        PE_StoreU32(actor + 0x14u, target);
    else
        PE_StoreU32(actor + 0x14u, next);
}

/* R3000 Kuseg/KSEG0 RAM mirror used by these original actor routines. */
static pe_addr_t pe_kseg0(pe_addr_t addr)
{
    return 0x80000000u | (addr & 0x1FFFFFu);
}

/* Weapon-category tables of the retail EXE: 0x800106A4 (9 rows x 5 bytes)
 * and 0x800106D4 (9 scale bytes).  They are read from the disc-loaded EXE in
 * guest RAM (PE_GuestImage_LoadExe), never embedded: retail bytes are not
 * committed (owner decision 2026-09-28). */
#define K_209F0_ROW   0x800106A4u
#define K_209F0_SCALE 0x800106D4u

/* Complete 209F0 with the two immutable executable tables above. The
 * historical cut name below remains an alias for existing translated callers. */
void func_800209F0(void)
{
    pe_addr_t record;
    pe_addr_t obj;
    unsigned int idx;
    unsigned int scale;
    int prod;
    int hi;
    pe_addr_t row;

    record = PE_LoadU32(GA_RECORD_P);
    obj = pe_kseg0(PE_LoadU32(record + 0x68u));
    idx = PE_LoadU8(obj + 6u);
    if (idx >= 9u) abort(); /* Original tables have nine weapon categories. */
    scale = PE_LoadU8(K_209F0_SCALE + idx);
    row = K_209F0_ROW + idx * 5u;
    prod = (int)PE_LoadU16(record + 0x22u) * (int)scale;
    hi = (int)(((long long)prod * 0x66666667LL) >> 32);
    PE_StoreU8(record + 0x12u, 4u);
    PE_StoreU16(record + 0x24u, (uint16_t)(hi >> 2));
    PE_StoreU8(record + 0x13u, 5u);
    PE_StoreU8(record + 0x14u, 6u);
    PE_StoreU8(record + 0x17u, 7u);
    PE_StoreU8(record + 0x15u, 8u);
    PE_StoreU8(record + 0x18u, 9u);
    PE_StoreU8(record + 0x16u, 10u);
    PE_StoreU8(record + 0x19u, 11u);
    PE_StoreU8(obj + 0x14u, PE_LoadU8(row + 0u));
    PE_StoreU8(obj + 0x15u, PE_LoadU8(row + 1u));
    PE_StoreU8(obj + 0x16u, PE_LoadU8(row + 2u));
    PE_StoreU8(obj + 0x17u, PE_LoadU8(row + 3u));
    PE_StoreU32(obj + 8u, PE_LoadU8(row + 4u));
    (void)func_8006C4C4((int)(int16_t)PE_LoadU16(obj + 6u));
}

void func_800209F0_cut(void) {func_800209F0();}

/* Full retail 30640 (matching src/func_80030640.c): First Strike's
 * chance to start with full AT is based on speed and the BIOS RNG. */
void func_80030640_cut(void)
{
    pe_addr_t record;
    pe_addr_t obj;

    record = PE_LoadU32(GA_RECORD_P);
    obj = pe_kseg0(PE_LoadU32(record + 0x68u));
    if ((PE_LoadU32(obj + 0x10u) & 0x00010000u) == 0u)
        return;
    if (func_80071A54() % 100u < PE_LoadU16(record + 0x22u))
        PE_StoreU16(record + 0x10u,9000u);
}

/*
 * 339A0 32w. Copy 16B 0x80010E38 onto $sp, index (a0&0xFF)*4,
 * sh pair to D_8009CE84 / D_8009CE86, sb a0 to D_8009CE80.
 * 0 jals. Not ATB/AI/damage.
 */
void func_800339A0_cut(unsigned int encounter)
{
    static const uint16_t k_pairs[4][2] = {
        { 0x00DDu, 0x00B1u },
        { 0x000Fu, 0x00B1u },
        { 0x000Fu, 0x000Fu },
        { 0x00DDu, 0x000Fu },
    };
    unsigned int idx;

    idx = encounter & 0xFFu;
    PE_StoreU8(0x8009CE80u, (uint8_t)idx);
    if (idx < 4u) {
        PE_StoreU16(0x8009CE84u, k_pairs[idx][0]);
        PE_StoreU16(0x8009CE86u, k_pairs[idx][1]);
    }
}

void func_80029810_after_hp_cut(unsigned int encounter)
{
    pe_addr_t record;
    pe_addr_t actor;
    pe_addr_t source;
    int value;
    int cap;

    func_800209F0_cut();
    record = PE_LoadU32(GA_RECORD_P);
    value = (int)PE_LoadU32(record + 0x08u);
    if (value <= 0) {
        PE_StoreU32(record + 0x08u, 0x00010000u);
    } else {
        cap = (int)PE_LoadU32(record + 0x28u);
        if (value >= cap) {
            PE_StoreU32(record + 0x08u, (unsigned int)cap);
            PE_StoreU32(record + 0x34u, 240u);
        }
    }

    func_80030640_cut();
    actor = PE_LoadU32(GA_ACTOR_P);
    PE_StoreU32(actor + 0x194u, CALLBACK_1D268);
    source = PE_LoadU32(actor + 0x238u);
    PE_StoreU16(GA_VALUE_D27C,
                (uint16_t)(PE_LoadU32(source + 0x18u) - 100u));
    func_800339A0_cut(encounter);
    record = PE_LoadU32(GA_RECORD_P);
    func_8001A680_command_cut(actor, PE_LoadU8(record + 0x12u));
}

/*
 * 144FC 0x3B at 0x80014630 (12 words to park). $s0 = overlay
 * D_800B0CD8. Busy (+0xE&3) j 0x80014660 v0=0. Clear: lw/sw
 * 0($s0) &= ~0x00800000, sb F4=0, j 0x8001467C v0=1.
 * 0x3A's $s1 binder is a different object (8E22 vs 8E02).
 * No jal/jalr; not 6914C; not 0x800E086C. v0=1 is not issued
 * on the live park (bits stay set until 3F074's 6C5BC poll
 * returns 0 via EE=0 → 6CC68 after 6CC2C).
 */
int func_800144FC_state3B_cut(void)
{
    if ((PE_LoadU8(GA_OVERLAY + 0x0Eu) & 3u) != 0u)
        return 0;
    PE_StoreU8(GA_OVERLAY + 0xF4u, 0u);
    PE_StoreU32(GA_OVERLAY, PE_LoadU32(GA_OVERLAY) & 0xFF7FFFFFu);
    return 1;
}

/*
 * 144FC state 0 at 0x80014544. If (+0xE & 3)==0, sb 0x37 → +0xF4;
 * always overlay[0] |= 0x00800000 (lw/sw 0($s0)). j 0x80014660
 * v0=0. Does not complete 0x55.
 */
int func_800144FC_state0_cut(void)
{
    if ((PE_LoadU8(GA_OVERLAY + 0x0Eu) & 3u) == 0u)
        PE_StoreU8(GA_OVERLAY + 0xF4u, 0x37u);
    PE_StoreU32(GA_OVERLAY, PE_LoadU32(GA_OVERLAY) | 0x00800000u);
    return 0;
}

/* func_80042EDC: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80042EDC_port.c (src/func_80042EDC.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_80042F20: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80042F20_port.c (src/func_80042F20.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/*
 * 144FC state 0x37 at 0x80014570. If overlay word bit 0x400000 is
 * clear, jal 42EDC; always sb 0x38 → +0xF4 and re-dispatch
 * (j 0x80014518). Named cut does not enter 0x38 / 6D60C.
 */
int func_800144FC_state37_cut(void)
{
    if ((PE_LoadU32(GA_OVERLAY) & 0x00400000u) == 0u)
        func_80042EDC();
    PE_StoreU8(GA_OVERLAY + 0xF4u, 0x38u);
    return 0;
}

extern void func_80087024(void);

#define GA_GP_418 0x8009D188u
#define GA_GP_41C 0x8009D18Cu
#define GA_GP_420 0x8009D190u
#define GA_CDA4   0x8009CDA4u

/*
 * 6D60C after 6D078 returns 0, at 0x8006D79C. Overlay word bit 0x4
 * (set by 0x2C) and bit 0x40 select the arm. Live NYPD: bit4 set,
 * bit40 clear → sb F2=0x3F, re-dispatch. 0x3F with the same bits
 * sb 0x2F. 0x2F jals 6CDA4(0, lb +0xE1, …); a0==0 parks (87198
 * not stubbed). 86C5C / 6DF50 / 864CC are not stubbed.
 */
void func_8006D60C_after_6d078_cut(void)
{
    unsigned int word;
    int remain;

    word = PE_LoadU32(GA_OVERLAY);
    if ((word & 4u) != 0u && (word & 0x40u) != 0u) {
        remain = 60 - ((int)PE_LoadU32(GA_CDA4) - (int)PE_LoadU32(GA_GP_41C));
        PE_StoreU32(GA_GP_420, (unsigned int)remain);
        if (remain >= 9)
            PE_StoreU32(GA_GP_420, 8u);
        else if (remain < 0)
            PE_StoreU32(GA_GP_420, 0u);
    }
    PE_StoreU8(GA_OVERLAY + 0xF2u, 0x3Fu);
}

int func_8006D60C_state3F_cut(void)
{
    unsigned int word;
    int timer;

    word = PE_LoadU32(GA_OVERLAY);
    if ((word & 4u) == 0u) {
        PE_StoreU8(GA_OVERLAY + 0xF2u, 0x30u);
        return 1;
    }
    if ((word & 0x40u) == 0u) {
        PE_StoreU8(GA_OVERLAY + 0xF2u, 0x2Fu);
        return 1;
    }
    timer = (int)PE_LoadU32(GA_GP_420);
    if (timer > 0) {
        PE_StoreU32(GA_GP_420, (unsigned int)(timer - 1));
        return 1;
    }
    PE_StoreU8(GA_OVERLAY + 0xF2u, 0x2Fu);
    return 1;
}

int func_8006D60C_state2F_cut(void)
{
    if (func_8006CDA4(0, (int)(int8_t)PE_LoadU8(GA_OVERLAY + 0xE1u), 0,
                      PE_LoadU32(GA_OVERLAY + 0x194u), 0x21, 0) == 1)
        return 1;
    PE_StoreU8(GA_OVERLAY + 0xF2u, 0x30u);
    return 0;
}

int func_8006D60C_state30_cut(void)
{
    func_80086464(PE_LoadU32(GA_OVERLAY + 0x124u));
    func_80086C1C(0, 0x7F);
    PE_StoreU8(GA_OVERLAY + 0xF2u, 0u);
    return 0;
}

void func_8006D60C_state0_a0eq1_cut(void)
{
    PE_StoreU32(GA_GP_418, 0u);
    func_80087024();
    PE_StoreU8(GA_OVERLAY + 0xF2u, 0x2Cu);
}

void func_8006D60C_state2C_cut(void)
{
    if ((int8_t)PE_LoadU8(GA_OVERLAY + 0xE0u)
        != (int8_t)PE_LoadU8(GA_OVERLAY + 0xDCu)) {
        PE_StoreU32(GA_OVERLAY, PE_LoadU32(GA_OVERLAY) | 4u);
    }
    PE_StoreU8(GA_OVERLAY + 0xF2u, 0x2Eu);
}

/*
 * PE-BTL117 — 6D60C(0) F2==0 is not a wait. jtbl[0]=6D658
 * always jal 87024 / sw 0 at gp+0x418, then a0==0 takes
 * 6D6EC → F2=45. 6A674 stores +0xE8=-1 and +0xEA/+0xEB=0,
 * so 45 increments the index and 50 skips 6CDA4. jtbl[64]
 * at 6D9E8 with overlay bit 4 clear sb F2=0 and returns 0.
 * Do not plant F2=0x41. ATK20 restores the state64 timer and
 * music-stop completion; ambient reload 62/51 remains explicit.
 * 86C5C on the 6D6EC bit-4 arm is not this
 * cut; F2 still becomes 45.
 */
static void func_8006D60C_state0_common(void)
{
    PE_StoreU32(GA_GP_418, 0u);
    func_80087024();
}

static int func_8006D60C_state2D_cut(void)
{
    int idx;

    idx = (int)PE_LoadU32(GA_GP_418);
    if (idx < 2) {
        if (PE_LoadU8(GA_OVERLAY + 0xEAu + (unsigned int)idx) != 0u) {
            PE_StoreU8(GA_OVERLAY + 0xF2u, 0x31u);
            return 0;
        }
        PE_StoreU32(GA_GP_418, (unsigned int)(idx + 1));
    }
    PE_StoreU8(GA_OVERLAY + 0xF2u, 0x32u);
    return 0;
}

static int func_8006D60C_state31_cut(void)
{
    unsigned int idx;

    idx = PE_LoadU32(GA_GP_418);
    if (func_8006CDA4(2, (int)PE_LoadU8(GA_OVERLAY + 0xEAu + idx), 0,
                      PE_LoadU32(GA_OVERLAY + 0x194u), 0x21, 0) == 1)
        return 1;
    PE_StoreU8(GA_OVERLAY + 0xF2u, 0x2Du);
    PE_StoreU32(GA_GP_418, idx + 1u);
    return 0;
}

static int func_8006D60C_state32_cut(void)
{
    int16_t xa;

    xa = (int16_t)PE_LoadU16(GA_OVERLAY + 0xE8u);
    if (xa != -1) {
        if (func_8006CDA4(1, (int)xa, 0,
                          PE_LoadU32(GA_OVERLAY + 0x194u), 0x21, 0) == 1)
            return 1;
    }
    if (PE_LoadU32(GA_OVERLAY) & 4u) {
        int32_t remain=(int32_t)(60u-(PE_LoadU32(GA_CDA4)-PE_LoadU32(GA_GP_41C)));
        PE_StoreU32(GA_GP_420,(uint32_t)remain);
        if (remain>=9) {
            PE_StoreU32(GA_GP_420,8u);
            func_80086C5C(0,16u,0u);
        } else if (remain<0) PE_StoreU32(GA_GP_420,0u);
    }
    PE_StoreU8(GA_OVERLAY + 0xF2u, 0x40u);
    return 0;
}

static int func_8006D60C_state40_cut(void)
{
    unsigned int word;

    word = PE_LoadU32(GA_OVERLAY);
    if (word & 4u) {
        int32_t timer=(int32_t)PE_LoadU32(GA_GP_420);
        if (timer>0) {
            PE_StoreU32(GA_GP_420,(uint32_t)(timer-1));
            return 1;
        }
        func_80086FF8();
        word=PE_LoadU32(GA_OVERLAY);
        if (word & 0x40u) {
            PE_StoreU32(GA_OVERLAY,word&~0x40u);
            PE_StoreU8(GA_OVERLAY+0xF2u,0x3Eu);
            return -1; /* Retail redispatches immediately to state 3E. */
        }
    }
    PE_StoreU8(GA_OVERLAY + 0xF2u, 0u);
    PE_StoreU32(GA_OVERLAY, word & ~4u);
    return 0;
}

/* Retail 6DA98..6DB24. The bank loader's stack output is unused here.
 * Preserve the borrowed guest word; no persistent output is manufactured. */
static int func_8006D60C_state33(void)
{
    const pe_addr_t slot=0x801FEFE8u;
    uint32_t saved=PE_LoadU32(slot);
    int result=func_8006D2B8((int8_t)PE_LoadU8(GA_OVERLAY+0xDCu),1,0,slot,0);
    PE_StoreU32(slot,saved);
    if (result==1) return 1;
    int handle=func_80086464(PE_LoadU32(GA_OVERLAY+0x128u));
    if (handle==-1) {
        PE_StoreU8(GA_OVERLAY+0xF2u,0x3Eu);
        return 1;
    }
    func_80086C5C(handle,60u,PE_LoadU8(GA_OVERLAY+0xFEu));
    if (handle) func_8006DB48(0u,(uint32_t)(int32_t)(int8_t)PE_LoadU8(GA_OVERLAY+0xDCu),
        (uint32_t)handle,PE_LoadU8(GA_OVERLAY+0xFEu));
    PE_StoreU8(GA_OVERLAY+0xF2u,0u);
    PE_StoreU32(GA_OVERLAY,(PE_LoadU32(GA_OVERLAY)&~4u)|0x40u);
    return 0;
}

void func_800864CC(void);
int func_8006D60C(int arg)
{
    /* Audit item 17: line-for-line src/func_8006D60C.c (sound bank / music
     * change state machine): restores the 0x451/0x452 SFX pair, the
     * D_8009D18C / D_8009D190 timer latches, func_80086C5C / func_800864CC /
     * func_80086FF8 / func_80087024 calls, and the D_8009D188 argument of
     * the 0x31 state.  Loops within the call like the retail goto. */
    const pe_addr_t s = GA_OVERLAY;
#define S_FLAGS() PE_LoadU32(s)
#define S_STATE(v) PE_StoreU8(s + 0xF2u, (uint8_t)(v))
#define D188() (int32_t)PE_LoadU32(0x8009D188u)
    int32_t d;
    for (;;) {
        switch (PE_LoadU8(s + 0xF2u)) {
        case 0:
            PE_StoreU32(0x8009D188u, 0u);
            func_80087024();
            if (arg != 0) {
                if (!(PE_LoadU32(0x800B0CD8u) & 0x400000u) && PE_LoadU32(0x800B0E08u) != 0u) {
                    (void)func_8006DF50((int)PE_LoadU32(0x800B0E08u), 0x451, 0, 0x80, 0x7F);
                    if (PE_LoadU32(0x800B0E08u) != 0u)
                        (void)func_8006DF50((int)PE_LoadU32(0x800B0E08u), 0x452, 0, 0x80, 0x7F);
                }
                S_STATE(0x2Cu);
                continue;
            }
            if (S_FLAGS() & 4u) {
                PE_StoreU32(0x8009D18Cu, PE_LoadU32(0x8009CDA4u));
                func_80086C5C(0, 0x3C, 0);
            }
            S_STATE(0x2Du);
            continue;
        case 0x2C:
            if ((int8_t)PE_LoadU8(s + 0xE0u) != (int8_t)PE_LoadU8(s + 0xDCu)) {
                if (S_FLAGS() & 0x40u) {
                    PE_StoreU32(0x8009D18Cu, PE_LoadU32(0x8009CDA4u));
                    func_80086C5C(0, 0x3C, 0);
                }
                PE_StoreU32(s, S_FLAGS() | 4u);
            }
            S_STATE(0x2Eu);
            continue;
        case 0x2E:
            if (func_8006D078() == 1) return 1;
            if ((S_FLAGS() & 4u) && (S_FLAGS() & 0x40u)) {
                d = 0x3C - ((int32_t)PE_LoadU32(0x8009CDA4u) - (int32_t)PE_LoadU32(0x8009D18Cu));
                PE_StoreU32(0x8009D190u, (uint32_t)d);
                if (d >= 9) { PE_StoreU32(0x8009D190u, 8u); func_80086C5C(0, 0x10, 0); }
                else if (d < 0) PE_StoreU32(0x8009D190u, 0u);
            }
            S_STATE(0x3Fu);
            continue;
        case 0x3F:
            if (!(S_FLAGS() & 4u)) { S_STATE(0x30u); continue; }
            if (S_FLAGS() & 0x40u) {
                if ((int32_t)PE_LoadU32(0x8009D190u) > 0) {
                    PE_StoreU32(0x8009D190u, PE_LoadU32(0x8009D190u) - 1u);
                    return 1;
                }
                func_800864CC();
                func_80086FF8();
            }
            S_STATE(0x2Fu);
            continue;
        case 0x2F:
            if (func_8006CDA4(0, (int8_t)PE_LoadU8(s + 0xE1u), 0, PE_LoadU32(s + 0x194u), 0x21, 0) == 1)
                return 1;
            S_STATE(0x30u);
            continue;
        case 0x30:
            (void)func_80086464(PE_LoadU32(s + 0x124u));
            func_80086C1C(0, 0x7F);
            S_STATE(0u);
            return 0;
        case 0x2D:
            if (D188() < 2) {
                if (PE_LoadU8(s + 0xEAu + (uint32_t)D188()) != 0u) { S_STATE(0x31u); continue; }
                PE_StoreU32(0x8009D188u, (uint32_t)(D188() + 1));
            }
            S_STATE(0x32u);
            continue;
        case 0x31:
            if (func_8006CDA4(2, PE_LoadU8(s + 0xEAu + (uint32_t)D188()), D188(), PE_LoadU32(s + 0x194u),
                              0x21, 0) == 1)
                return 1;
            S_STATE(0x2Du);
            PE_StoreU32(0x8009D188u, (uint32_t)(D188() + 1));
            continue;
        case 0x32:
            if ((int16_t)PE_LoadU16(s + 0xE8u) != -1) {
                if (func_8006CDA4(1, (int16_t)PE_LoadU16(s + 0xE8u), 0, PE_LoadU32(s + 0x194u), 0x21, 0) == 1)
                    return 1;
            }
            if (S_FLAGS() & 4u) {
                d = 0x3C - ((int32_t)PE_LoadU32(0x8009CDA4u) - (int32_t)PE_LoadU32(0x8009D18Cu));
                PE_StoreU32(0x8009D190u, (uint32_t)d);
                if (d >= 9) { PE_StoreU32(0x8009D190u, 8u); func_80086C5C(0, 0x10, 0); }
                else if (d < 0) PE_StoreU32(0x8009D190u, 0u);
            }
            S_STATE(0x40u);
            continue;
        case 0x40:
            if (S_FLAGS() & 4u) {
                if ((int32_t)PE_LoadU32(0x8009D190u) > 0) {
                    PE_StoreU32(0x8009D190u, PE_LoadU32(0x8009D190u) - 1u);
                    return 1;
                }
                func_80086FF8();
                if (S_FLAGS() & 0x40u) {
                    PE_StoreU32(s, S_FLAGS() & ~0x40u);
                    S_STATE(0x3Eu);
                    continue;
                }
            }
            S_STATE(0u);
            PE_StoreU32(s, S_FLAGS() & ~4u);
            return 0;
        case 0x3E:
            if (func_8006CDA4(0, (int8_t)PE_LoadU8(s + 0xDAu), 0, PE_LoadU32(s + 0x194u), 0x21, 0) == 1)
                return 1;
            S_STATE(0x33u);
            continue;
        case 0x33:
            return func_8006D60C_state33();
        default:
            return 0;
        }
    }
#undef S_FLAGS
#undef S_STATE
#undef D188
}

int func_800144FC_state38_cut(void)
{
    if (func_8006D60C(1) == 1)
        return 0;
    if ((PE_LoadU32(GA_OVERLAY) & 0x00400000u) == 0u)
        func_80042F20();
    PE_StoreU8(GA_OVERLAY + 0xF4u, 0x39u);
    return 0;
}

extern int func_8006E6A8(int lba, pe_addr_t dest, int sectors);
extern int func_8006E7E8(void);
extern pe_addr_t func_8006E498(pe_addr_t base, uint32_t key);


#define GA_930E2  0x800930E2u
#define GA_930E4  0x800930E4u
#define GA_942E4  0x800942E4u
#define GA_942E8  0x800942E8u

/*
 * 6914C state 0 table prefix at 0x800691D0. Two 11-slot inits:
 * dest=+0x188 stride 0xA0C, then dest=+0x188+0x6E84 stride 0x10C.
 * Each slot: sb 0, sb 0xFF ×3, sw 0 at +4, sw 0 at +8. 6A8D4
 * publishes +0x188=D_800B0E60. jalr D_800942E0 → 0x800E0xxx
 * overlay leaves are not invented (overlay not loaded).
 */
static void func_8006914C_state0_tables(void)
{
    pe_addr_t base;
    pe_addr_t slot;
    unsigned int i;

    base = PE_LoadU32(GA_OVERLAY + 0x188u);
    PE_StoreU32(GA_942E4, base);
    if (base == 0u)
        return;
    for (i = 0; i < 11u; i++) {
        slot = base + i * 0xA0Cu;
        PE_StoreU8(slot, 0u);
        PE_StoreU8(slot + 1u, 0xFFu);
        PE_StoreU8(slot + 2u, 0xFFu);
        PE_StoreU8(slot + 3u, 0xFFu);
        PE_StoreU32(slot + 4u, 0u);
        PE_StoreU32(slot + 8u, 0u);
    }
    base = PE_LoadU32(GA_OVERLAY + 0x188u) + 0x6E84u;
    PE_StoreU32(GA_942E8, base);
    for (i = 0; i < 11u; i++) {
        slot = base + i * 0x10Cu;
        PE_StoreU8(slot, 0u);
        PE_StoreU8(slot + 1u, 0xFFu);
        PE_StoreU8(slot + 2u, 0xFFu);
        PE_StoreU8(slot + 3u, 0xFFu);
        PE_StoreU32(slot + 4u, 0u);
        PE_StoreU32(slot + 8u, 0u);
    }
}

/*
 * func_8006914C is 274 words (0x8006914C..0x80069594). Dispatch on
 * lbu +0xEF. State 0: if D1A0 bit 0x80 clear, fill the two 11-slot
 * tables then jalr D_800942E0 (overlay 0x800E0xxx — not invented).
 * Proven tail: overlay |= 8, D1A0 |= 0x80. a0!=0 && bit 8 → sb 0x34,
 * return 1. 0x34 jals real 6E6A8(LBA=+0x100+lhu 930E2,
 * dest=+0x194=6A8D4 D_800B0E6C=0x801ED800, sectors=5). -1 stays
 * 0x34 return 1; else sb 0x35 return 1. 0x35 jals real 6E7E8;
 * host-sync → sb 0x36 return 1. 0x36 jals 6E1C0 × count (LoadImage
 * over the 10240B TIM-like dest; does not write it) then 6E498
 * (+0x18C, key 0x73DECD80). Done: sb EF=0, overlay&=~8, v0=0.
 * Next 6914C(1) at EF=0 with bit8 clear returns 0 (not a stub).
 * a0==0 at 0x800693B4: D1A0 bit 1 clear → sb EF=0, v0=0.
 * a0==0 and bit 1 set uses the same overlay&8 arm as a0!=0.
 * 2CEE0 needs that v0=0 (plus s1!=0) to reach 2CF24. Does not sb 0x3A.
 */
/* 691D0..692F8 (src/func_8006914C.c): after the slot tables, run each
 * effect module's init (descriptor word 0) for modules 0..7, module 0x55
 * and every room-exported module 8..0x54: `h = D_800942E0[i];
 * if (h && *h) (*h)();`.  Module 0x55 is 0x800E13D4 -> func_800D4860,
 * which fills the effect texpage table D_800E2850 (func_800CECAC), the
 * D_800F32D4.. resource pointers and uploads the effect texture; without
 * it every effect sprite samples texpage 0 (the framebuffer).
 * An init with no native implementation is logged once and skipped. */
static void func_8006914C_module_init(pe_addr_t table, unsigned int code)
{
    pe_addr_t descriptor, fn;
    if (!table || !PE_RangeIsRam(table + code * 4u, 4u)) return;
    descriptor = PE_LoadU32(table + code * 4u);
    if (!descriptor || !PE_RangeIsRam(descriptor, 4u)) return;
    fn = PE_LoadU32(descriptor);
    if (!fn) return;
    if (PE_GuestCode_Resolve(fn)) {
        (void)PE_GuestCall("@8006914C:module-init", fn, 0u, 0, 0, 0, 0);
        return;
    }
    /* Resident code `jr ra; move v0,zero` (e.g. m0018i module 0x45 init
     * 8018EFF4, whose room is not split) is a complete `return 0`. */
    if (PE_RangeIsRam(fn, 8u) && PE_LoadU32(fn) == 0x03E00008u &&
        PE_LoadU32(fn + 4u) == 0x00001021u)
        return;
    {
        static pe_addr_t seen[16]; static unsigned n;
        unsigned k;
        for (k = 0; k < n && seen[k] != fn; k++) {}
        if (k == n && n < 16u) {
            seen[n++] = fn;
            fprintf(stderr, "[DAY1_SHIM] func_8006914C: module %02X init %08X not ported; skipped\n",
                    code, (unsigned)fn);
        }
    }
}

static void func_8006914C_module_inits(void)
{
    pe_addr_t table = PE_LoadU32(0x800942E0u), b0, h0, e0;
    uint32_t span;
    unsigned int i;
    for (i = 0; i < 8u; i++) func_8006914C_module_init(table, i);
    func_8006914C_module_init(table, 0x55u);
    b0 = PE_LoadU32(GA_OVERLAY + 0x18Cu);
    if (!b0 || !PE_RangeIsRam(b0, 8u)) return;
    h0 = b0 + PE_LoadU32(b0 + 4u);
    if (!PE_RangeIsRam(h0, 8u)) return;
    span = PE_LoadU32(h0 + 4u);
    e0 = b0 + (span & 0x3FFFFFu);
    for (i = 0; i < (span >> 22); i++) {
        unsigned int code = PE_LoadU8(e0 + i * 12u + 7u);
        if (code - 8u < 0x4Du) func_8006914C_module_init(table, code);
    }
}

int func_8006914C(int a0)
{
    unsigned int ef;
    unsigned int word;
    int status;
    unsigned int off;
    unsigned int end;

    ef = PE_LoadU8(GA_OVERLAY + 0xEFu);
    if (ef == 0u) {
        if ((D_8009D1A0 & 0x80u) == 0u) {
            func_8006914C_state0_tables();
            func_8006914C_module_inits();
            word = PE_LoadU32(GA_OVERLAY);
            PE_StoreU32(GA_OVERLAY, word | 8u);
            D_8009D1A0 |= 0x80u;
        }
        if (a0 == 0 && (D_8009D1A0 & 2u) == 0u) {
            PE_StoreU8(GA_OVERLAY + 0xEFu, 0u);
            return 0;
        }
        if ((PE_LoadU32(GA_OVERLAY) & 8u) != 0u) {
            PE_StoreU8(GA_OVERLAY + 0xEFu, 0x34u);
            return 1;
        }
        PE_StoreU8(GA_OVERLAY + 0xEFu, 0u);
        return 0;
    }
    if (ef == 0x34u) {
        off = PE_LoadU16(GA_930E2);
        end = PE_LoadU16(GA_930E4);
        status = func_8006E6A8(
            (int)(PE_LoadU32(GA_OVERLAY + 0x100u) + off),
            PE_LoadU32(GA_OVERLAY + 0x194u),
            (int)(end - off));
        if (status != -1)
            PE_StoreU8(GA_OVERLAY + 0xEFu, 0x35u);
        return 1;
    }
    if (ef == 0x35u) {
        status = func_8006E7E8();
        if (status == -1) {
            PE_StoreU8(GA_OVERLAY + 0xEFu, 0x34u);
            return 1;
        }
        if (status != 0)
            return 1;
        PE_StoreU8(GA_OVERLAY + 0xEFu, 0x36u);
        return 1;
    }
    if (ef == 0x36u) {
        pe_addr_t dest;
        pe_addr_t block;
        pe_addr_t entry;
        pe_addr_t arch;
        pe_addr_t found;
        unsigned int word;
        int count;
        int i;
        uint32_t key;
        RECT rc;
        uint32_t dims;
        uint32_t h;
        uint32_t off;

        dest = PE_LoadU32(GA_OVERLAY + 0x194u);
        block = dest + PE_LoadU32(dest + 4u);
        word = PE_LoadU32(block + 0x28u);
        count = (int)(word >> 22); /* 69488: ten-bit entry count */
        if (count > 0) {
            entry = dest + (word & 0x3FFFFFu);
            for (i = 0; i < count; i++) {
                (void)func_8006E1C0(entry, dest);
                entry += 0x14u;
            }
        }
        arch = PE_LoadU32(GA_OVERLAY + 0x18Cu);
        key = 0x73DECD80u;
        while (arch != 0u) {
            found = func_8006E498(arch, key);
            if (found == 0u)
                break;
            dims = PE_LoadU32(found + 8u);
            rc.x = (int16_t)((dims >> 10) & 0x7FFu);
            rc.y = (int16_t)(dims >> 21);
            rc.w = (int16_t)(dims & 0x3FFu);
            h = PE_LoadU8(found + 7u);
            if (h == 0u)
                h = 0x100u;
            else
                h &= 0xFFu;
            rc.h = (int16_t)h;
            off = PE_LoadU32(found + 4u) & 0x00FFFFFFu;
            func_8007506C(&rc, found + off);
            key += 4u;
        }
        PE_StoreU8(GA_OVERLAY + 0xEFu, 0u);
        PE_StoreU32(GA_OVERLAY, PE_LoadU32(GA_OVERLAY) & 0xFFFFFFF7u);
        return 0;
    }
    /* Original69198/691B4: other state values return zero. */
    return 0;
}

/*
 * 144FC state 0x39 at 0x800145DC. jal 6914C(1); v0==1 parks at
 * 0x80014660 (v0=0). Else sb 0x3A and re-dispatch. Named cut does
 * not enter 0x3A / 29810 / mode 7.
 */
int func_800144FC_state39_cut(void)
{
    if (func_8006914C(1) == 1)
        return 0;
    PE_StoreU8(GA_OVERLAY + 0xF4u, 0x3Au);
    return 0;
}

/*
 * 29810 remainder 0x80029854..0x800298FF (43 words). After 20EFC:
 * sb 0 / lw / and 0xFFFFFCFF / sw D_8009D1AC; sw 0 D_8009D1A8;
 * sb 0 D_8009D1CE / D_8009D235; sw 0 D_8009D304; sh 0 D_8009D21C;
 * 10 pairs {0,-1} at 0x800A7FF0; 7 words at 0x800B8A90 (lui 0x800C
 * + signed addiu 0x8A90). Does not write mode 7 or +0xE.
 */
void func_80029810_remainder_cut(void)
{
    unsigned int word;
    unsigned int i;

    PE_StoreU8(0x8009D1ACu, 0u);
    word = PE_LoadU32(0x8009D1ACu);
    PE_StoreU32(0x8009D1A8u, 0u);
    PE_StoreU8(0x8009D1CEu, 0u);
    PE_StoreU8(0x8009D235u, 0u);
    PE_StoreU32(0x8009D304u, 0u);
    PE_StoreU16(0x8009D21Cu, 0u);
    PE_StoreU32(0x8009D1ACu, word & 0xFFFFFCFFu);
    for (i = 0; i < 10u; i++) {
        PE_StoreU16(0x800A7FF0u + i * 4u, 0u);
        PE_StoreU16(0x800A7FF2u + i * 4u, 0xFFFFu);
    }
    for (i = 0; i < 7u; i++)
        PE_StoreU32(0x800B8A90u + i * 4u, 0u);
}

/*
 * 29810 prologue 0x80029810..0x8002984B (15 words). lw D_8009D254,
 * lw *actor, sw 0 D_8009D1E8 / D_8009D290 / D_8009D28C (mode 0, not
 * 7), sb 0 D_8009CE7C / D_8009CE78 / D_8009D288 / D_8009CE74, sw
 * *actor → D_8009D278. Does not write 7 or +0xE.
 */
void func_80029810_prologue_cut(void)
{
    pe_addr_t actor;

    actor = PE_LoadU32(GA_ACTOR_P);
    PE_StoreU32(0x8009D1E8u, 0u);
    PE_StoreU32(0x8009D290u, 0u);
    PE_StoreU32(0x8009D28Cu, 0u);
    PE_StoreU8(0x8009CE7Cu, 0u);
    PE_StoreU8(0x8009CE78u, 0u);
    PE_StoreU8(0x8009D288u, 0u);
    PE_StoreU8(0x8009CE74u, 0u);
    PE_StoreU32(GA_RECORD_P, PE_LoadU32(actor));
}

/*
 * 29810 through 293F4: prologue + 20EFC + remainder + 71A64 +
 * 293F4(0) + after_hp tail. 144FC 0x3A is the sole jal.
 */
void func_80029810_cut(unsigned int encounter)
{
    func_80029810_prologue_cut();
    func_80020EFC();
    func_80029810_remainder_cut();
    func_80071A64(D_8009D250);
    func_800293F4(0u);
    func_80029810_after_hp_cut(encounter);
}

/*
 * 144FC state 0x3A at 0x800145F8. D1A0 |= 2, then
 * lw v0, 0(s1); lbu a0, 0(v0); jal 29810. s1 is 144FC a0 =
 * binder arg0 (mode 0 = address of the immediate word).
 * NYPD 0x55(2) therefore does lbu(2), not lbu(overlay) and not
 * the encounter value 2. APPROXIMATION: host RAM[2]==0.
 * sb F4=0x3B, j park v0=0.
 */
int func_800144FC_state3A_cut(pe_addr_t arg0)
{
    pe_addr_t value;

    D_8009D1A0 |= 2u;
    value = PE_LoadU32(arg0);
    func_80029810_cut(PE_LoadU8(pe_kseg0(value)));
    PE_StoreU8(GA_OVERLAY + 0xF4u, 0x3Bu);
    return 0;
}

/* func_800144FC: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800144FC_port.c (src/func_800144FC.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

#define GA_GP_5C  0x8009CDCCu
#define GA_B0E64  0x800B0E64u
#define GA_D270   0x8009D270u

/* func_80087198: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80087198_port.c (src/func_80087198.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_80087414: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80087414_port.c (src/func_80087414.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/*
 * func_8006D078 is 117 words (0x8006D078..0x8006D24C), not the
 * 357-word span to 6D60C (that includes 6D24C and 6D2B8).
 * +0xF3 JT 0x80011458: 0 / 0x28 / 0x29 / 0x2A / 0x2B; default v0=0.
 * No +0xE store. State 0: sw 0 → gp+0x5C, sb 0x28, re-dispatch.
 * State 0x28 jals 6CDA4(1,1,0,lw +0x194,0x21,0); v0==1 returns 1.
 * v0!=1 && +0x10>=2 sb 0x29 return 1; else sb 0x2A and re-dispatch.
 * 0x2A walks D_800B0E64 records (8B, count = word24>>22). Empty
 * or exhausted → sb 0 return 0. bit0x10 && lhu+4>=2 → sb 0x2B.
 * 0x2B jals 6CDA4(3,lhu+4,lhu+6,…). 87198 is not stubbed.
 */
void func_8006D078_state0_cut(void)
{
    PE_StoreU32(GA_GP_5C, 0u);
    PE_StoreU8(GA_OVERLAY + 0xF3u, 0x28u);
}

static pe_addr_t func_8006D078_rec(unsigned int index, unsigned int *count_out)
{
    pe_addr_t base;
    pe_addr_t s2;
    unsigned int word24;

    base = PE_LoadU32(GA_B0E64);
    if (base == 0) {
        if (count_out)
            *count_out = 0;
        return 0;
    }
    s2 = base + PE_LoadU32(base + 4u);
    word24 = PE_LoadU32(s2 + 0x24u);
    if (count_out)
        *count_out = word24 >> 22; /* 6D180: srl v0,v0,22 */
    return base + (word24 & 0x3FFFFFu) + index * 8u;
}

int func_8006D078_state2A_cut(void)
{
    unsigned int index;
    unsigned int count;
    pe_addr_t rec;

    index = PE_LoadU32(GA_GP_5C);
    rec = func_8006D078_rec(index, &count);
    if (rec == 0 || index >= count) {
        PE_StoreU8(GA_OVERLAY + 0xF3u, 0u);
        return 0;
    }
    if ((PE_LoadU8(rec + 3u) & 0x10u) != 0 && PE_LoadU16(rec + 4u) >= 2u) {
        PE_StoreU8(GA_OVERLAY + 0xF3u, 0x2Bu);
        return 1;
    }
    PE_StoreU32(GA_GP_5C, index + 1u);
    return 1;
}

int func_8006D078_state2B_cut(void)
{
    unsigned int index;
    pe_addr_t rec;
    pe_addr_t dest;

    index = PE_LoadU32(GA_GP_5C);
    rec = func_8006D078_rec(index, 0);
    dest = PE_LoadU32(GA_OVERLAY + 0x194u);
    if (func_8006CDA4(3, (int)PE_LoadU16(rec + 4u),
                      (int)PE_LoadU16(rec + 6u), dest, 0x21, 0) == 1)
        return 1;
    PE_StoreU8(GA_OVERLAY + 0xF3u, 0x2Au);
    PE_StoreU32(GA_GP_5C, index + 1u);
    return 1;
}

int func_8006D078(void)
{
    /* Audit item 20: line-for-line src/func_8006D078.c.  The state machine
     * loops within one call (goto loop) instead of returning a frame early. */
    const pe_addr_t s = 0x800B0CD8u;
    pe_addr_t h = PE_LoadU32(0x800B0E64u);
    pe_addr_t dir = h + PE_LoadU32(h + 4u);
    pe_addr_t tab = h + (PE_LoadU32(dir + 0x24u) & 0x3FFFFFu);
    pe_addr_t e;

    for (;;) {
        switch (PE_LoadU8(s + 0xF3u)) {
        case 0:
            PE_StoreU32(0x8009CDCCu, 0u);
            PE_StoreU8(s + 0xF3u, 0x28u);
            continue;
        case 0x28:
            if (func_8006CDA4(1, 1, 0, PE_LoadU32(s + 0x194u), 0x21, 0) == 1)
                return 1;
            if (PE_LoadU8(s + 0x10u) < 2u) {
                PE_StoreU8(s + 0xF3u, 0x2Au);
                continue;
            }
            PE_StoreU8(s + 0xF3u, 0x29u);
            return 1;
        case 0x29:
            if (func_8006CDA4(1, (int)PE_LoadU8(s + 0x10u), 0, PE_LoadU32(s + 0x194u), 0x21, 0) == 1)
                return 1;
            PE_StoreU8(s + 0xF3u, 0x2Au);
            continue;
        case 0x2A:
            if ((int32_t)PE_LoadU32(0x8009CDCCu) < (int32_t)(PE_LoadU32(dir + 0x24u) >> 22)) {
                pe_addr_t p = (PE_LoadU32(0x8009CDCCu) << 3) + tab;
                if (!(PE_LoadU8(p + 3u) & 0x10u) || PE_LoadU16(p + 4u) < 2u)
                    PE_StoreU32(0x8009CDCCu, PE_LoadU32(0x8009CDCCu) + 1u);
                else
                    PE_StoreU8(s + 0xF3u, 0x2Bu);
                continue;
            }
            PE_StoreU8(s + 0xF3u, 0u);
            return 0;
        case 0x2B:
            e = (PE_LoadU32(0x8009CDCCu) << 3) + tab;
            if (func_8006CDA4(3, (int)PE_LoadU16(e + 4u), (int)PE_LoadU16(e + 6u),
                              PE_LoadU32(s + 0x194u), 0x21, 0) == 1)
                return 1;
            PE_StoreU8(s + 0xF3u, 0x2Au);
            PE_StoreU32(0x8009CDCCu, PE_LoadU32(0x8009CDCCu) + 1u);
            continue;
        default:
            return 0;
        }
    }
}

#define GA_XA_TABLE  0x8009317Cu
#define GA_PEIMG_LBA 0x800B0DD8u
#define GA_GP_400    0x8009D170u
#define GA_GP_404    0x8009D174u
#define GA_GP_408    0x8009D178u
#define GA_GP_40C    0x8009D17Cu

extern int func_8006E6D4(int lba_base, int lba_off, pe_addr_t dest, int size);
extern int func_8006E7E8(void);

/* Complete6CDA4 control flow lives in func_8006CDA4_port.c. */

/* 871AC..87414, plus 8A02C: stream a music sample bank in disc-sized
 * chunks. The first AKAO chunk carries instrument descriptors; later
 * chunks append sample bytes at the saved SPU address. */
int func_800871AC(pe_addr_t buffer, uint32_t bytes)
{
    uint32_t size, destination;
    if (PE_LoadU32(0x8009D270u)&1u) {
        uint32_t count, total, offset, i, bank_flags;
        pe_addr_t descriptors, player, dest_table;
        if (func_80085084(buffer)) return -1;
        total=PE_LoadU32(buffer+0x14u);
        offset=PE_LoadU32(buffer+0x18u);
        count=PE_LoadU32(buffer+0x1Cu);
        count=(count ? count : 256u)-offset;
        descriptors=buffer+0x40u;
        size=bytes-0x40u-count*64u;
        if (size>total) size=total;
        destination=0x8000u;
        player=PE_LoadU32(0x8009D2C8u);
        bank_flags=PE_LoadU32(0x8009CDE8u)&~0x100u;
        if (count<49u && player) {
            pe_addr_t slot=PE_LoadU32(player+0x6Cu) ? player+0x68u : player;
            if (PE_LoadU32(slot+4u) && !(PE_LoadU32(slot)&0x100u)) {
                destination=0x38000u;
                bank_flags|=0x100u;
            }
        }
        PE_StoreU32(0x8009CDE8u,bank_flags);
        func_80085EB4(destination);
        func_800850F4(descriptors+count*64u,size);
        PE_StoreU32(0x8009D2BCu,destination+size);
        PE_StoreU32(0x8009D2E4u,total-size);
        /* 8A02C relocates two sample addresses per 64-byte instrument. */
        offset=destination-PE_LoadU32(descriptors);
        for (i=0;i<count;i++) {
            pe_addr_t d=descriptors+i*64u;
            PE_StoreU32(d,PE_LoadU32(d)+offset);
            PE_StoreU32(d+4u,PE_LoadU32(d+4u)+offset);
        }
        dest_table=0x800B2900u+(destination==0x8000u ? 0x800u : 0x1400u);
        for (i=0;i<count*16u;i++)
            PE_StoreU32(dest_table+i*4u,PE_LoadU32(descriptors+i*4u));
        PE_StoreU32(0x8009D270u,PE_LoadU32(0x8009D270u)&~1u);
    } else {
        destination=PE_LoadU32(0x8009D2BCu);
        size=PE_LoadU32(0x8009D2E4u);
        if (size>bytes) size=bytes;
        func_80085EB4(destination);
        func_800850F4(buffer,size);
        PE_StoreU32(0x8009D2BCu,destination+size);
        PE_StoreU32(0x8009D2E4u,PE_LoadU32(0x8009D2E4u)-size);
    }
    return (int32_t)PE_LoadU32(0x8009D2E4u);
}

static void pe_audio_bank_descriptors(pe_addr_t source, uint32_t destination,
                                      uint32_t count, pe_addr_t table)
{
    uint32_t offset=destination-PE_LoadU32(source), i;
    for (i=0;i<count;i++) {
        pe_addr_t d=source+i*64u;
        PE_StoreU32(d,PE_LoadU32(d)+offset);
        PE_StoreU32(d+4u,PE_LoadU32(d+4u)+offset);
    }
    for (i=0;i<count*16u;i++)
        PE_StoreU32(table+i*4u,PE_LoadU32(source+i*4u));
}

/* 87428..875FC: the same chunk protocol for a battle sound bank. */
int func_80087428(unsigned bank, pe_addr_t buffer, uint32_t bytes)
{
    uint32_t size, destination;
    if (PE_LoadU32(0x8009D270u)&2u) {
        uint32_t count,total,offset;
        pe_addr_t descriptors=buffer+0x40u;
        if (func_80085084(buffer)) return -1;
        total=PE_LoadU32(buffer+0x14u);
        offset=PE_LoadU32(buffer+0x18u);
        count=PE_LoadU32(buffer+0x1Cu);
        count=(count ? count : 256u)-offset;
        size=bytes-0x40u-count*64u;
        if (size>total) size=total;
        destination=0x4F000u+bank*0xA000u;
        func_80085EB4(destination);
        func_800850F4(descriptors+count*64u,size);
        PE_StoreU32(0x8009D1ECu,destination+size);
        PE_StoreU32(0x8009D204u,total-size);
        pe_audio_bank_descriptors(descriptors,destination,count,0x800B4D00u+bank*1024u);
        PE_StoreU32(0x8009D270u,PE_LoadU32(0x8009D270u)&~2u);
    } else {
        destination=PE_LoadU32(0x8009D1ECu);
        size=PE_LoadU32(0x8009D204u);
        if (size>bytes) size=bytes;
        func_80085EB4(destination);
        func_800850F4(buffer,size);
        PE_StoreU32(0x8009D1ECu,destination+size);
        PE_StoreU32(0x8009D204u,PE_LoadU32(0x8009D204u)-size);
    }
    return (int32_t)PE_LoadU32(0x8009D204u);
}

/* 875FC..87708: small, complete effect bank in slot zero or one. */
int func_800875FC(unsigned bank, pe_addr_t buffer)
{
    uint32_t count,offset,destination;
    pe_addr_t descriptors=buffer+0x40u;
    if (bank&~1u) return 1;
    if (func_80085084(buffer)) return -1;
    count=PE_LoadU32(buffer+0x1Cu);
    offset=PE_LoadU32(buffer+0x18u);
    count=(count ? count : 256u)-offset;
    destination=0x68000u+bank*8192u;
    func_80085EB4(destination);
    func_800850F4(descriptors+count*64u,PE_LoadU32(buffer+0x14u));
    pe_audio_bank_descriptors(descriptors,destination,count,0x800B4900u+bank*1024u);
    return 0;
}
