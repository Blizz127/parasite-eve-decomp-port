/* Retail command-panel drawing, 314E4..32B0C, authority 20EE0.s.
 * Uses the packets initialized by 30894 and the original font atlas. */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "pe_sdk.h"

static pe_addr_t command_ot(void)
{
    return PE_LoadU32(0x800B0E38u+(uint32_t)PE_LoadU32(0x8009CDDCu)*4u);
}
static pe_addr_t command_anchor(void) { return 0x8009E360u+(uint32_t)PE_LoadU32(0x8009CDDCu)*48u; }
static pe_addr_t command_weapon(void) { return PE_LoadU32(PE_LoadU32(0x8009D278u)+0x68u); }
static void panel_xy(pe_addr_t sprite, int32_t x, int32_t y)
{
    PE_StoreU16(sprite+8u,(uint16_t)x); PE_StoreU16(sprite+10u,(uint16_t)y);
}

void func_80031D6C(int8_t height)
{
    unsigned i;
    pe_addr_t anchor=command_anchor();
    for (i=0;i<3;i++) {
        pe_addr_t tile=anchor-8u+i*16u;
        panel_xy(tile,PE_LoadU16(anchor)+(int)i,PE_LoadU16(anchor+2u)+(int)i);
        PE_StoreU16(tile+12u,(uint16_t)(80u-i*2u));
        PE_StoreU16(tile+14u,(uint16_t)((int)height-(int)i*2));
        func_80077AC4(command_ot()+(7u-i)*4u,tile);
    }
}

int32_t func_800328DC(pe_addr_t digits, int16_t x, int16_t y, int16_t number, int16_t tint)
{
    uint8_t values[6];
    int32_t value=number, last=0, i;
    do {
        values[last++]=(uint8_t)(value%10); value/=10;
    } while (value);
    --last; x=(int16_t)(x-last*6);
    for (i=last;i>=0;i--) {
        pe_addr_t packet=digits+(uint32_t)i*28u;
        PE_StoreU16(packet+16u,(uint16_t)x); PE_StoreU16(packet+18u,(uint16_t)y);
        PE_StoreU8(packet+20u,(uint8_t)(values[i]*8u)); PE_StoreU8(packet+21u,0xF2u);
        if (tint==1) {
            pe_addr_t color=0x8009E46Cu+(uint32_t)PE_LoadU32(0x8009CDDCu)*28u;
            PE_StoreU8(packet+12u,(uint8_t)(PE_LoadU8(color)+40u));
            PE_StoreU8(packet+13u,PE_LoadU8(color+1u)); PE_StoreU8(packet+14u,PE_LoadU8(color+2u));
        }
        func_80077AC4(command_ot()+16u,packet);
        x=(int16_t)(x+6);
    }
    return last;
}

/* func_80056C14: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80056C14_port.c (src/func_80056C14.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

void func_8003205C(void)
{
    uint32_t bank=(uint32_t)PE_LoadU32(0x8009CDDCu), i, count=PE_LoadU8(0x8009D1DCu);
    pe_addr_t anchor=command_anchor(), weapon=command_weapon();
    uint32_t ammo_type=PE_LoadU32(weapon+12u)&0x300000u;
    int32_t x=PE_LoadU16(anchor), y=PE_LoadU16(anchor+2u)+4;
    if (ammo_type==0x200000u) {
        pe_addr_t sprite=0x8009E508u+bank*280u;
        PE_StoreU8(sprite+12u,0xE0u); PE_StoreU8(sprite+13u,0xE0u);
        PE_StoreU16(sprite+16u,8u); PE_StoreU16(sprite+18u,16u);
        panel_xy(sprite,x+60,y);
        PE_StoreU16(sprite+14u,(uint16_t)func_80077AA4(0x130u,0x1FDu));
        func_80077AC4(command_ot()+16u,sprite-8u);
        return;
    }
    for (i=0;i<count;i++) {
        pe_addr_t sprite=0x8009E508u+bank*280u+i*28u;
        uint32_t u=0xD4u, clut=0x1FAu;
        if (ammo_type==0x300000u) { u=0xD8u; clut=0x1FBu; }
        else if ((PE_LoadU32(weapon+16u)&0xC0u)==0x80u) { u=0xDCu; clut=0x1FCu; }
        PE_StoreU16(sprite+16u,4u); PE_StoreU16(sprite+18u,16u);
        PE_StoreU8(sprite+12u,(uint8_t)u); PE_StoreU8(sprite+13u,0xE0u);
        PE_StoreU16(sprite+14u,(uint16_t)func_80077AA4(0x130u,clut));
        panel_xy(sprite,x+64-(int)i*6,y);
        func_80077AC4(command_ot()+16u,sprite-8u);
    }
}

/* func_800323C8: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800323C8_port.c (src/func_800323C8.c); hand port retired (audit batch, port3). */

