/* Original list navigation, scrolling and owner-chain event dispatch.
 * 53CD4.s / 556B4.s / 4E92C.s. Guest callbacks stay native functions. */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game_port.h"
#include "pe_guestcode.h"
#include <stdio.h>

static uint32_t menu_callback(pe_addr_t fn,uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3)
{
    switch (fn) {
    case 0x80048838u:return (uint32_t)func_80048838(a0,a1);
    case 0x80058030u:return (uint32_t)func_80058030(a0,(int32_t)a1,a2,(int32_t)a3);
    case 0x8004DA9Cu:return (uint32_t)func_8004DA9C();
    case 0x8004D2DCu:return (uint32_t)func_8004D2DC(a0,a1);
    case 0x8004F730u:return (uint32_t)func_8004F730(a0,a1);
    case 0x80045D0Cu:return (uint32_t)func_80045D0C(a0,a1);
    case 0x8004620Cu:return (uint32_t)func_8004620C(a0,a1);
    case 0x800466C0u:return (uint32_t)func_800466C0(a0,a1);
    case 0x800471E4u:return (uint32_t)func_800471E4(a0,a1);
    case 0x80047354u:return (uint32_t)func_80047354(a0,a1);
    case 0x800474D0u:return (uint32_t)func_800474D0(a0,a1);
    case 0x80046B58u:return (uint32_t)func_80046B58(a0,a1);
    case 0x800452C0u:return (uint32_t)func_800452C0(a0,a1);
    case 0x80044B0Cu:return (uint32_t)func_80044B0C(a0,a1);
    case 0x80043DA4u:return (uint32_t)func_80043DA4(a0,a1);
    case 0x8004D030u:return (uint32_t)func_8004D030(a0,a1);
    case 0x80044E98u:return (uint32_t)func_80044E98(a0,a1);
    case 0x80044444u:return (uint32_t)func_80044444(a0,a1);
    case 0x80057C54u:return (uint32_t)func_80057C54(a0,(int32_t)a1,a2,(int32_t)a3);
    case 0x80050260u:func_80050260();return 0u;
    case 0x80063E0Cu:return (uint32_t)func_80063E0C(a0,a1);
    case 0x800650E0u:return (uint32_t)func_800650E0(a0,a1);
    case 0x8004E074u:return (uint32_t)func_8004E074(a0,a1);
    case 0x8004E2E4u:return (uint32_t)func_8004E2E4(a0,a1);
    case 0x8004BB80u:return (uint32_t)func_8004BB80(a0,a1);
    case 0x8004C1E0u:return (uint32_t)func_8004C1E0(a0,a1);
    /* Field-menu tree rooted at 0x8004AE1C (jtbl_80011034).  Every input
     * handler the tree installs is native now: the page handler, the four
     * sub-page constructors, and the Items/Escape/Equipment/Modal handlers.
     * The Equipment draw side lives in menu_draw_callback (0x8004B214). */
    case 0x8004AE1Cu:return (uint32_t)func_8004AE1C(a0,a1);
    case 0x8004AFA4u:return (uint32_t)func_8004AFA4(a0,a1);
    case 0x8004B0A4u:return (uint32_t)func_8004B0A4(a0,a1);
    case 0x8004B394u:return (uint32_t)func_8004B394(a0,a1);
    case 0x8004B650u:return (uint32_t)func_8004B650(a0,a1);
    default:
        /* Any other retail input handler the guest-code registry resolves
         * (generated TUs, canonical-header ports) is the retail `jalr`
         * target itself (port lane round 6). */
        if (PE_GuestCode_Resolve(fn))
            return (uint32_t)PE_GuestCall("menu input callback",fn,4u,a0,a1,a2,a3);
        fprintf(stderr,"[MENU] Unported input callback %08X\n",fn);
        Bootstrap_ReturnVoid("PE_MenuInputCallback","menu input callback");
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);return 0u;
    }
}

static int32_t menu_word(pe_addr_t node,uint32_t offset)
{ return (int32_t)PE_LoadU32(node+offset); }

static int32_t menu_add(int32_t a,int32_t b)
{ return (int32_t)((uint32_t)a+(uint32_t)b); }

static int32_t menu_sub(int32_t a,int32_t b)
{ return (int32_t)((uint32_t)a-(uint32_t)b); }

static void menu_scroll(pe_addr_t node,int32_t amount)
{
    int32_t old=menu_word(node,92u),next=menu_add(old,amount);
    int32_t maximum=menu_sub(menu_word(node,88u),menu_word(node,56u));
    if (next<0) next=0;
    else if (maximum<next) next=maximum;
    PE_StoreU32(node+92u,(uint32_t)next);
    if (next!=old) {
        int32_t distance=menu_word(node,64u);
        if (amount<=0) distance=menu_sub(0,distance);
        PE_StoreU32(node+96u,(uint32_t)(distance/2));
    }
}

