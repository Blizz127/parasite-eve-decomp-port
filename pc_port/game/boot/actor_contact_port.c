/* SEW18: original actor contact pass and task retirement. Native translation
 * of 36448..36DC8, 35F54..3601C, 12774..12850 and 1D268..1D340. */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game_port.h"
#include <stdio.h>

static int32_t contact_h(pe_addr_t p) {return (int16_t)PE_LoadU16(p);}
static uint32_t contact_square(int32_t x) {return (uint32_t)x*(uint32_t)x;}
static int32_t contact_radius(pe_addr_t actor,unsigned offset)
{return (contact_h(actor+offset)*(int32_t)PE_LoadU16(actor+38u))/4096;}
static int32_t contact_distance(pe_addr_t a,pe_addr_t b,int three_d)
{
    uint32_t d=contact_square(contact_h(a)-contact_h(b));
    d+=contact_square(contact_h(a+4u)-contact_h(b+4u));
    if (three_d) d+=contact_square(contact_h(a+2u)-contact_h(b+2u));
    return (int32_t)d;
}

void func_80012774(void)
{
    pe_addr_t actor=PE_LoadU32(0x8009D20Cu);
    while (actor) {
        for (unsigned i=0;i<3u;i++) {
            pe_addr_t head=actor+0xA0u+i*4u,task=PE_LoadU32(head);
            while (task) {
                pe_addr_t next=PE_LoadU32(task+36u);
                if ((PE_LoadU32(actor+0x98u)&16u) || (PE_LoadU16(task+8u)&16u)) {
                    pe_addr_t previous=PE_LoadU32(task+40u);
                    if (previous) PE_StoreU32(previous+36u,next);
                    else PE_StoreU32(head,next);
                    if (next) PE_StoreU32(next+40u,PE_LoadU32(task+40u));
                    previous=PE_LoadU32(0x8009CDFCu);
                    PE_StoreU32(0x8009CDFCu,task);PE_StoreU32(task+36u,previous);
                }
                task=next;
            }
        }
        actor=PE_LoadU32(actor+4u);
    }
}

static void contact_restore(pe_addr_t actor)
{
    uint32_t x=PE_LoadU32(actor+64u),y=PE_LoadU32(actor+68u),z=PE_LoadU32(actor+72u);
    pe_addr_t floor=PE_LoadU32(actor+0x1A8u);
    PE_StoreU32(actor+40u,x);PE_StoreU32(actor+44u,y);PE_StoreU32(actor+48u,z);
    PE_StoreU32(actor+0x1A4u,floor);
}

/* Retail 3601C..360B4: one list pass, preserving parent/child ordering.
 * Flag400000 copies the parent's pose before model projection. */
void func_8003601C(void)
{
    for (pe_addr_t actor=PE_LoadU32(0x8009D20Cu);actor;actor=PE_LoadU32(actor+4u)) {
        pe_addr_t parent=PE_LoadU32(actor+0x18Cu);
        if (!parent || !(PE_LoadU32(actor+0x98u)&0x400000u)) continue;
        for (unsigned i=0;i<3u;i++)
            PE_StoreU32(actor+40u+i*4u,PE_LoadU32(PE_LoadU32(actor+0x18Cu)+40u+i*4u));
        for (unsigned i=0;i<3u;i++)
            PE_StoreU16(actor+56u+i*2u,PE_LoadU16(PE_LoadU32(actor+0x18Cu)+56u+i*2u));
    }
}

/* Retail 360B4..361F4, including effect release and actor free-list writes.
 * 363F4 removes the first matching model allocation. 3D82C is the original
 * two-instruction return-one leaf and has no memory effects. */
void func_800360B4(void)
{
    pe_addr_t actor=PE_LoadU32(0x8009D20Cu);
    while (actor) {
        uint32_t flags=PE_LoadU32(actor+0x98u);
        pe_addr_t parent=PE_LoadU32(actor+0x18Cu);
        PE_StoreU32(actor+0x98u,flags&0xFF7FFFFFu);
        if ((flags&16u) || (parent && (PE_LoadU32(parent+0x98u)&16u))) {
            pe_addr_t next,previous,free_head;
            (void)func_8006FE14(actor);
            if (PE_Port_ShouldStop()) return;
            next=PE_LoadU32(actor+4u);
            if (actor==PE_LoadU32(0x8009D254u)) {
                PE_StoreU32(0x8009D254u,0u);
                PE_StoreU32(0x8009D2E8u,PE_LoadU32(0x8009D2E8u)&0xFFFFFFF2u);
            }
            if (PE_LoadU32(actor+0x1ACu)) {
                pe_addr_t allocation=PE_LoadU32(actor+0x278u);
                for (unsigned i=0;i<16u;i++) {
                    pe_addr_t entry=0x800A7624u+i*8u;
                    if (PE_LoadU32(entry)==allocation) {PE_StoreU32(entry,0u);break;}
                }
            }
            previous=PE_LoadU32(actor+8u);
            if (!previous) {
                free_head=PE_LoadU32(0x8009D2ACu);
                PE_StoreU32(0x8009D2ACu,actor);
                PE_StoreU32(0x8009D20Cu,PE_LoadU32(actor+4u));
                /* Retail also stores through physical 8 when next is zero. */
                PE_StoreU32((next<0x200000u?next|0x80000000u:next)+8u,0u);
                PE_StoreU32(actor+4u,free_head);
            } else {
                PE_StoreU32(previous+4u,PE_LoadU32(actor+4u));
                if (PE_LoadU32(actor+4u))
                    PE_StoreU32(PE_LoadU32(actor+4u)+8u,PE_LoadU32(actor+8u));
                free_head=PE_LoadU32(0x8009D2ACu);
                PE_StoreU32(0x8009D2ACu,actor);
                PE_StoreU32(actor+4u,free_head);
            }
            PE_StoreU16(0x8009D2A6u,(uint16_t)(PE_LoadU16(0x8009D2A6u)-1u));
            actor=next;
        } else actor=PE_LoadU32(actor+4u);
    }
}

