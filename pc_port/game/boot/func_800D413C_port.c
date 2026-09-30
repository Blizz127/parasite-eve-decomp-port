/* Original effect scheduler, room-effect VM and particle pools (5F484.s,
 * BEC9C.s, BF0F0.s). Effect bytecode stays data; callbacks execute natively. */
#include "psx_compat.h"
#include "pe_guestcode.h"
#include "pe_port_compat.h"
#include "pe_sdk.h"
#include "game_port.h"
#include <stdio.h>

/* Retail passes a fourth register (a3) to descriptor command callbacks;
 * D4698 publishes it here before dispatching (SEW12). */
static uint32_t g_pe_effect_extra1;
void PE_EffectCallback_SetExtra1(uint32_t extra1) { g_pe_effect_extra1 = extra1; }

/* Six original stack bytes shared by M0023I callbacks in the 69594 pump.
 * Track their writers, including the retained joint Z on paused frames.
 * Unknown call graphs invalidate the bytes instead of supplying a position. */
enum { EFFECT_STACK_NONE,EFFECT_STACK_INIT,EFFECT_STACK_UPDATE,EFFECT_STACK_DRAW,
       EFFECT_STACK_PARTICLE_UPDATE,EFFECT_STACK_PARTICLE_DRAW,EFFECT_STACK_WEAPON_DRAW,
       EFFECT_STACK_WEAPON_UPDATE };
static struct { uint64_t generation;unsigned active,context,known;int16_t position[3]; } effect_stack;
void PE_EffectStackInvalidate(void) { effect_stack.known=0; }
void PE_EffectStackBegin(void)
{
    if(effect_stack.generation!=PE_RamGeneration()) {
        effect_stack.generation=PE_RamGeneration();effect_stack.known=0;
    }
    effect_stack.active=1;effect_stack.context=EFFECT_STACK_NONE;
    /* The next particle prologue establishes X/Y at this draw depth. */
    effect_stack.known&=4u;
}
void PE_EffectStackEnd(void) { effect_stack.active=0;effect_stack.context=EFFECT_STACK_NONE; }
int PE_EffectStackWeaponDraw(pe_addr_t fn,pe_addr_t slot)
{
    unsigned saved=effect_stack.context;effect_stack.context=EFFECT_STACK_WEAPON_DRAW;
    int result=fn==0x800C9B68u?func_800C9B68(slot):
        fn==0x800CE144u?func_800CE144((int32_t)slot):func_800CD8C8(slot);
    effect_stack.context=saved;return result;
}
int PE_EffectStackWeaponUpdate(pe_addr_t fn,pe_addr_t slot)
{
    /* At the 69594 update depth, C9B90/CD8F0/CE16C and their C251C/C2758
     * frames do not reach the flash's borrowed six bytes. Empty child
     * lists and bytecode without a constructor therefore preserve them.
     * Deeper callbacks remain unknown until their stack writers are bound. */
    unsigned saved=effect_stack.context;effect_stack.context=EFFECT_STACK_WEAPON_UPDATE;
    int result=fn==0x800C9B90u?func_800C9B90(slot):
        fn==0x800CE16Cu?func_800CE16C(slot):func_800CD8F0(slot);
    effect_stack.context=saved;return result;
}
void PE_EffectStackWeaponUpdateCall(void)
{
    if(effect_stack.active && effect_stack.context==EFFECT_STACK_WEAPON_UPDATE)
        effect_stack.known=0;
}
void PE_EffectStackWeaponCallback(pe_addr_t fn,pe_addr_t data)
{
    if(!effect_stack.active || effect_stack.context!=EFFECT_STACK_WEAPON_DRAW)return;
    switch(fn) {
    case 0x800C9EA0u:case 0x800CDD04u:case 0x800CE3ACu:return; /* Original empty leaves. */
    case 0x800C9EA8u:
        /* C9ED8/DC/F8: casing angles at sp+30 overlap the borrowed vector. */
        effect_stack.position[0]=0;effect_stack.position[1]=0;
        effect_stack.position[2]=(int16_t)((int32_t)(int8_t)PE_LoadU8(data+1u)*64);
        break;
    case 0x800C9FD8u:
        /* CA444/450: last muzzle plane copies scale.z and the next word
         * into sp+78/+7C, at this same 69594 draw depth. */
        for(unsigned i=0;i<3;i++)effect_stack.position[i]=(int16_t)PE_LoadU16(0x800C21BCu+i*2u);
        break;
    default:effect_stack.known=0;return;
    }
    effect_stack.known=7u;
}
static int effect_context_call(unsigned context,pe_addr_t fn,int32_t mode,pe_addr_t data,pe_addr_t extra)
{
    unsigned saved=effect_stack.context;effect_stack.context=context;
    int result=PE_EffectCallback(fn,mode,data,extra);
    effect_stack.context=saved;return result;
}
static int m0023i_code(void)
{
    return PE_LoadU32(0x8018F3C8u)==0x27BDFFD0u && PE_LoadU32(0x8018F3CCu)==0x00803821u &&
        PE_LoadU32(0x8018F49Cu)==0x0C033958u && PE_LoadU32(0x8018F708u)==0x03E00008u;
}

