/* Card operation processor func_80041108 and its cleanup func_80040F80:
 * full line-for-line translations of the matched C (src/func_80041108.c,
 * src/func_80040F80.c) over guest RAM, with the BIOS file functions served
 * by the host memory-card image (platform/pe_bios_card.c, pe_memcard.c) and
 * the Psy-Q sprintf by the host formatter (PE_Bios_Sprintf).
 *
 * Test frontier: with PE_Memcard_BoundaryMode() on, every BIOS/formatter
 * call instead records the loud boundary the first-unresolved-callee oracle
 * fixtures expect (payload: known-register mask, fifth argument) and the
 * function returns at that call, exactly as before the host card existed.
 *
 * PE_CardOperationFrame / PE_CardCleanupFrame below are the older
 * guest-stack formatter-frame models (their oracle fixture drives them);
 * the live game path is func_80041108 / func_80040F80. */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game_port.h"
#include "pe_sdk.h"
#include "pe_memcard.h"
#include "pe_bios_string.h"
#include <string.h>

int  func_8004D27C(void);
int  func_8004D4C4(int a0, int a1);
void func_8004D298(int a0);
void func_80042264(void);

static void operation_boundary(const char *caller,pe_addr_t target,uint32_t a0,uint32_t a1,
                               uint32_t a2,uint32_t a3,uint32_t known,uint32_t fifth)
{
    uint32_t payload[2]={known,fifth};
    (void)Bootstrap_ReturnInt4Indirect("card operation unresolved call",caller,0,
        target,a0,a1,a2,a3,payload,sizeof(payload));
    PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
}