void func_80035F54(pe_addr_t actor)
{
    pe_addr_t parent=PE_LoadU32(actor+0x18Cu);
    if (parent) {
        func_80035F54(parent);
        for (pe_addr_t p=PE_LoadU32(0x8009D20Cu);p;p=PE_LoadU32(p+4u))
            if (PE_LoadU32(p+0x18Cu)==PE_LoadU32(actor+0x18Cu)) contact_restore(p);
    } else {
        contact_restore(actor);
        PE_StoreU32(actor+0x98u,PE_LoadU32(actor+0x98u)|0x40000u);
    }
}

void func_8001D268(pe_addr_t actor,int32_t own_part,pe_addr_t other,int32_t part)
{
    pe_addr_t record=PE_LoadU32(other),action;
    (void)own_part;
    if (!record) return;
    action=PE_LoadU32(record+24u);
    /* Retail 1D280 reads KUSEG RAM, including address zero between
     * attacks. Use its cached alias, preserving the byte read and branch.
     * Run39 rehearsal contact reaches this with action==0. */
    if (action<0x200000u) action|=0x80000000u;
    if (PE_LoadU8(action)!=1u ||
        ((PE_LoadU32(record)>>21u)&7u)>=3u || (int32_t)PE_LoadU32(record+16u)<=0) return;
    part=(int16_t)part;
    for (unsigned i=0;i<4u;i++) {
        unsigned offset=(PE_LoadU32(record)>>19u)&28u;
        int value=(int8_t)PE_LoadU8(record+0x7Cu+offset+i);
        if (value==part) {
            pe_addr_t body=PE_LoadU32(actor);
            PE_StoreU32(body+76u,PE_LoadU32(body+76u)|0x4000u);
            PE_StoreU32(record,PE_LoadU32(record)|0x80000000u);
            return;
        }
        if (value<0) return;
    }
}

static void contact_task(pe_addr_t actor,pe_addr_t other)
{
    pe_addr_t entry=PE_LoadU32(actor+0x1A0u),task;
    if (!entry) return;
    for (task=PE_LoadU32(actor+0xA4u);task;task=PE_LoadU32(task+36u)) {
        unsigned flags=PE_LoadU16(task+8u);
        if ((flags&1u) && !(flags&16u) &&
            PE_LoadU32(task+12u)==PE_LoadU16(other+36u)) return;
    }
    task=func_80012700(entry,0u);
    PE_StoreU16(task+8u,PE_LoadU16(task+8u)|1u);
    PE_StoreU32(task+12u,PE_LoadU16(other+36u));
    PE_StoreU32(task+24u,PE_LoadU8(other+12u));PE_StoreU32(task+28u,PE_LoadU8(other+13u));
    entry=PE_LoadU32(actor+0xA4u);
    if (entry) {PE_StoreU32(task+36u,entry);PE_StoreU32(entry+40u,task);}
    PE_StoreU32(actor+0xA4u,task);
}

static void contact_pair_tasks(pe_addr_t a,pe_addr_t b)
{contact_task(a,b);contact_task(b,a);}

static void contact_callback(pe_addr_t fn,pe_addr_t actor,int part,pe_addr_t other,int other_part)
{
    if (!fn) return;
    if (fn==0x8001D268u) {func_8001D268(actor,part,other,other_part);return;}
    fprintf(stderr,"[CONTACT] Unported callback %08X\n",fn);
    Bootstrap_ReturnVoid("func_80036448_callback","actor contact");
    PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
}