int PE_EffectCallback(pe_addr_t fn, int32_t mode, pe_addr_t data, pe_addr_t extra)
{
    PE_M34StackCallback(fn);
    (void)data; (void)extra;
    int rehearsal=m0023i_code();
    if(effect_stack.active && (!rehearsal || effect_stack.context==EFFECT_STACK_INIT ||
       effect_stack.context==EFFECT_STACK_NONE))effect_stack.known=0;
    if (PE_MirrorOverlay()) {
        if(effect_stack.active)effect_stack.known=0;
        if (fn==0x8018F0F0u) return PE_MirrorDraw(data);
        if (fn==0x8018EFF4u || fn==0x8018F1A4u || fn==0x8018F1BCu) return 0;
        if (fn==0x8018F1ACu) {PE_StoreU8(data,4u);return 0;}
    }
    if (PE_M34BossEffectOverlay()) {
        if(fn==0x8018F0E4u)return PE_M34BossEffectDraw(data);
        if(fn==0x8018F12Cu)return PE_M34BossEffectUpdate(data);
        if(fn==0x8018F1B8u)return PE_M34BossEffectCleanup(data);
        if(fn==0x8018F23Cu || fn==0x8018F244u)return 0;
    }
    if (PE_M0348iEffectOverlay()) {
        if (fn==0x8018F0C4u) return PE_M0348iEffectCallback(data);
        if (fn==0x8018F128u) return PE_M0348iEffectUpdate(data);
        if (fn==0x8018F1B4u) return PE_M0348iEffectCleanup(data);
    }
    if (PE_M32MovementOverlay()) {
        if (fn==0x801917ECu) return PE_M32MovementUpdate(data);
        if (fn==0x80192090u) return PE_M28MovementCleanup(data);
        if (fn==0x80191504u || fn==0x801917E4u || fn==0x801920ECu) return 0;
    }
    if (PE_M28MovementOverlay()) {
        if (fn==0x80192700u) return PE_M28ProjectileMain(mode,data,extra);
        if (fn==0x801924F8u) return PE_M0013I_Particle(mode,data);
        if (fn==0x801917FCu) return PE_M28MovementUpdate(data);
        if (fn==0x801920A0u) return PE_M28MovementCleanup(data);
        if (fn==0x80191514u || fn==0x801917F4u || fn==0x801920FCu) return 0;
        if (fn==0x80193148u) {
            PE_StoreU32(0x80193288u,data);PE_StoreU32(0x8019328Cu,extra);
            PE_StoreU32(0x80193290u,g_pe_effect_extra1);
            return (int32_t)0x80193288u;
        }
    }
    /* Overlay addresses are reused: identify M0013I's main prologue
     * before dispatching either of its callbacks. */
    if (PE_LoadU32(0x8018F20Cu)==0x27BDFF98u &&
        PE_LoadU32(0x8018F210u)==0x00803821u &&
        PE_LoadU32(0x8018F214u)==0xAFB00058u &&
        PE_LoadU32(0x8018FC4Cu)==0x03E00008u) {
        if (fn==0x8018F20Cu) return PE_M0013I_Main(mode,data,extra);
        if (fn==0x8018F004u) return PE_M0013I_Particle(mode,data);
    }
    /* M0013I (backstage corridor) room command callback: 9 words at
     * 8018FC54 = lui/addiu 8018FCEC; sw a1,0; sw a2,+4; sw a3,+8; jr ra.
     * Keyed on the code words so another overlay at this address cannot
     * match. Returns the block address, which D4698 stores at slot+0x14. */
    if (fn==0x8018FC54u && PE_LoadU32(fn)==0x3C028019u && PE_LoadU32(fn+4u)==0x2442FCECu &&
        PE_LoadU32(fn+8u)==0xAC450000u && PE_LoadU32(fn+28u)==0x03E00008u) {
        PE_StoreU32(0x8018FCECu, data);
        PE_StoreU32(0x8018FCF0u, extra);
        PE_StoreU32(0x8018FCF4u, g_pe_effect_extra1);
        return (int32_t)0x8018FCECu;
    }
    /* M0023I shares F004 with other room overlays. Match its main code. */
    if (rehearsal) {
        if (fn==0x8018F3C8u) {
            int result=PE_M0023I_Main(mode,data);
            if(effect_stack.active && effect_stack.context==EFFECT_STACK_UPDATE && mode==1) {
                /* CE934 at this update depth stores zero over borrowed Z. */
                effect_stack.position[2]=0;effect_stack.known|=4u;
            }
            return result;
        }
        if (fn==0x8018F004u) {
            if(effect_stack.active && effect_stack.context==EFFECT_STACK_PARTICLE_DRAW && mode==2) {
                /* F014 saves the CE78C return address800CE818. */
                effect_stack.position[0]=(int16_t)0xE818u;effect_stack.position[1]=(int16_t)0x800Cu;
                effect_stack.known|=3u;
            }
            return PE_M0023I_Particle(mode,data);
        }
        if (fn==0x8018FC14u) {
            if(effect_stack.active)effect_stack.known=0;
            return PE_M0023I_Beam(mode,data);
        }
        if (fn==0x8018F710u) {
            int16_t *retained=0;
            if(effect_stack.active && effect_stack.context==EFFECT_STACK_DRAW && mode==2 &&
               (effect_stack.known==7u || (int16_t)PE_LoadU16(data+4u)!=0))retained=effect_stack.position;
            int result=PE_M0023I_Flash(mode,data,retained);
            if(retained && !PE_Port_ShouldStop())effect_stack.known=7u;
            return result;
        }
        /* Eleven-word descriptor command; +0x34 is bytecode, not a callback. */
        if (fn==0x80190644u) {
            if (mode==1) PE_StoreU16(PE_LoadU32(0x800E2368u)+18u,1);
            return (int32_t)0x80190758u;
        }
    }
    if(effect_stack.active)effect_stack.known=0;
    if (fn==0x8018F330u) return func_8018F330(mode,data);
    if (fn==0x8018F018u) return func_8018F018(mode,data);
    if (fn==0x8018F614u) return func_8018F614(mode,data);
    if (fn==0x8018FB84u) return func_8018FB84(mode,data);
    if (fn==0x8018FDC4u) return func_8018FDC4(mode,data);
    if (fn==0x800D751Cu) return func_800D751C(mode,data);
    if (fn==0x800D71B8u) return func_800D71B8(mode,data);
    if (fn==0x800D70C0u) return func_800D70C0(mode,data);
    if (fn==0x800D4C24u) return func_800D4C24(mode,data);
    if (fn==0x800D4928u) return func_800D4928(mode,data);
    if (fn==0x800DF87Cu) return func_800DF87C(mode,data);
    /* M0005's descriptor command callback (80190A6C..80190A98). */
    if (fn==0x80190A6Cu) {
        if (mode==1) PE_StoreU16(PE_LoadU32(0x800E2368u)+18u,1u);
        return (int32_t)0x80190B80u;
    }
    if (PE_GuestCode_Resolve(fn))   /* generated overlay / EXE TU (pe_guestcode.h) */
        return PE_GuestCall("@800D413C:effect", fn, 2u, (uintptr_t)(uint32_t)mode, (uintptr_t)data, 0, 0);
    if (g_pe_strict_effect_stack) {
        fprintf(stderr,"[EFFECT] Unported callback %08X mode %d\n",fn,mode);
        Bootstrap_ReturnVoid("PE_EffectCallback", "room effect callback");
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
        return 0;
    }
    /* DAY1 SHIM (docs/ai_context/DAY1_FIDELITY_GAPS.md, "unported room effect
     * callback"): room effect callbacks are the visual effect pool
     * (init/update/draw).  An unported one is skipped instead of stopping the
     * game: mode 1 (update) reports finished so the pool retires the record,
     * modes 0/2 do nothing.  Logged once per distinct fn. */
    {
        static pe_addr_t seen[32]; static unsigned nseen;
        unsigned k;
        for (k = 0; k < nseen && seen[k] != fn; k++) {}
        if (k == nseen && nseen < 32u) {
            seen[nseen++] = fn;
            fprintf(stderr,"[DAY1_SHIM] PE_EffectCallback: room effect %08X (mode %d, token %08X) not ported; skipped\n",
                    fn, mode, (unsigned)PE_LoadU32(0x8009D280u));
        }
    }
    return mode == 1 ? 1 : 0;
}