/* Legacy frame model (formatter-frame oracle only). */
static void legacy_frame_operation(uint32_t index,pe_addr_t frame,const uint32_t *incoming)
{
    pe_addr_t record=0x800A0ED4u+index*0x418u;
    uint32_t state=PE_LoadU8(record+1u),selected;
    if(state==0u || state>=15u)return;
    if(state==13u) {
        if(PE_LoadU8(record)!=5u) {
            PE_StoreU32(0x800A1864u,0xFFFFFFFFu);PE_StoreU8(record+1u,0u);return;
        }
        goto ready;
    }
    if(state==14u) {
        if(incoming) {
            uint32_t saved[9];for(unsigned i=0;i<8;i++)saved[i]=incoming[16u+i];
            saved[0]=record;saved[2]=index;saved[8]=0x80041284u;
            unsigned epoch=PE_Port_StopEpoch();
            (void)PE_FormatterFrame(frame+24u,0x80010F60u,index,incoming[7],frame,saved);
            if(PE_Port_StopEpoch()!=epoch)return;
            operation_boundary("PE_CardOperationFrame",0x80072784u,frame+24u,0u,0u,0u,1u,0u);return;
        }
        /* a0 is the unresolved original frame's sp+18h, not a RAM scratch. */
        operation_boundary("func_80041108",0x80071A84u,0u,0x80010F60u,index,0u,6u,0u);return;
    }
    if(PE_LoadU8(record)!=1u) {
        if(incoming) {
            uint32_t cleanup[32];for(unsigned i=0;i<32;i++)cleanup[i]=incoming[i];
            cleanup[16]=record;cleanup[18]=index;cleanup[31]=0x80042000u;
            if(state>=4u && state<=10u)cleanup[17]=PE_LoadU8(record);
            if(state==11u)cleanup[19]=PE_LoadU8(record);
            PE_CardCleanupFrame(record,frame,cleanup);
        } else func_80040F80(record);
        return;
    }
    switch(state) {
    case 1:
    ready: {
        if(PE_LoadU8(record+8u)!=4u)return;
        uint32_t other=PE_LoadU8(0x800A0EDCu+(index==0u?0x418u:0u));
        if(other!=0u && other!=4u)return;
        PE_StoreU32(0x800A1838u,1u);
        PE_StoreU8(record+1u,state==13u?14u:PE_LoadU8(record+11u));return;
    }
    case 2:
        PE_StoreU8(PE_LoadU32(0x80092230u)+2u,index+0x30u);
        PE_StoreU8(record+2u,0u);PE_StoreU8(record+3u,0u);PE_StoreU8(record+6u,0u);
        PE_StoreU8(record+4u,0u);PE_StoreU8(record+7u,0u);PE_StoreU8(record+10u,0u);
        for(int i=14;i>=0;i--)PE_StoreU8(record+(uint32_t)i*0x44u+28u,2u);
        /* a1 is the unresolved original frame's sp+20h. */
        operation_boundary("func_80041108",0x800727B4u,PE_LoadU32(0x80092230u),incoming?frame+32u:0u,0u,0u,incoming?3u:1u,0u);return;
    case 3: {
        uint32_t cursor=PE_LoadU8(record+6u),limit=PE_LoadU8(record+2u)*2u;
        if(cursor<limit) {
            uint32_t center=PE_LoadU8(record+5u);
            do {
                uint32_t slot=center+(((cursor&1u)*2u-1u)*(((cursor&255u)+1u)>>1));
                if(slot<15u && !PE_LoadU8(record+slot*0x44u+29u))break;
                cursor++;PE_StoreU8(record+6u,cursor);
            } while((cursor&255u)<limit);
        }
        cursor=PE_LoadU8(record+6u);limit=PE_LoadU8(record+2u)*2u;
        selected=cursor<limit?PE_LoadU8(record+5u)+(((cursor&1u)*2u-1u)*((cursor+1u)>>1)):255u;
        PE_StoreU8(record+3u,selected);selected&=255u;
        if(selected==255u) {
            PE_StoreU8(record+1u,12u);PE_StoreU32(0x800A1838u,0u);return;
        }
        goto format_name;
    }
    case 4:
        selected=PE_LoadU8(record+3u);
        PE_StoreU8(record+selected*0x44u+69u,PE_LoadU32(0x800A1704u));
        selected=PE_LoadU8(record+3u);goto format_name;
    case 5:case 6:case 11:
        selected=PE_LoadU8(record+3u);
    format_name:
        if(incoming) {
            uint32_t saved[9];for(unsigned i=0;i<8;i++)saved[i]=incoming[16u+i];
            saved[0]=record;saved[2]=index;
            if(state==4u || state==5u || state==6u)saved[1]=1u;
            if(state==11u) {saved[2]=record>0x800A0ED4u;saved[3]=1u;}
            saved[8]=state==3u?0x800416D8u:state==4u?0x80041834u:
                state==5u?0x80041908u:state==6u?0x800419D4u:0x80041E98u;
            pe_addr_t format=PE_LoadU32(0x80092224u);
            uint32_t variant=PE_LoadU8(record+selected*0x44u+69u)+0x30u;
            PE_StoreU32(frame+16u,selected+0x41u);
            unsigned epoch=PE_Port_StopEpoch();
            (void)PE_FormatterFrame(0x8009EE70u,format,record>0x800A0ED4u,variant,frame,saved);
            if(PE_Port_StopEpoch()!=epoch)return;
            operation_boundary("PE_CardOperationFrame",0x80072734u,0x8009EE70u,
                state==4u?0x10200u:state==6u?2u:1u,0u,0u,3u,0u);return;
        }
        operation_boundary("func_80041108",0x80071A84u,0x8009EE70u,PE_LoadU32(0x80092224u),
            record>0x800A0ED4u,PE_LoadU8(record+selected*0x44u+69u)+0x30u,15u,selected+0x41u);return;
    case 7:case 8:case 9: {
        int32_t remaining=(int16_t)PE_LoadU16(record+20u),cap=state==8u?128:1024;
        operation_boundary("func_80041108",state==9u?0x80072764u:0x80072754u,
            PE_LoadU32(record+12u),PE_LoadU32(record+24u),
            (uint32_t)(remaining<cap+1?remaining:cap),0u,7u,0u);return;
    }
    case 10:
        operation_boundary("func_80041108",0x80072774u,PE_LoadU32(record+12u),0u,0u,0u,1u,0u);return;
    case 12:return;
    }
}


/* Memory/output adapter for a supplied original call context. Cleanup and BIOS
 * remain explicit boundaries; no return/register state is synthesized for them. */
void PE_CardOperationFrame(uint32_t index,pe_addr_t caller_sp,const uint32_t incoming[32])
{
    pe_addr_t frame=caller_sp-0x78u;
    PE_StoreU32(frame+0x68u,incoming[18]);PE_StoreU32(frame+0x60u,incoming[16]);
    PE_StoreU32(frame+0x70u,incoming[31]);PE_StoreU32(frame+0x6Cu,incoming[19]);
    PE_StoreU32(frame+0x64u,incoming[17]);
    legacy_frame_operation(index,frame,incoming);
}

