/* Test-only finite GPU readback model; VSync and GPU events are scripted. */
#include <stdint.h>
typedef uint32_t (*load_t)(uint32_t,uint32_t);
typedef void (*store_t)(uint32_t,uint32_t,uint32_t);
typedef uint32_t (*hardware_t)(uint32_t,uint32_t,uint32_t);
int draft(uint32_t rect,uint32_t destination,load_t ld,store_t st,hardware_t hw){
 st(0x8009588cu,0,4);st(0x80095888u,257,4);
 int32_t width=(int16_t)ld(rect+4,2),limitw=(int16_t)ld(0x80095750u,2);
 int32_t height=(int16_t)ld(rect+6,2),limith=(int16_t)ld(0x80095752u,2);
 if(width<0)width=0;else if(width>limitw)width=limitw;
 if(height<0)height=0;else if(height>limith)height=limith;
 st(rect+4,(uint16_t)width,2);st(rect+6,(uint16_t)height,2);
 int32_t pixels=(int16_t)width*(int16_t)height;int32_t words=(pixels+1)/2;
 if(words<=0)return -1;
 (void)hw(0,0x1f801814u,0);hw(1,0x1f801814u,0x04000000u);hw(1,0x1f801810u,0x01000000u);hw(1,0x1f801810u,0xc0000000u);
 hw(1,0x1f801810u,ld(rect,4));hw(1,0x1f801810u,ld(rect+4,4));(void)hw(0,0x1f801814u,0);
 uint32_t count=(uint32_t)words&15u,blocks=(uint32_t)words>>4;
 for(uint32_t i=0;i<count;i++)st(destination+4u*i,hw(0,0x1f801810u,0),4);
 if(blocks){hw(1,0x1f801814u,0x04000003u);hw(1,0x1f8010a0u,destination+4u*count);hw(1,0x1f8010a4u,(blocks<<16)|16u);hw(1,0x1f8010a8u,0x01000200u);}
 return 0;
}