void func_800325DC(void)
{
    pe_addr_t anchor=command_anchor(), weapon=command_weapon();
    pe_addr_t sprite=0x8009E770u+(uint32_t)PE_LoadU32(0x8009CDDCu)*28u;
    uint32_t packed=PE_LoadU32(weapon+12u);
    int32_t ammo=(int32_t)func_80056C14(((packed>>20u)&3u)-1u)+(packed&1023u), i=0;
    int32_t x=PE_LoadU16(anchor), y=PE_LoadU16(anchor+2u)+22;
    panel_xy(sprite,x+8,y); func_80077AC4(command_ot()+16u,sprite-8u);
    while ((int8_t)i<(int8_t)func_80021054()) {
        int16_t command=(int16_t)PE_LoadU16(0x800BE834u+(uint32_t)(int32_t)(int8_t)i*8u);
        if (command==1 || command==0x189) --ammo;
        else if (command==2) {
            uint32_t flags=PE_LoadU32(weapon+16u);
            if ((flags&0xC0u)==0xC0u) ammo-=(int32_t)(flags&15u);
            else if ((flags&0xC0u)==0x40u) --ammo;
        }
        ++i;
    }
    if ((int16_t)ammo<0) ammo=0;
    (void)func_800328DC(0x8009E7A0u+(uint32_t)PE_LoadU32(0x8009CDDCu)*112u,
        (int16_t)(x+64),(int16_t)(y-1),(int16_t)ammo,0);
}

void func_800327D8(int16_t number, int32_t vertical)
{
    pe_addr_t anchor=command_anchor(), sprite=0x8009E888u+(uint32_t)PE_LoadU32(0x8009CDDCu)*28u;
    int32_t x=PE_LoadU16(anchor), y=(int32_t)((uint32_t)PE_LoadU16(anchor+2u)+(uint32_t)vertical);
    panel_xy(sprite,x+8,y); func_80077AC4(command_ot()+16u,sprite-8u);
    (void)func_800328DC(0x8009E8B8u+(uint32_t)PE_LoadU32(0x8009CDDCu)*56u,
        (int16_t)(x+64),(int16_t)(y-1),number,1);
}

void func_80031E68(void)
{
    unsigned i, count=(PE_LoadU32(command_weapon()+16u)>>4u)&3u;
    pe_addr_t anchor=command_anchor();
    for (i=0;i<count;i++) {
        pe_addr_t sprite=0x8009E3C0u+(uint32_t)PE_LoadU32(0x8009CDDCu)*84u+i*28u;
        PE_StoreU8(sprite+12u,(uint8_t)((int)i==(int8_t)PE_LoadU8(0x8009D2D8u)-1 ? 104u+i*24u:176u));
        PE_StoreU8(sprite+13u,224u);
        panel_xy(sprite,PE_LoadU16(anchor)+(int)i*24,PE_LoadU16(anchor+2u)-8);
        func_80077AC4(command_ot()+28u,sprite-8u);
    }
}