int func_8006DC18(uint32_t code)
{
    uint32_t key=0x73DECD80u;
    pe_addr_t package=PE_LoadU32(0x800B0E64u);
    if (package<0x200000u) package|=0x80000000u;
    for (;;) {
        pe_addr_t entry=func_8006E498(package,key);
        uint32_t flags;
        if (!entry) return -1;
        flags=PE_LoadU32(entry+8u);
        if (PE_LoadU8(entry+3u)==code && (flags&0x1FFC00u)==0xD0000u)
            return (flags&0xFFE00000u)!=0x20000000u;
        key+=4u;
    }
}

int32_t func_800D3FD8(void)
{
    pe_addr_t actor=PE_LoadU32(PE_LoadU32(0x800F32D0u)+8u);
    pe_addr_t body=PE_LoadU32(actor);
    int32_t id=body?(int32_t)PE_LoadU32(body+8u):128;
    return id<65?id:128;
}

uint32_t func_800CE560(pe_addr_t pool, uint32_t size, int32_t count, pe_addr_t fn)
{
    int32_t i;
    uint32_t stride=size+4u;
    PE_StoreU32(pool,stride);PE_StoreU32(pool+4u,(uint32_t)count);PE_StoreU32(pool+8u,fn);
    for (i=0;i<count;i++) PE_StoreU16(pool+12u+(uint32_t)i*stride,0u);
    return stride*(uint32_t)count+12u;
}