int func_800650E0(pe_addr_t node,uint32_t event)
{
    pe_addr_t list=PE_LoadU32(node+52u);int handled=0;
    if (event&0x500Cu) {
        int32_t old=menu_word(list,92u);
        handled=1;
        if (!PE_LoadU32(list+96u)) {
            int32_t amount=menu_word(list,56u);
            if (event&0x1004u) amount=menu_sub(0,amount);
            menu_scroll(list,amount);
            if (menu_word(list,92u)!=old) func_8005267C();
        }
    }
    return handled || !(PE_LoadU32(node+64u)&64u);
}

static int32_t menu_saved_index(pe_addr_t node)
{
    if (node && menu_word(node,76u)>=0 && menu_word(node,80u)>=0)
        return (int32_t)(PE_LoadU32(node+52u)*PE_LoadU32(node+80u)+PE_LoadU32(node+76u));
    return -1;
}

static int menu_transfer(pe_addr_t node,pe_addr_t other,int same)
{
    int32_t current,saved;uint32_t id,other_id;
    if (!other || menu_word(other,76u)<0) return 0;
    current=func_80063428(node);saved=menu_saved_index(other);
    id=PE_LoadU32(node+36u);other_id=PE_LoadU32(other+36u);
    if ((same || id==other_id) && current==saved) return 0;
    if (menu_callback(PE_LoadU32(node+132u),id,(uint32_t)current,other_id,(uint32_t)saved)) {
        PE_StoreU32(other+76u,UINT32_MAX);func_800525EC();
    } else func_800526C4();
    return 1;
}

static void menu_restore_cursor(pe_addr_t node)
{
    int32_t row,top,rows;
    PE_StoreU32(node+68u,PE_LoadU32(node+76u));PE_StoreU32(node+72u,PE_LoadU32(node+80u));
    PE_StoreU32(node+76u,UINT32_MAX);
    row=menu_word(node,72u);top=menu_word(node,92u);rows=menu_word(node,56u);
    if (row<top) PE_StoreU32(node+92u,(uint32_t)row);
    else if (row>=menu_add(top,rows)) PE_StoreU32(node+92u,(uint32_t)menu_add(menu_sub(row,rows),1));
}