/* func_80042228: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80042228_port.c (src/func_80042228.c); hand port retired (port3 switch-over D). */

void func_8004D5CC(uint32_t index)
{
    unsigned epoch=PE_Port_StopEpoch();pe_addr_t list=func_80062A34(2u,36u);
    func_80062F3C(40u);if(PE_Port_StopEpoch()!=epoch)return;
    pe_addr_t callback=PE_LoadU32(0x8009CFFCu);
    if(callback) {
        switch(callback) {
        case 0x80042910u:func_80042910();break;
        case 0x80042928u:func_80042928();break;
        case 0x8005C488u:func_8005C488();break;
        case 0x80062F9Cu:func_80062F9C();break;
        default:operation_boundary("func_8004D5CC",callback,0u,0u,0u,0u,0u,0u);return;
        }
        if(PE_Port_StopEpoch()!=epoch)return;
        PE_StoreU32(0x8009CFFCu,0u);
    }
    if(index!=PE_LoadU32(0x8009CF44u))return;
    func_80062F3C(63u);if(PE_Port_StopEpoch()!=epoch)return;
    func_80062F3C(39u);if(PE_Port_StopEpoch()!=epoch)return;
    func_80062F1C(func_80062A34(1u,41u));if(PE_Port_StopEpoch()!=epoch)return;
    func_80062F3C(31u);if(PE_Port_StopEpoch()!=epoch)return;
    func_80062F3C(index+37u);if(PE_Port_StopEpoch()!=epoch)return;
    if(list) {
        PE_StoreU32(list+68u,0u);
        if(!func_800631DC())func_80062CB8(list);
    }
}

/* Original40F80 frame through the first unresolved callee. The direct native
 * entry above continues to own returning cleanup without a guest frame. */
void PE_CardCleanupFrame(pe_addr_t record,pe_addr_t caller_sp,const uint32_t incoming[32])
{
    uint32_t override=PE_LoadU32(0x800A185Cu);
    pe_addr_t frame=caller_sp-0x50u;
    PE_StoreU32(frame+0x40u,incoming[18]);PE_StoreU32(frame+0x48u,incoming[31]);
    PE_StoreU32(frame+0x44u,incoming[19]);PE_StoreU32(frame+0x3Cu,incoming[17]);
    PE_StoreU32(frame+0x38u,incoming[16]);
    if(override) {
        operation_boundary("PE_CardCleanupFrame",0x80042228u,0u,0u,0u,0u,0u,0u);return;
    }
    uint32_t handle=PE_LoadU32(record+12u);
    if((int32_t)handle>=0) {
        operation_boundary("PE_CardCleanupFrame",0x80072774u,handle,0u,0u,0u,1u,0u);return;
    }
    uint32_t index=(uint32_t)((int32_t)((record-0x800A0ED4u)*0xC9484E2Bu)>>3);
    if(PE_LoadU8(record+1u)!=9u) {
        operation_boundary("PE_CardCleanupFrame",0x8004D5CCu,index,0u,0u,0u,1u,0u);return;
    }
    uint32_t saved[9];for(unsigned i=0;i<8;i++)saved[i]=incoming[16u+i];
    saved[0]=index;saved[1]=0u;saved[2]=record;saved[3]=0xFFFFFFFFu;saved[8]=0x80041048u;
    uint32_t selected=PE_LoadU8(record+3u);
    pe_addr_t format=PE_LoadU32(0x80092224u);
    uint32_t variant=PE_LoadU8(record+selected*0x44u+69u)+0x30u;
    PE_StoreU32(frame+16u,selected+0x41u);
    unsigned epoch=PE_Port_StopEpoch();
    (void)PE_FormatterFrame(0x8009EE70u,format,record>0x800A0ED4u,variant,frame,saved);
    if(PE_Port_StopEpoch()!=epoch)return;
    /* Reload original callee-saved memory for the next formatter call. */
    for(unsigned i=0;i<8;i++)saved[i]=PE_LoadU32(frame-0x250u+0x228u+i*4u);
    saved[8]=0x80041064u;
    (void)PE_FormatterFrame(frame+24u,0x80010F4Cu,saved[0],0x8009EE70u,frame,saved);
    if(PE_Port_StopEpoch()!=epoch)return;
    operation_boundary("PE_CardCleanupFrame",0x80072734u,frame+24u,1u,0u,0u,3u,0u);
}