uint32_t func_800CE5AC(pe_addr_t out, uint32_t offset, uint32_t size, int32_t count, pe_addr_t fn)
{
    pe_addr_t pool=PE_LoadU32(PE_LoadU32(0x800F33E0u)+8u)+offset;
    PE_StoreU32(out,pool);
    return func_800CE560(pool,size,count,fn);
}

/* func_800CE610: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800CE610_port.c (src/func_800CE610.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md, round 6). */

static int pe_particles(pe_addr_t pool,int mode)
{
    uint32_t saved=PE_LoadU32(0x800E27ECu),stride=PE_LoadU32(pool);
    pe_addr_t callback=PE_LoadU32(pool+8u),p=pool+12u;
    int32_t i,active=0;
    for (i=0;i<(int32_t)PE_LoadU32(pool+4u);i++,p+=stride) {
        int result;
        if (!(int16_t)PE_LoadU16(p)) continue;
        active++;
        PE_StoreU32(0x800E27ECu,(uint32_t)(int32_t)(int16_t)PE_LoadU16(p+2u));
        result=effect_context_call(mode==1?EFFECT_STACK_PARTICLE_UPDATE:EFFECT_STACK_PARTICLE_DRAW,
            callback,mode,p+4u,PE_LoadU32(PE_LoadU32(0x800E2368u)+8u));
        if (mode==1) {
            if (result) PE_StoreU16(p,0u);
            else PE_StoreU16(p+2u,(uint16_t)(PE_LoadU16(p+2u)+1u));
        }
    }
    PE_StoreU32(0x800E27ECu,saved);
    return active;
}

int func_800CE688(pe_addr_t pool) { return pe_particles(pool,1); }
int func_800CE78C(pe_addr_t pool) { return pe_particles(pool,2); }

