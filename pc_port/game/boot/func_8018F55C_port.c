/* M0000I 8018F55C..8018F92C: cyclic position/heading interpolation.
 * The time integer is a logical shift, even for bit31-set inputs. */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game_port.h"

int func_8018F55C(uint32_t time, uint32_t id, pe_addr_t package,
                  pe_addr_t position, pe_addr_t rotation)
{
    pe_addr_t record=package+PE_LoadU32(package+id*4u);
    int count=(int16_t)PE_LoadU16(record+6u),period=count-2;
    int index=(int)(time>>8),fraction=(int)(time&255u);
    int p[3][3],d[2][3];
    if(period<index)PE_StoreU8(0x8019BFCCu,1);
    if(!period){
        /* Original BREAK 7; retain an explicit boundary instead of host UB. */
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
        return 0;
    }
    index%=period;
    if(index<0)index=index-2+count;
    int indices[]={index,(index+1)%period,(index+2)%period};
    for(unsigned i=0;i<3;i++)for(unsigned j=0;j<3;j++)
        p[i][j]=(int16_t)PE_LoadU16(record+8u+(uint32_t)indices[i]*8u+j*2u);
    for(unsigned i=0;i<2;i++)for(unsigned j=0;j<3;j++)d[i][j]=p[i+1][j]-p[i][j];
    int first=-func_80079FB4(d[0][2],d[0][0]);
    int second=-func_80079FB4(d[1][2],d[1][0]);
    int adjusted=first;
    if((int16_t)first-(int16_t)second>2048)second+=4096;
    second=(int16_t)second;
    if(second-(int16_t)first>2048)adjusted=first+4096;
    int turn=((int16_t)adjusted-second)>>3;
    PE_StoreU16(rotation+4u,(uint16_t)turn);
    PE_StoreU16(rotation,0);
    PE_StoreU16(rotation+2u,(uint16_t)(adjusted+((second-(int16_t)adjusted)*fraction>>8)));
    if(turn>128)PE_StoreU16(rotation+4u,128);
    if((int16_t)PE_LoadU16(rotation+4u)<-128)PE_StoreU16(rotation+4u,(uint16_t)-128);
    if((uint16_t)(PE_LoadU16(rotation+4u)+3u)<7u)PE_StoreU16(rotation+4u,0);
    for(unsigned j=0;j<3;j++)
        PE_StoreU32(position+j*4u,(uint32_t)((p[0][j]*256+d[0][j]*fraction)>>8));
    return count;
}