static void contact_spheres(pe_addr_t a,pe_addr_t b)
{
    if (PE_LoadU8(a+12u) && PE_LoadU8(b+12u)) {
        PE_StoreU32(a+0x98u,PE_LoadU32(a+0x98u)|0x40000u);
        PE_StoreU32(b+0x98u,PE_LoadU32(b+0x98u)|0x40000u);
        return;
    }
    pe_addr_t sa=PE_LoadU32(a+0x234u);
    for (unsigned i=0;i<PE_LoadU8(PE_LoadU32(a+0x1B4u)+3u);i++,sa+=12u) {
        int32_t ra=contact_h(sa+8u)*(int32_t)PE_LoadU16(a+38u)/4096;
        pe_addr_t sb=PE_LoadU32(b+0x234u);
        for (unsigned j=0;j<PE_LoadU8(PE_LoadU32(b+0x1B4u)+3u);j++,sb+=12u) {
            int32_t rb=contact_h(sb+8u)*(int32_t)PE_LoadU16(b+38u)/4096;
            if ((int32_t)contact_square(ra+rb)<contact_distance(sa,sb,1)) continue;
            contact_callback(PE_LoadU32(a+0x194u),a,contact_h(sa+6u),b,contact_h(sb+6u));
            contact_callback(PE_LoadU32(b+0x194u),b,contact_h(sb+6u),a,contact_h(sa+6u));
        }
    }
}

void func_80036448(void)
{
    pe_addr_t a,b;
    for (a=PE_LoadU32(0x8009D20Cu);a;a=PE_LoadU32(a+4u))
        PE_StoreU32(a+0x98u,PE_LoadU32(a+0x98u)&0xFDFBFFFFu);
    for (a=PE_LoadU32(0x8009D20Cu);a;a=PE_LoadU32(a+4u)) {
        int32_t ar,abroad;
        if (PE_LoadU32(a+0x98u)&32u) continue;
        ar=contact_radius(a,0x224u);abroad=contact_radius(a,0x230u);
        for (b=PE_LoadU32(a+4u);b;b=PE_LoadU32(b+4u)) {
            pe_addr_t aya=PE_LoadU32(0x8009D254u);
            int32_t radius,distance;
            if (PE_LoadU32(a+0x18Cu)==b || PE_LoadU32(b+0x18Cu)==a ||
                ((PE_LoadU32(a+0x98u)&0x20000u) && b!=aya) ||
                ((PE_LoadU32(b+0x98u)&0x20000u) && a!=aya) ||
                (PE_LoadU32(b+0x98u)&32u)) continue;
            radius=abroad+contact_radius(b,0x230u);
            if ((int32_t)contact_square(radius)<contact_distance(a+0x228u,b+0x228u,1)) continue;
            if (!PE_LoadU32(a+0x1ACu) || !PE_LoadU32(b+0x1ACu)) {
                contact_pair_tasks(a,b);continue;
            }
            if (contact_h(a+0x224u) && contact_h(b+0x224u)) {
                radius=ar+contact_radius(b,0x224u);
                distance=contact_distance(a+0x21Cu,b+0x21Cu,0);
                if ((int32_t)contact_square(radius)>=distance) {
                    int32_t dx=contact_h(a+0x21Cu)-contact_h(b+0x21Cu);
                    int32_t dz=contact_h(a+0x220u)-contact_h(b+0x220u);
                    int32_t mx=(int32_t)(PE_LoadU32(a+40u)-PE_LoadU32(a+64u))>>16;
                    int32_t mz=(int32_t)(PE_LoadU32(a+48u)-PE_LoadU32(a+72u))>>16;
                    if ((int32_t)((uint32_t)mx*(uint32_t)dx+(uint32_t)mz*(uint32_t)dz)<0) func_80035F54(a);
                    mx=(int32_t)(PE_LoadU32(b+40u)-PE_LoadU32(b+64u))>>16;
                    mz=(int32_t)(PE_LoadU32(b+48u)-PE_LoadU32(b+72u))>>16;
                    if ((int32_t)((uint32_t)mx*(uint32_t)dx+(uint32_t)mz*(uint32_t)dz)>0) func_80035F54(b);
                    aya=PE_LoadU32(0x8009D254u);
                    if (a!=aya && !(PE_LoadU32(a+0x98u)&0x1000000u)) PE_StoreU16(a+58u,PE_LoadU16(a+82u));
                    if (b!=aya && !(PE_LoadU32(b+0x98u)&0x1000000u)) PE_StoreU16(b+58u,PE_LoadU16(b+82u));
                    if (a==aya) PE_StoreU32(b+0x98u,PE_LoadU32(b+0x98u)|0x2000000u);
                    else if (b==aya) PE_StoreU32(a+0x98u,PE_LoadU32(a+0x98u)|0x2000000u);
                    contact_pair_tasks(a,b);
                    if (!(D_8009D1A0&2u)) continue;
                } else if (!(D_8009D1A0&2u) &&
                           (int32_t)(contact_square(radius)+10000u)>=distance) {
                    contact_pair_tasks(a,b);continue;
                }
            }
            contact_spheres(a,b);
        }
    }
}