void func_800CE870(pe_addr_t actor,int32_t mode,pe_addr_t out)
{
    unsigned i;
    if (mode==0) {
        pe_addr_t matrix=PE_LoadU32(actor+0x238u);
        for (i=0;i<3;i++) PE_StoreU16(out+i*2u,(uint16_t)PE_LoadU32(matrix+20u+i*4u));
    } else if (mode==1)
        for (i=0;i<3;i++) PE_StoreU16(out+i*2u,(uint16_t)(PE_LoadU32(actor+0x28u+i*4u)>>16u));
}

void PE_ActorJointPositionCE8F0(pe_addr_t actor,uint32_t joint,const int16_t local[3],int16_t out[3])
{
    pe_addr_t matrix=PE_LoadU32(actor+0x238u)+joint*32u;
    unsigned i;
    PE_GTE_LoadRT33(matrix);
    for (i=0;i<3;i++) g_pe_gte.tr[i]=0;
    PE_GTE_SetV0(local[0],local[1],local[2]);
    PE_GTE_MVMVA(0x80000u);
    for (i=0;i<3;i++) out[i]=(int16_t)((uint32_t)g_pe_gte.mac[i]+PE_LoadU32(matrix+20u+i*4u));
}

void func_800CE8F0(pe_addr_t actor,uint32_t joint,pe_addr_t local,pe_addr_t out)
{
    int16_t vector[3],position[3];unsigned i;
    for (i=0;i<3;i++) vector[i]=(int16_t)PE_LoadU16(local+i*2u);
    PE_ActorJointPositionCE8F0(actor,joint,vector,position);
    for (i=0;i<3;i++) PE_StoreU16(out+i*2u,(uint16_t)position[i]);
}

int func_800D401C(int32_t index)
{
    pe_addr_t body=PE_LoadU32(0x800E2368u),rec=body+0x20u,data,desc;
    uint16_t size,allocated;
    int32_t bytes;
    unsigned i;
    for (i=0;i<8;i++,rec+=12u) if (PE_LoadU16(rec)==65535u) break;
    if (i==8) return -1;
    PE_StoreU8(body+12u,(uint8_t)(PE_LoadU8(body+12u)+1u));
    PE_StoreU16(rec,(uint16_t)index);PE_StoreU16(rec+2u,0u);
    desc=PE_LoadU32(body+0x80u);size=PE_LoadU16(desc+0x20u+(uint32_t)index*2u);
    allocated=(uint16_t)(PE_LoadU16(body+16u)+size);data=PE_LoadU32(body+4u);
    PE_StoreU16(body+16u,allocated);
    if (allocated>=0x97Du) {
        data=body+0x84u;PE_StoreU32(body+4u,data);PE_StoreU16(body+16u,size);
    }
    PE_StoreU32(rec+8u,data+size);PE_StoreU32(rec+4u,data);
    PE_StoreU32(0x800F33E0u,rec);
    bytes=effect_context_call(EFFECT_STACK_INIT,PE_LoadU32(desc+(uint32_t)PE_LoadU16(rec)*4u),0,data,PE_LoadU32(body+8u));
    if (!bytes) PE_StoreU32(rec+8u,0u);
    PE_StoreU32(body+4u,PE_LoadU32(body+4u)+size+(uint32_t)bytes);
    PE_StoreU16(body+16u,(uint16_t)(PE_LoadU16(body+16u)+(uint32_t)bytes));
    return (int32_t)(size+(uint32_t)bytes);
}

static void pe_room_effect_context(pe_addr_t slot)
{
    PE_StoreU32(0x800F32D0u,slot);PE_StoreU32(0x800E2368u,slot+12u);
}

static pe_addr_t pe_effect_action(pe_addr_t record)
{
    pe_addr_t action=PE_LoadU32(record+24u);
    /* D4488/D44B4 also read physical RAM address zero between attacks.
     * Preserve the original read through its cached RAM alias. */
    return action<0x200000u?action|0x80000000u:action;
}