void func_80031760(pe_addr_t model, int8_t out_of_range)
{
    uint32_t bank=(uint32_t)PE_LoadU32(0x8009CDDCu), i,j;
    pe_addr_t sprite=0x8009E468u+bank*28u, triangle=0x8009E4D8u+bank*20u;
    pe_addr_t lines=0x8009E498u+bank*32u, anchor=command_anchor();
    int32_t x=(int16_t)PE_LoadU16(model+0x64u), y=(int16_t)PE_LoadU16(model+0x66u);
    uint32_t rotation=PE_LoadU32(0x800966ECu+((D_8009D250<<6u)&4095u)*4u);
    int16_t sine=(int16_t)rotation, cosine=(int16_t)(rotation>>16u);
    uint32_t red=out_of_range?50u:150u, green=out_of_range?10u:20u;
    panel_xy(sprite,x-12,y-12);
    PE_StoreU8(sprite+4u,(uint8_t)red); PE_StoreU8(sprite+5u,(uint8_t)green); PE_StoreU8(sprite+6u,(uint8_t)green);
    PE_StoreU8(triangle+4u,(uint8_t)red); PE_StoreU8(triangle+5u,(uint8_t)green); PE_StoreU8(triangle+6u,(uint8_t)green);
    /* 794C4 with only Z rotation, then 78E04/78E94 and three 792D4
     * MVMVA operations. Transient matrices/vectors stay in native storage. */
    for (i=0;i<3;i++) for (j=0;j<3;j++) g_pe_gte.rt[i][j]=0;
    g_pe_gte.rt[0][0]=cosine; g_pe_gte.rt[0][1]=(int16_t)-sine;
    g_pe_gte.rt[1][0]=sine; g_pe_gte.rt[1][1]=cosine; g_pe_gte.rt[2][2]=4096;
    g_pe_gte.tr[0]=x; g_pe_gte.tr[1]=y; g_pe_gte.tr[2]=0;
    for (i=0;i<3;i++) {
        pe_addr_t v=0x80010DE4u+i*8u;
        PE_GTE_SetV0((int16_t)PE_LoadU16(v),(int16_t)PE_LoadU16(v+2u),(int16_t)PE_LoadU16(v+4u));
        PE_GTE_MVMVA(0x480012u);
        PE_StoreU16(triangle+8u+i*4u,(uint16_t)g_pe_gte.mac[0]);
        PE_StoreU16(triangle+10u+i*4u,(uint16_t)g_pe_gte.mac[1]);
    }
    func_80077AC4(command_ot()+20u,triangle);
    {
        int32_t near_x=x+8, far_x=x+35, panel_x=far_x;
        int32_t near_y=y+8, far_y=y+35;
        if (x+134>=301) { near_x=x-8; far_x=x-35; panel_x=x-115; }
        if (y>=82) { near_y=y-8; far_y=y-35; }
        PE_StoreU16(lines+8u,(uint16_t)near_x); PE_StoreU16(lines+24u,(uint16_t)near_x);
        PE_StoreU16(lines+12u,(uint16_t)far_x); PE_StoreU16(lines+28u,(uint16_t)far_x);
        PE_StoreU16(anchor,(uint16_t)panel_x);
        PE_StoreU16(lines+10u,(uint16_t)near_y); PE_StoreU16(lines+26u,(uint16_t)(near_y+1));
        PE_StoreU16(lines+14u,(uint16_t)far_y); PE_StoreU16(lines+30u,(uint16_t)(far_y+1));
        PE_StoreU16(anchor+2u,(uint16_t)far_y);
    }
    func_80077AC4(command_ot()+28u,lines+16u);
    func_80077AC4(command_ot()+24u,lines);
    func_80077AC4(command_ot()+20u,sprite-8u);
}

void func_800314E4(pe_addr_t model, int8_t mode, int8_t out_of_range, int16_t number)
{
    pe_addr_t anchor=command_anchor();
    if (mode==8 || mode==1 || mode==2) {
        PE_StoreU16(anchor,120u); PE_StoreU16(anchor+2u,mode==8?108u:104u);
    } else func_80031760(model,out_of_range);
    switch (mode) {
    case 0: func_80031D6C(40); func_8003205C(); func_800325DC(); func_800327D8(number,30); break;
    case 1: case 2: func_80031D6C(32); func_800323C8(mode); func_800325DC(); break;
    case 3: {
        pe_addr_t sprite=0x8009E738u+(uint32_t)PE_LoadU32(0x8009CDDCu)*28u;
        func_80031D6C(28);
        panel_xy(sprite,PE_LoadU16(anchor)+28,PE_LoadU16(anchor+2u)+8);
        func_80077AC4(command_ot()+16u,sprite-8u); func_800327D8(number,18); break;
    }
    case 4: func_80031D6C(40); func_800323C8(mode); func_800325DC(); func_800327D8(number,30); break;
    case 5: case 6: case 7: func_80031D6C(32); func_800323C8(mode); func_800327D8(number,22); break;
    case 8: func_80031D6C(24); func_800323C8(8); break;
    default: break;
    }
    if (((PE_LoadU32(command_weapon()+16u)>>4u)&3u)>=2u) func_80031E68();
}