/* ── src/func_80041108.c / src/func_80040F80.c translations ─────────── */

#define CARD_BASE 0x800A0ED4u
#define CARD_STRIDE 0x418u
#define CARD_NAME 0x8009EE70u      /* D_8009EE70 filename buffer */
#define CARD_SAVE 0x8009EED0u      /* D_8009EED0 0x2000-byte save image */
#define CARD_HDRS 0x800A1720u      /* D_800A1720[] 0x80-byte headers */
#define E(c,n) ((c)+0x1Cu+(uint32_t)(n)*0x44u)   /* &c->e[n] */

/* Test frontier (see header): nonzero = boundary recorded, caller returns. */
static int card_frontier(const char *caller,pe_addr_t target,uint32_t a0,uint32_t a1,
                         uint32_t a2,uint32_t a3,uint32_t known,uint32_t fifth)
{
    if(!PE_Memcard_BoundaryMode())return 0;
    operation_boundary(caller,target,a0,a1,a2,a3,known,fifth);
    return 1;
}

static void guest_cstr(pe_addr_t a,char *out,size_t cap)
{
    size_t i=0;
    for(;i+1<cap;i++){uint8_t ch=PE_LoadU8(a+(uint32_t)i);if(!ch)break;out[i]=(char)ch;}
    out[i]=0;
}

/* SPRINTF_NAME(up): func_80071A84(&D_8009EE70, D_80092224, up,
 * c->e[c->f3].f29 + '0', c->f3 + 'A').  Returns 0 at a test frontier. */
static int sprintf_name(const char *caller,pe_addr_t c,uint32_t up)
{
    uint32_t f3=PE_LoadU8(c+3u);
    uint32_t args[3]={up,PE_LoadU8(E(c,f3)+0x29u)+(uint32_t)'0',f3+(uint32_t)'A'};
    if(card_frontier(caller,0x80071A84u,CARD_NAME,PE_LoadU32(0x80092224u),args[0],args[1],15u,args[2]))
        return 0;
    (void)PE_Bios_Sprintf(CARD_NAME,PE_LoadU32(0x80092224u),args,3);
    return 1;
}

/* MARK: entry named by the directory name's last character. */
static void card_mark(pe_addr_t c,const PeMcDirEntry *dir)
{
    pe_addr_t e=c+0x1Cu+((uint32_t)(uint8_t)dir->name[19]-(uint32_t)'A')*0x44u;
    PE_StoreU8(e,1u);PE_StoreU8(e+1u,0u);
    PE_StoreU8(e+0x29u,(uint8_t)dir->name[18]!='0');
    PE_StoreU8(c+4u,1u);
}

static int name_matches(const PeMcDirEntry *dir)
{
    /* func_80071A04(dir.name, D_80092224 + 6, 12) == 0 (BIOS strncmp) */
    pe_addr_t ref=PE_LoadU32(0x80092224u)+6u;
    for(uint32_t i=0;i<12u;i++) {
        uint8_t a=(uint8_t)dir->name[i],b=PE_LoadU8(ref+i);
        if(a!=b)return 0;
        if(!a)return 1;
    }
    return 1;
}

/* RETRY macro.  Returns 1 when the caller must return (always). */
static void card_retry(pe_addr_t c)
{
    unsigned epoch=PE_Port_StopEpoch();
    int16_t old=(int16_t)PE_LoadU16(c+0x16u);
    PE_StoreU16(c+0x16u,(uint16_t)(old-1));
    if(old>0)return;
    if(!(PE_LoadU8(c)&1u)) {func_80040F80(c);return;}
    func_80040F80(c);if(PE_Port_StopEpoch()!=epoch)return;
    switch(PE_LoadU8(c+7u)) {
    case 1:func_8004CE28(0x3Du,0x3Fu);break;
    case 2:func_8004CC50(0x3Eu,0u);break;
    }
    if(PE_Port_StopEpoch()!=epoch)return;
    PE_StoreU8(c+7u,0u);PE_StoreU8(c+1u,12u);
}