int func_800D413C(pe_addr_t slot)
{
    pe_addr_t body=slot+12u;
    int run=1;
    unsigned i;
    pe_room_effect_context(slot);
    while (run) {
        pe_addr_t pc,local,owner,record;
        uint16_t op,a,b,delay=PE_LoadU16(body+14u);
        if (delay) {
            PE_StoreU16(body+14u,--delay);
            if (delay) break;
        }
        pc=PE_LoadU32(body);
        /* Opcode 3 can store a physical RAM PC; preserve that value and
         * normalize only the access to the port's checked RAM interface. */
        local=pc<0x200000u?pc|0x80000000u:pc;
        op=PE_LoadU16(local);a=PE_LoadU16(local+2u);b=PE_LoadU16(local+4u);
        PE_StoreU32(body,pc+6u);
        switch (op) {
        case 0:
            PE_StoreU32(body,pc);
            if (PE_LoadU8(body+12u)) { run=0;break; }
            PE_StoreU8(slot,4u);
            run=0;
            if (!PE_LoadU8(PE_LoadU32(0x800E2368u)+13u)) break;
            owner=PE_LoadU32(PE_LoadU32(0x800F32D0u)+8u);record=PE_LoadU32(owner);
            if (record) {
                PE_StoreU32(record,PE_LoadU32(record)&0xC0FFFFFFu);
                PE_StoreU8(pe_effect_action(PE_LoadU32(owner)),4u);
                PE_StoreU8(PE_LoadU32(0x800E2368u)+13u,0u);
            }
            break;
        case 1: PE_StoreU16(body+18u+(uint32_t)(int32_t)(int16_t)a*2u,b);break;
        case 2: (void)func_800D401C((int16_t)a);break;
        case 3:
            local=body+18u+(uint32_t)(int32_t)(int16_t)b*2u;
            PE_StoreU16(local,(uint16_t)(PE_LoadU16(local)-1u));
            if ((int16_t)PE_LoadU16(local)>0) PE_StoreU32(body,(uint32_t)(int32_t)(int16_t)a-6u);
            else { PE_StoreU16(local,0u);run=0; }
            break;
        case 4:
            if ((int16_t)b!=1) { PE_StoreU16(body+14u,a);run=0; }
            else if (!(int16_t)PE_LoadU16(body+18u+(uint32_t)(int32_t)(int16_t)a*2u)) {
                PE_StoreU32(body,pc);run=0;
            }
            break;
        case 5:
            if (!a) {
                record=PE_LoadU32(PE_LoadU32(PE_LoadU32(0x800F32D0u)+8u));
                PE_StoreU32(record,(PE_LoadU32(record)&0xC0FFFFFFu)|0x1000000u);
                PE_StoreU8(PE_LoadU32(0x800E2368u)+13u,1u);
            } else if (PE_LoadU8(PE_LoadU32(0x800E2368u)+13u)) {
                owner=PE_LoadU32(PE_LoadU32(0x800F32D0u)+8u);
                if (owner && (record=PE_LoadU32(owner))!=0u) {
                    local=pe_effect_action(record);
                    if (PE_LoadU8(local)==1u) PE_StoreU8(local,2u);
                }
            }
            break;
        default: break;
        }
        if (PE_Port_ShouldStop()) return 0;
    }
    if (PE_LoadU8(PE_LoadU32(0x800E2368u)+13u)) {
        pe_addr_t owner=PE_LoadU32(PE_LoadU32(0x800F32D0u)+8u),record;
        if (owner && (record=PE_LoadU32(owner))!=0u) {
            uint32_t flags=PE_LoadU32(record),timer=(flags>>24u)&63u;
            if (timer>=2u) PE_StoreU32(record,(flags&0xC0FFFFFFu)|((timer-1u)<<24u));
            if (!(PE_LoadU8(record+3u)&63u) || !PE_LoadU8(pe_effect_action(record))) {
                if ((PE_LoadU8(record+3u)&63u) && (int32_t)PE_LoadU32(record+16u)<=0)
                    PE_StoreU8(slot,4u);
                if (!PE_LoadU8(pe_effect_action(record))) {
                    if (PE_LoadU32(record)&0x180Eu) PE_StoreU8(slot,4u);
                    if (PE_LoadU8(owner+14u)<2u) return 0;
                }
            }
        }
    }
    if (PE_LoadU8(body+13u)) PE_StoreU32(0x800F3428u,(uint32_t)func_8006DC18(PE_LoadU8(slot+1u)));
    for (i=0;i<8;i++) {
        pe_addr_t rec=body+32u+i*12u,pool;
        unsigned index=PE_LoadU16(rec);
        int result;
        if (index==65535u) continue;
        PE_StoreU32(0x800F33E0u,rec);PE_StoreU32(0x800E27ECu,PE_LoadU16(rec+2u));
        result=effect_context_call(EFFECT_STACK_UPDATE,PE_LoadU32(PE_LoadU32(body+128u)+index*4u),1,PE_LoadU32(rec+4u),PE_LoadU32(body+8u));
        pool=PE_LoadU32(rec+8u);
        if (pool) { int active=func_800CE688(pool);if (result==2 && !active) result=1; }
        PE_StoreU16(rec+2u,(uint16_t)(PE_LoadU16(rec+2u)+1u));
        if (result==1) { PE_StoreU16(rec,65535u);PE_StoreU8(body+12u,(uint8_t)(PE_LoadU8(body+12u)-1u)); }
    }
    return 0;
}

