/* Independent test-only draft from original execution observations.
 * Guest ABI scratch slots deliberately retain observed ambient stack bytes.
 * Not production code; no parity claim until checked. */
#include <stdint.h>
typedef uint32_t (*load_fn)(uint32_t,uint32_t);
typedef void (*store_fn)(uint32_t,uint32_t,uint32_t);
typedef uint32_t (*call_fn)(uint32_t,uint32_t,uint32_t);
#define MODE_SLOT 0x801FEF99u
#define LOC_SLOT 0x801FEFA9u
static void record(uint32_t serial,uint32_t command,uint32_t source,uint32_t extra,uint32_t callback,load_fn ld,store_fn st,call_fn call)
{
    uint32_t slot=call(0x8007E6B0u,0,0);
    st(slot,4,serial);st(slot+4,1,command);
    if(source)call(0x80080950u,slot+5,source);
    st(slot+12,4,source?slot+5:0);
    st(slot+20,4,callback);st(slot+16,4,extra);
    st(0x800A3608u,4,ld(0x800A3608u,4)+1u);
}
uint32_t draft(uint32_t mode,uint32_t location,uint32_t command,uint32_t extra,load_fn ld,store_fn st,call_fn call)
{
    uint32_t callback=ld(0x801FF010u,4);
    mode&=255u;command&=255u;
    st(MODE_SLOT,1,mode);
    if((int32_t)call(0x80080C48u,location,0)<0)return 0;
    st(LOC_SLOT,4,ld(location,4));st(LOC_SLOT,4,ld(location,4));
    if(command<3u||command>27u)return 0;
    (void)ld(0x80011C9Cu+4u*(command-3u),4);
    if(command!=3u&&command!=6u&&command!=21u&&command!=22u&&command!=27u)return 0;
    if((int32_t)(ld(0x800A3608u,4)+4u)>8)return 0;
    uint32_t serial=ld(0x8009B53Cu,4)+1u;st(0x8009B53Cu,4,serial);
    if(!serial){serial=1;st(0x8009B53Cu,4,serial);}
    record(serial,9,0,0,callback,ld,st,call);
    record(serial,14,MODE_SLOT,0,callback,ld,st,call);
    record(serial,2,LOC_SLOT,0,callback,ld,st,call);
    record(serial,command,0,extra,callback,ld,st,call);
    if(call(0x8007FBF0u,0,0)==1u){
        uint32_t index=ld(0x800A3604u,4);
        if(ld(0x800A3540u+24u*index,4)==serial)call(0x8007E8F4u,0,0);
    }
    return serial;
}