int func_80063E0C(pe_addr_t node,uint32_t event)
{
    int handled=0;int32_t row,column,extra;pe_addr_t other;
    if (PE_LoadU32(0x8009D0E8u) && (func_8005E038()&32u)) return 1;
    if (event&0x1000u) {
        row=menu_word(node,72u);
        if (row>0) {PE_StoreU32(node+72u,(uint32_t)menu_sub(row,1));handled=1;func_8005267C();}
        else if (PE_LoadU32(node+100u)&16u) {
            PE_StoreU32(node+72u,PE_LoadU32(node+88u)-1u);handled=1;func_8005267C();
        }
        if (menu_word(node,72u)<menu_word(node,92u) && !PE_LoadU32(node+96u)) menu_scroll(node,-1);
        return handled || !(PE_LoadU32(node+100u)&4u);
    }
    if (event&0x4000u) {
        extra=PE_LoadU32(node+68u) && PE_LoadU32(node+104u);
        row=menu_word(node,72u);
        if (row<menu_sub(menu_word(node,88u),extra+1)) {
            PE_StoreU32(node+72u,(uint32_t)menu_add(row,1));handled=1;func_8005267C();
        } else if (PE_LoadU32(node+100u)&16u) {PE_StoreU32(node+72u,0u);handled=1;func_8005267C();}
        row=menu_word(node,72u);
        extra=extra && row==menu_sub(menu_word(node,88u),2);
        if (row>=menu_sub(menu_add(menu_word(node,92u),menu_word(node,56u)),extra) &&
            !PE_LoadU32(node+96u)) menu_scroll(node,1);
        return handled || !(PE_LoadU32(node+100u)&8u);
    }
    if (event&0x8000u) {
        column=menu_word(node,68u);
        if (column>0) {
            PE_StoreU32(node+68u,column==6?4u:(uint32_t)menu_sub(column,1));handled=1;func_8005267C();
        } else if ((other=PE_LoadU32(node+120u))!=0u) {
            int32_t maximum=menu_sub(menu_word(other,88u),1);
            PE_StoreU32(node+68u,UINT32_MAX);PE_StoreU32(other+68u,PE_LoadU32(other+52u)-1u);
            row=menu_add(menu_sub(menu_word(node,72u),menu_word(node,92u)),menu_word(other,92u));
            if (row<maximum) maximum=row;
            PE_StoreU32(other+72u,(uint32_t)maximum);
            extra=PE_LoadU32(other+104u) && maximum==menu_sub(menu_word(other,88u),1);
            PE_StoreU32(other+68u,PE_LoadU32(other+52u)-(uint32_t)extra-1u);
            func_80062CB8(other);handled=1;func_8005267C();
        }
        extra=PE_LoadU32(0x8009D0E8u) && (func_8005E038()&0x5000u);
        return handled || !(PE_LoadU32(node+100u)&1u) || extra;
    }
    if (event&0x2000u) {
        column=menu_word(node,68u);
        if (column>=0) {
            extra=menu_word(node,72u)==menu_sub(menu_word(node,88u),1) && PE_LoadU32(node+104u);
            if (column<menu_sub(menu_word(node,84u),extra+1)) {
                PE_StoreU32(node+68u,column==4?6u:(uint32_t)menu_add(column,1));handled=1;func_8005267C();
            } else if ((other=PE_LoadU32(node+124u))!=0u) {
                int32_t maximum=menu_sub(menu_word(other,88u),1);
                PE_StoreU32(node+68u,UINT32_MAX);PE_StoreU32(other+68u,0u);
                row=menu_add(menu_sub(menu_word(node,72u),menu_word(node,92u)),menu_word(other,92u));
                if (row<maximum) maximum=row;
                PE_StoreU32(other+72u,(uint32_t)maximum);func_80062CB8(other);handled=1;func_8005267C();
            }
        }
        extra=PE_LoadU32(0x8009D0E8u) && (func_8005E038()&0x5000u);
        return handled || !(PE_LoadU32(node+100u)&2u) || extra;
    }
    if (event&0x10000u) {
        if (!PE_LoadU32(node+132u)) return 0;
        handled|=menu_transfer(node,node,1);
        handled|=menu_transfer(node,PE_LoadU32(node+120u),0);
        handled|=menu_transfer(node,PE_LoadU32(node+124u),0);
        return handled;
    }
    if (event&64u) {
        if (menu_word(node,76u)>=0) {
            menu_restore_cursor(node);
            if (PE_LoadU32(node+136u)) (void)menu_callback(PE_LoadU32(node+136u),0u,0u,0u,0u);
            handled=1;func_80052634();
        }
        other=PE_LoadU32(node+120u);
        if (other && menu_word(other,76u)>=0) {
            menu_restore_cursor(other);PE_StoreU32(node+68u,UINT32_MAX);func_80062CB8(other);handled=1;
        }
        other=PE_LoadU32(node+124u);
        if (other && menu_word(other,76u)>=0) {
            menu_restore_cursor(other);PE_StoreU32(node+68u,UINT32_MAX);func_80062CB8(other);handled=1;
        }
        return handled;
    }
    if (PE_LoadU32(node+128u) && func_80062CC4()==node) {
        if (event&4u) {
            row=menu_sub(menu_word(node,72u),menu_word(node,56u));
            if (row<0) row=0;
            PE_StoreU32(node+72u,(uint32_t)row);(void)func_800650E0(PE_LoadU32(node+128u),0x1000u);
        } else if (event&8u) {
            int32_t maximum=menu_sub(menu_word(node,88u),1+(PE_LoadU32(node+68u)==1u && PE_LoadU32(node+104u)));
            row=menu_add(menu_word(node,72u),menu_word(node,56u));
            if (row>maximum) row=maximum;
            PE_StoreU32(node+72u,(uint32_t)row);(void)func_800650E0(PE_LoadU32(node+128u),0x4000u);
        }
        return 0;
    }
    return (event&32u)!=0u;
}

void func_8005E30C(void)
{
    pe_addr_t focus=func_80062CC4(),event,previous=0u;
    uint32_t type=0u,buttons=0u;
    if (!PE_LoadU32(0x8009D0ECu)) {
        pe_addr_t address=focus+40u;
        func_8005E12C((int32_t)PE_LoadU32(address<0x200000u?address|0x80000000u:address));
    }
    event=PE_LoadU32(0x8009D0E0u);
    while (event && !PE_LoadU32(event+4u)) {previous=event;event=PE_LoadU32(event);}
    if (event) {
        if (previous) PE_StoreU32(previous,PE_LoadU32(event));
        else PE_StoreU32(0x8009D0E0u,PE_LoadU32(event));
        if (PE_LoadU32(0x8009D0E4u)==event) PE_StoreU32(0x8009D0E4u,previous);
        PE_StoreU32(event,PE_LoadU32(0x8009D0DCu));PE_StoreU32(0x8009D0DCu,event);
        type=PE_LoadU32(event+4u);buttons=PE_LoadU32(event+8u);
    }
    if (type==4u && (buttons&32u)) {focus=func_80062CC4();buttons=0x10000u;}
    else if (type==1u || type==2u) {if (type==2u) buttons|=0x20000u;}
    else return;
    while (focus) {
        if (menu_callback(PE_LoadU32(focus+44u),focus,buttons,0u,0u)) break;
        if (PE_Port_ShouldStop()) break;
        focus=PE_LoadU32(focus+4u);
    }
}