int func_800D4704(pe_addr_t slot)
{
    pe_addr_t body=slot+12u;
    unsigned i;
    pe_room_effect_context(slot);
    if (PE_LoadU8(slot+25u)) PE_StoreU32(0x800F3428u,(uint32_t)func_8006DC18(PE_LoadU8(slot+1u)));
    for (i=0;i<8;i++) {
        pe_addr_t rec=body+32u+i*12u,pool;
        unsigned index=PE_LoadU16(rec);
        if (index==65535u) continue;
        PE_GTE_LoadRT(PE_LoadU32(0x800BCFA4u));
        PE_StoreU32(0x800F33E0u,rec);PE_StoreU32(0x800E27ECu,PE_LoadU16(rec+2u));
        (void)effect_context_call(EFFECT_STACK_DRAW,PE_LoadU32(PE_LoadU32(body+128u)+index*4u),2,PE_LoadU32(rec+4u),PE_LoadU32(body+8u));
        if(PE_Port_ShouldStop())return 0;
        pool=PE_LoadU32(rec+8u);if (pool) (void)func_800CE78C(pool);
    }
    return 0;
}

int func_8006F9F0(unsigned index)
{
    pe_addr_t slot,entry,fn;
    unsigned state,code;
    int result;
    if (index>=22u) return -19;
    slot=index<11u?PE_LoadU32(0x800942E4u)+index*0xA0Cu:PE_LoadU32(0x800942E8u)+(index-11u)*0x10Cu;
    state=PE_LoadU8(slot);
    if ((state-1u)>=2u && (state-4u)>=2u) return 0;
    if (state==4u) { PE_StoreU8(slot,5u);return 0; }
    if (state==5u) {
        unsigned i;
        if (PE_LoadU8(slot+1u)==0x72u) {
            for (i=0;i<7;i++) PE_StoreU32(0x800E10A0u+i*4u,0u);
            PE_StoreU32(0x800B0CD8u,PE_LoadU32(0x800B0CD8u)&~0x10000u);
        }
        PE_StoreU8(slot,0u);
        for (i=1;i<4;i++) PE_StoreU8(slot+i,255u);
        PE_StoreU32(slot+4u,0u);PE_StoreU32(slot+8u,0u);
        return 0;
    }
    if (state==1u) PE_StoreU8(slot,2u);
    code=PE_LoadU8(slot+1u);if (code>=192u) return -20;
    entry=PE_LoadU32(PE_LoadU32(0x800942E0u)+(code<85u?code:85u)*4u);
    if (!entry) return -21;
    fn=PE_LoadU32(entry+16u);if (!fn) return -1;
    if (fn==0x800D413Cu) result=func_800D413C(slot);
    else if (fn==0x800C9B90u || fn==0x800CD8F0u || fn==0x800CE16Cu) result=PE_EffectStackWeaponUpdate(fn,slot);
    else {PE_EffectStackInvalidate();result=PE_EffectCallback(fn,1,slot,0u);}
    PE_StoreU32(slot+4u,PE_LoadU32(slot+4u)+1u);
    return result;
}
