/* Dressing-room mirror overlay, original Disc 1 sectors 12868–12869.
 * 8018EFF4..8018FB44: lifecycle, reflected pose and reversed-face packets.
 * Native translation; the overlay remains the authority for its data. */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "pe_sdk.h"

int PE_MirrorOverlay(void)
{
    return PE_LoadU32(0x8018EFFCu)==0x00001021u &&
        PE_LoadU32(0x8018F000u)==0xAC80000Cu &&
        PE_LoadU32(0x8018F1C4u)==0x24020005u &&
        PE_LoadU32(0x8018FB3Cu)==0x03E00008u;
}
int PE_MirrorInit(pe_addr_t slot)
{
    PE_StoreU32(slot+12u,0u);
    PE_StoreU16(slot+18u,0u);PE_StoreU16(slot+16u,0u);
    PE_StoreU16(slot+22u,0u);PE_StoreU16(slot+20u,0u);
    PE_StoreU8(slot+24u,0u);PE_StoreU8(slot+25u,0u);
    return 0;
}
int PE_MirrorCommand(pe_addr_t slot,uint32_t mode,uint32_t command,uint32_t x,uint32_t z)
{
    switch(command) {
    case 0:
        if (!mode) {
            pe_addr_t a=PE_LoadU32(0x8009D20Cu);
            PE_StoreU32(slot+12u,a);
            while(a) {
                if (PE_LoadU8(a+12u)==x && PE_LoadU8(a+13u)==z &&
                    !(PE_LoadU32(a+0x98u)&0x10u)) break;
                a=PE_LoadU32(a+4u);PE_StoreU32(slot+12u,a);
            }
        }
        break;
    case 1:PE_StoreU16(slot+16u,(uint16_t)x);PE_StoreU16(slot+18u,(uint16_t)z);break;
    case 2:PE_StoreU16(slot+20u,(uint16_t)x);PE_StoreU16(slot+22u,(uint16_t)z);break;
    case 3:PE_StoreU8(slot+25u,(uint8_t)x);break;
    default:return -6;
    }
    return 0;
}
void PE_MirrorBind(pe_addr_t dest,pe_addr_t actor,int32_t x0,int32_t z0,int32_t x1,int32_t z1)
{
    PE_StoreU16(dest+0x28u,5u);PE_StoreU32(dest+0x60u,actor);
    PE_StoreU16(dest+0xBCu,(uint16_t)x0);PE_StoreU16(dest+0xBEu,(uint16_t)z0);
    PE_StoreU16(dest+0x9Cu,(uint16_t)(PE_LoadU16(dest+0x9Cu)|0x1000u));
    PE_StoreU16(dest+0xC0u,(uint16_t)x1);PE_StoreU16(dest+0xC2u,(uint16_t)z1);
}
static int32_t mirror_mul(int32_t a,int32_t b) {return (int32_t)((int64_t)a*b);}
static int32_t mirror_div(int32_t a,int32_t b)
{
    if (!b) return a<0?1:-1;
    if (a==INT32_MIN && b==-1) return INT32_MIN;
    return a/b;
}
void PE_MirrorPose(pe_addr_t dest)
{
    pe_addr_t actor=PE_LoadU32(dest+0x60u),source=actor+0x1B4u;
    int32_t dx=(int16_t)(PE_LoadU16(dest+0xC0u)-PE_LoadU16(dest+0xBCu));
    int32_t dz=(int16_t)(PE_LoadU16(dest+0xC2u)-PE_LoadU16(dest+0xBEu));
    int32_t slope,distance,angle,rx,rz,turn;
    uint32_t x=PE_LoadU32(actor+0x1FCu),z=PE_LoadU32(actor+0x204u),v;
    unsigned col,row;
    if (!dx) dx=1;
    slope=dz/dx;
    v=(uint32_t)mirror_mul(slope,(int32_t)x)-z+
      (uint32_t)(int32_t)(int16_t)PE_LoadU16(dest+0xC2u)-
      (uint32_t)mirror_mul(slope,(int16_t)PE_LoadU16(dest+0xC0u));
    if ((int32_t)v<0) v=0u-v;
    distance=mirror_div((int32_t)v,(int32_t)func_80078004((uint32_t)mirror_mul(slope,slope)+1u));
    angle=func_80079FB4(dz,dx);
    rx=mirror_mul(func_80077CF4(angle),distance)>>12;
    rz=mirror_mul(func_80077DC4(angle),distance)>>12;
    x-=2u*(uint32_t)rx;z+=2u*(uint32_t)rz;
    PE_StoreU32(dest+0x48u,x);PE_StoreU32(dest+0x4Cu,PE_LoadU32(actor+0x200u));
    PE_StoreU32(dest+0x50u,z);
    if ((dz<0?-dz:dz)<(dx<0?-dx:dx))
        turn=(int32_t)PE_LoadU32(actor+0x204u)<(int32_t)z?0x800:0;
    else turn=(int32_t)x<(int32_t)PE_LoadU32(actor+0x1FCu)?0xC00:0x400;
    PE_StoreU16(dest+0x2Cu,(uint16_t)(-(int16_t)PE_LoadU16(actor+0x1E0u)));
    PE_StoreU16(dest+0x2Eu,(uint16_t)(-(angle+turn+(int16_t)PE_LoadU16(source+0x2Eu))));
    PE_StoreU16(dest+0x30u,(uint16_t)(-(int16_t)PE_LoadU16(source+0x30u)));
    func_800794C4(dest+0x2Cu,dest+0x34u);
    PE_GTE_LoadRT33(dest+0x34u);
    for(col=0;col<3;col++) {
        PE_GTE_SetIR((int16_t)PE_LoadU16(0x8018FB60u+col*2u),
            (int16_t)PE_LoadU16(0x8018FB66u+col*2u),(int16_t)PE_LoadU16(0x8018FB6Cu+col*2u));
        PE_GTE_MVMVA(0x49E012u);
        for(row=0;row<3;row++) PE_StoreU16(dest+0x34u+row*6u+col*2u,(uint16_t)g_pe_gte.ir[row]);
    }
    func_80039B74(dest,PE_LoadU32(actor+0x1B0u),(int16_t)PE_LoadU16(actor+0x16u),1);
}
void PE_MirrorPackets(pe_addr_t dest)
{
    static const unsigned size[4]={52,40,36,28};
    pe_addr_t object=PE_LoadU32(dest),faces=PE_LoadU32(dest+16u),packets=PE_LoadU32(dest+0x54u);
    unsigned bank=PE_LoadU32(0x8009CDDCu),kind,i,j;
    pe_addr_t ot=PE_LoadU32(0x800B0E38u+bank*4u);
    for(kind=0;kind<4;kind++) {
        unsigned vertices=(kind&1u)?3u:4u,spacing=kind<2?12u:8u;
        for(i=0;i<PE_LoadU16(object+8u+kind*2u);i++,faces+=12u,packets+=size[kind]*2u) {
            uint32_t xy[4],sum=0;int32_t clip,depth;
            pe_addr_t packet=packets+bank*size[kind];
            for(j=0;j<vertices;j++) {
                unsigned index=PE_LoadU16(faces+4u+j*2u);
                xy[j]=PE_LoadU32(0x800B1644u+index*4u);
                sum+=PE_LoadU32(0x800A636Cu+index*4u);
            }
            clip=(int32_t)((int64_t)(int16_t)xy[0]*((int16_t)(xy[1]>>16)-(int16_t)(xy[2]>>16))+
                (int64_t)(int16_t)xy[1]*((int16_t)(xy[2]>>16)-(int16_t)(xy[0]>>16))+
                (int64_t)(int16_t)xy[2]*((int16_t)(xy[0]>>16)-(int16_t)(xy[1]>>16)));
            PE_StoreU32(0x1F800000u,(uint32_t)clip);
            if (clip>=0) {PE_StoreU32(packet,PE_LoadU32(packet)&0xFF000000u);continue;}
            depth=vertices==4?((int32_t)sum>>4):(((int32_t)sum/3)>>2);
            if(depth>=4096) continue;
            pe_addr_t link=ot+(uint32_t)depth*4u;
            PE_StoreU32(packet,(PE_LoadU32(packet)&0xFF000000u)|(PE_LoadU32(link)&0xFFFFFFu));
            PE_StoreU32(link,(PE_LoadU32(link)&0xFF000000u)|(packet&0xFFFFFFu));
            for(j=0;j<vertices;j++) PE_StoreU32(packet+8u+j*spacing,xy[j]);
        }
    }
}
int PE_MirrorDraw(pe_addr_t slot)
{
    pe_addr_t dest=PE_LoadU32(slot+8u)+0x1B4u;
    if (!PE_LoadU8(slot+24u)) {
        PE_MirrorBind(dest,PE_LoadU32(slot+12u),(int16_t)PE_LoadU16(slot+16u),
            (int16_t)PE_LoadU16(slot+18u),(int16_t)PE_LoadU16(slot+20u),(int16_t)PE_LoadU16(slot+22u));
        PE_StoreU8(slot+24u,1u);
    }
    if (PE_LoadU8(slot+25u)) {
        PE_MirrorPose(dest);func_8003A088_mode0_walk_cut(dest);
        func_8003AC90(dest,PE_LoadU32(0x800BCFA4u));PE_MirrorPackets(dest);
    }
    return 0;
}