/* Shared transfer step for states 7/8/9 (read/read-header/write). */
static int card_transfer(pe_addr_t c,int write,int32_t cap)
{
    int16_t len=(int16_t)PE_LoadU16(c+0x14u);
    int fd=(int)PE_LoadU32(c+0xCu);
    pe_addr_t buf=PE_LoadU32(c+0x18u);
    int r;
    if(len>cap)len=(int16_t)cap;
    if(card_frontier("func_80041108",write?0x80072764u:0x80072754u,(uint32_t)fd,buf,
                     (uint32_t)(int32_t)len,0u,7u,0u))return -0x7FFFFFFF;
    r=write?func_80072764(fd,buf,len):func_80072754(fd,buf,len);
    if(r>0) {
        PE_StoreU32(c+0x18u,buf+(uint32_t)r);
        PE_StoreU16(c+0x14u,(uint16_t)(PE_LoadU16(c+0x14u)-(uint32_t)r));
    }
    return r;
}

void func_80041108(uint32_t idx)
{
    unsigned epoch=PE_Port_StopEpoch();
    pe_addr_t c=CARD_BASE+idx*CARD_STRIDE;
    pe_addr_t other=CARD_BASE+(idx==0u?1u:0u)*CARD_STRIDE;
    int fd,r;
#define STOPPED() (PE_Port_StopEpoch()!=epoch)
#define REQUIRE_SEL1() do { if(PE_LoadU8(c)!=1u) goto cleanup; } while(0)
    switch(PE_LoadU8(c+1u)) {
    case 1:
        REQUIRE_SEL1();
        if(PE_LoadU8(c+8u)!=4u)return;
        if(PE_LoadU8(other+8u)!=0u && PE_LoadU8(other+8u)!=4u)return;
        PE_StoreU32(0x800A1838u,1u);
        PE_StoreU8(c+1u,PE_LoadU8(c+0xBu));
        return;
    case 13:
        if(PE_LoadU8(c)==5u) {
            if(PE_LoadU8(c+8u)!=4u)return;
            if(PE_LoadU8(other+8u)!=0u && PE_LoadU8(other+8u)!=4u)return;
            PE_StoreU32(0x800A1838u,1u);
            PE_StoreU8(c+1u,14u);
        } else {
            PE_StoreU32(0x800A1864u,0xFFFFFFFFu);
            PE_StoreU8(c+1u,0u);
        }
        return;
    case 14: {
        /* func_80071A84(buf, D_80010F60 "bu%ld0:", idx); format(buf) */
        char buf[16];uint32_t arg=idx;int ok;
        if(card_frontier("func_80041108",0x80071A84u,0u,0x80010F60u,idx,0u,6u,0u))return;
        (void)PE_Bios_SprintfHost(buf,sizeof buf,0x80010F60u,&arg,1);
        ok=PE_Memcard_Format(buf);
        PE_StoreU32(0x800A1864u,ok?12u:0xFFFFFFFFu);
        PE_StoreU8(c,PE_LoadU8(c)&~4u);
        PE_StoreU32(0x800A1838u,0u);
        PE_StoreU8(c+1u,0u);
        return;
    }
    case 2: {
        PeMcDirEntry dir;char pattern[32];int n,i,ok;
        REQUIRE_SEL1();
        PE_StoreU8(PE_LoadU32(0x80092230u)+2u,(uint8_t)(idx+'0'));
        PE_StoreU8(c+2u,0u);PE_StoreU8(c+3u,0u);PE_StoreU8(c+6u,0u);
        PE_StoreU8(c+4u,0u);PE_StoreU8(c+7u,0u);PE_StoreU8(c+0xAu,0u);
        for(i=0;i<15;i++)PE_StoreU8(E(c,i),2u);
        if(card_frontier("func_80041108",0x800727B4u,PE_LoadU32(0x80092230u),0u,0u,0u,1u,0u))return;
        guest_cstr(PE_LoadU32(0x80092230u),pattern,sizeof pattern);
        if(PE_Memcard_FirstFile(pattern,&dir)) {
            if(name_matches(&dir))card_mark(c,&dir);
            for(;;) {
                PE_StoreU8(c+0xAu,(uint8_t)(PE_LoadU8(c+0xAu)+((int32_t)dir.size>>13)));
                if(!PE_Memcard_NextFile(&dir))break;
                if(name_matches(&dir))card_mark(c,&dir);
            }
            PE_StoreU16(c+0x16u,0u);
        } else PE_StoreU16(c+0x16u,(uint16_t)(PE_LoadU16(c+0x16u)-1u));
        if((int16_t)PE_LoadU16(c+0x16u)>0)return;
        n=PE_LoadU8(c+0xAu);
        for(i=0;i<15;i++) {
            if(PE_LoadU8(E(c,i))==2u)PE_StoreU8(E(c,i)+1u,1u);
            else n--;
        }
        for(i=14;i>=0;i--) {
            if(n==0)break;
            if(PE_LoadU8(E(c,i))==2u) {PE_StoreU8(E(c,i),3u);n--;}
        }
        PE_StoreU8(c+2u,15u);
        if(PE_LoadU32(0x800A186Cu)==0u) {
            PE_StoreU8(c+1u,15u);PE_StoreU32(0x800A1838u,0u);return;
        }
        for(i=0;i<15;i++)if(PE_LoadU8(E(c,i))==1u)break;
        ok=i<15 || (PE_LoadU8(c+0xAu)<15u && func_8004D27C());
        if(ok) {
            PE_StoreU8(c+1u,3u);PE_StoreU16(c+0x16u,10u);
            func_80062CE4();if(STOPPED())return;
            PE_StoreU8(c+5u,(uint8_t)func_8004D4C4((int)idx,PE_LoadU8(c+2u)));
            return;
        }
        func_80062CE4();if(STOPPED())return;
        func_8004D298((int)idx);if(STOPPED())return;
        PE_StoreU8(c+1u,12u);PE_StoreU32(0x800A1838u,0u);
        return;
    }
    case 3: {
        uint32_t cur,f6,sel;
        REQUIRE_SEL1();
        cur=PE_LoadU8(c+6u);
        if(PE_LoadU8(c+6u)<PE_LoadU8(c+2u)*2u) {
            int k;uint32_t n=PE_LoadU8(c+2u)*2u;
            for(;;) {
                k=(int)PE_LoadU8(c+5u)+(int)(((cur&1u)*2u-1u)*((cur+1u)>>1));
                if((unsigned)k<15u && PE_LoadU8(E(c,k)+1u)==0u)break;
                cur=(cur+1u)&255u;PE_StoreU8(c+6u,(uint8_t)cur);
                if(cur>=n)break;
            }
        }
        f6=PE_LoadU8(c+6u);
        sel=f6<PE_LoadU8(c+2u)*2u?PE_LoadU8(c+5u)+((f6&1u)*2u-1u)*((f6+1u)>>1):0xFFu;
        PE_StoreU8(c+3u,(uint8_t)sel);
        if(PE_LoadU8(c+3u)!=0xFFu) {
            if(!sprintf_name("func_80041108",c,CARD_BASE<c))return;
            if(card_frontier("func_80041108",0x80072734u,CARD_NAME,1u,0u,0u,3u,0u))return;
            fd=func_80072734(CARD_NAME,1u);
            PE_StoreU32(c+0xCu,(uint32_t)fd);
            if(fd>=0) {
                PE_StoreU8(c+1u,8u);
                PE_StoreU32(c+0x18u,CARD_HDRS+idx*0x80u);
                PE_StoreU16(c+0x14u,0x80u);PE_StoreU16(c+0x16u,30u);
                if(PE_LoadU8(E(c,PE_LoadU8(c+3u)))==1u) {
                    if(card_frontier("func_80041108",0x80072744u,(uint32_t)fd,0x100u,0u,0u,7u,0u))return;
                    (void)func_80072744((int)PE_LoadU32(c+0xCu),0x100,0);
                }
                return;
            }
            card_retry(c);return;
        }
        PE_StoreU8(c+1u,12u);PE_StoreU32(0x800A1838u,0u);
        return;
    }
    case 4:
        REQUIRE_SEL1();
        PE_StoreU8(E(c,PE_LoadU8(c+3u))+0x29u,(uint8_t)PE_LoadU32(0x800A1704u));
        if(!sprintf_name("func_80041108",c,CARD_BASE<c))return;
        if(card_frontier("func_80041108",0x80072734u,CARD_NAME,0x10200u,0u,0u,3u,0u))return;
        fd=func_80072734(CARD_NAME,0x10200u);
        PE_StoreU32(c+0xCu,(uint32_t)fd);
        if(fd>=0) {
            if(card_frontier("func_80041108",0x80072774u,(uint32_t)fd,0u,0u,0u,1u,0u))return;
            (void)func_80072774(fd);
            PE_StoreU32(c+0xCu,0xFFFFFFFFu);
            PE_StoreU8(c+1u,6u);PE_StoreU16(c+0x16u,10u);
            return;
        }
        card_retry(c);return;
    case 5:
        REQUIRE_SEL1();
        if(!sprintf_name("func_80041108",c,CARD_BASE<c))return;
        if(card_frontier("func_80041108",0x80072734u,CARD_NAME,1u,0u,0u,3u,0u))return;
        fd=func_80072734(CARD_NAME,1u);
        PE_StoreU32(c+0xCu,(uint32_t)fd);
        if(fd>=0) {PE_StoreU16(c+0x16u,30u);PE_StoreU8(c+1u,7u);return;}
        card_retry(c);return;
    case 6:
        REQUIRE_SEL1();
        if(!sprintf_name("func_80041108",c,CARD_BASE<c))return;
        if(card_frontier("func_80041108",0x80072734u,CARD_NAME,2u,0u,0u,3u,0u))return;
        fd=func_80072734(CARD_NAME,2u);
        PE_StoreU32(c+0xCu,(uint32_t)fd);
        if(fd>=0) {
            PE_StoreU8(c+1u,9u);PE_StoreU32(c+0x18u,CARD_SAVE);PE_StoreU16(c+0x16u,30u);
            return;
        }
        card_retry(c);return;
    case 8:
        REQUIRE_SEL1();
        r=card_transfer(c,0,0x80);
        if(r==-0x7FFFFFFF)return;
        if(r>0) {
            if((int16_t)PE_LoadU16(c+0x14u)>0)return;
            PE_StoreU8(c+1u,10u);return;
        }
        card_retry(c);return;
    case 7:
        REQUIRE_SEL1();
        r=card_transfer(c,0,0x400);
        if(r==-0x7FFFFFFF)return;
        if(r>0) {
            if((int16_t)PE_LoadU16(c+0x14u)>0)return;
            if(card_frontier("func_80041108",0x80072774u,PE_LoadU32(c+0xCu),0u,0u,0u,1u,0u))return;
            (void)func_80072774((int)PE_LoadU32(c+0xCu));
            PE_StoreU32(c+0xCu,0xFFFFFFFFu);
            PE_StoreU8(c+1u,12u);PE_StoreU8(c+7u,0u);
            PE_StoreU32(0x800A1854u,0u);PE_StoreU32(0x800A1838u,0u);
            func_80042264();
            return;
        }
        card_retry(c);return;
    case 9:
        REQUIRE_SEL1();
        r=card_transfer(c,1,0x400);
        if(r==-0x7FFFFFFF)return;
        if(r>0) {
            if((int16_t)PE_LoadU16(c+0x14u)>0)return;
            if(card_frontier("func_80041108",0x80072774u,PE_LoadU32(c+0xCu),0u,0u,0u,1u,0u))return;
            (void)func_80072774((int)PE_LoadU32(c+0xCu));
            PE_StoreU32(c+0xCu,0xFFFFFFFFu);
            PE_StoreU8(c+1u,3u);PE_StoreU8(c+7u,0u);
            PE_StoreU32(0x800A1854u,0u);
            func_8004D9D8();if(STOPPED())return;
            func_8004CC50(0x53u,0u);
            return;
        }
        card_retry(c);return;
    case 10: {
        pe_addr_t e,h;
        REQUIRE_SEL1();
        if(card_frontier("func_80041108",0x80072774u,PE_LoadU32(c+0xCu),0u,0u,0u,1u,0u))return;
        (void)func_80072774((int)PE_LoadU32(c+0xCu));
        PE_StoreU32(c+0xCu,0xFFFFFFFFu);
        e=E(c,PE_LoadU8(c+3u));
        h=CARD_HDRS+idx*0x80u;
        if(PE_LoadU8(e)==1u) {
            PE_StoreU16(e+0x24u,PE_LoadU16(h+0x28u));
            PE_StoreU16(e+0x26u,PE_LoadU16(h+0x26u));
            PE_StoreU8(e+0x28u,PE_LoadU8(h+0x2Au));
            PE_StoreU32(e+0x0Cu,PE_LoadU32(h+0x08u));
            PE_StoreU32(e+0x10u,PE_LoadU32(h+0x0Cu));
            PE_StoreU32(e+0x20u,PE_LoadU32(h+0x64u));
            PE_StoreU8(e+0x2Au,PE_LoadU8(h+0x2Bu));
            PE_StoreU16(e+0x2Cu,PE_LoadU16(h+0x5Cu));
            PE_StoreU16(e+0x2Eu,PE_LoadU16(h+0x5Eu));
            {uint32_t w0=PE_LoadU32(h),w1=PE_LoadU32(h+4u);
             PE_StoreU32(e+4u,w0);PE_StoreU32(e+8u,w1);}
            {uint32_t w0=PE_LoadU32(h+0x10u),w1=PE_LoadU32(h+0x14u),w2=PE_LoadU32(h+0x18u);
             PE_StoreU32(e+0x14u,w0);PE_StoreU32(e+0x18u,w1);PE_StoreU32(e+0x1Cu,w2);}
        }
        PE_StoreU8(e+1u,1u);
        {
            uint32_t f6=(PE_LoadU8(c+6u)+1u)&255u;
            PE_StoreU8(c+6u,(uint8_t)f6);
            PE_StoreU8(c+1u,f6<PE_LoadU8(c+2u)*2u?3u:12u);
        }
        if(PE_LoadU8(c+1u)==12u)PE_StoreU32(0x800A1838u,0u);
        return;
    }
    case 11: {
        uint32_t up;
        REQUIRE_SEL1();
        up=CARD_BASE<c;
        if(!sprintf_name("func_80041108",c,up))return;
        if(card_frontier("func_80041108",0x80072734u,CARD_NAME,1u,0u,0u,3u,0u))return;
        fd=func_80072734(CARD_NAME,1u);
        PE_StoreU32(c+0xCu,(uint32_t)fd);
        if(fd>=0) {
            (void)func_80072774(fd);
            PE_StoreU32(c+0xCu,0xFFFFFFFFu);
            if(!sprintf_name("func_80041108",c,up))return;
            if(func_800727A4(CARD_NAME)) {
                PE_StoreU8(c+1u,4u);PE_StoreU16(c+0x16u,10u);return;
            }
            card_retry(c);return;
        }
        card_retry(c);return;
    }
    case 0:case 15:
        return;
    case 12:
        if(PE_LoadU8(c)==1u)return;
    cleanup:
        func_80040F80(c);
        return;
    default:
        return;
    }
#undef STOPPED
#undef REQUIRE_SEL1
}

void func_80040F80(pe_addr_t c)
{
    unsigned epoch=PE_Port_StopEpoch();
    int i,fd=-1;
    if(PE_LoadU32(0x800A185Cu)!=0u) {func_80042228();return;}
    if((int32_t)PE_LoadU32(c+0xCu)>=0) {
        if(card_frontier("func_80040F80",0x80072774u,PE_LoadU32(c+0xCu),0u,0u,0u,1u,0u))return;
        (void)func_80072774((int)PE_LoadU32(c+0xCu));
        PE_StoreU32(c+0xCu,0xFFFFFFFFu);
    }
    if(PE_LoadU8(c+1u)==9u) {
        /* Retail formats the file name, then "bu%ld0:%s" over it (a
         * doubled device prefix), and retries open(buf,1) up to ten times
         * before closing and deleting what it opened. */
        uint32_t slot=(uint32_t)((int32_t)((c-CARD_BASE)*0xC9484E2Bu)>>3);
        char buf[0x20];uint32_t args[2];
        if(!sprintf_name("func_80040F80",c,CARD_BASE<c))return;
        args[0]=slot;args[1]=CARD_NAME;
        (void)PE_Bios_SprintfHost(buf,sizeof buf,0x80010F4Cu,args,2);
        for(i=0;i<10;i++) {
            fd=PE_Memcard_Open(buf,1u);
            if(fd!=-1)break;
        }
        if(i<10) {(void)func_80072774(fd);(void)PE_Memcard_Delete(buf);}
    }
    func_8004D5CC((uint32_t)((int32_t)((c-CARD_BASE)*0xC9484E2Bu)>>3));
    if(PE_Port_StopEpoch()!=epoch)return;
    PE_StoreU8(c+1u,0u);PE_StoreU8(c+4u,0u);
    PE_StoreU32(0x800A1854u,0u);PE_StoreU32(0x800A1838u,0u);
    func_80062CE4();
}
