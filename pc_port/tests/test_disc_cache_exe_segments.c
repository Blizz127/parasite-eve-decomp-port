/* Synthetic EXE bytes and cache state; no real EXE, file or disc. */
#define main disc_cache_seek_fixture_unused_main
#include "test_disc_cache_peimg_seek.c"
#undef main
static uint8_t synthetic_exe[PE_EXE_HDR+32];
static int read_equals(uint32_t address,uint32_t expected) {
    uint8_t out[4]={0};
    return PE_DiscCache_ExeRead(address,out,sizeof out)==0 &&
           out[0]==expected && out[1]==expected+1 && out[2]==expected+2 && out[3]==expected+3;
}
static int rejected(uint32_t address,uint32_t length) {
    uint8_t out[8];memset(out,0xa5,sizeof out);
    if(PE_DiscCache_ExeRead(address,out,length)!=-1) return 0;
    for(unsigned i=0;i<sizeof out;i++) if(out[i]!=0xa5) return 0;
    return 1;
}
int main(int argc,char **argv) {
    int mode=argc>1?atoi(argv[1]):-1;
    for(unsigned i=0;i<32;i++) synthetic_exe[PE_EXE_HDR+i]=(uint8_t)i;
    s_tried=1;s_exe=synthetic_exe;s_exe_size=sizeof synthetic_exe;s_taddr=0x80010000u;s_tsize=32;
    if(!read_equals(0x00010004u,4) || !read_equals(0x80010004u,4) || !read_equals(0xa0010004u,4)) return 1;
    if(mode==-1 || mode==0) {
        const uint32_t invalid[]={0x20010004u,0x40010004u,0x60010004u,0xc0010004u,0xe0010004u,0x80200000u,0xa0200000u,0xfffffffcu};
        for(unsigned i=0;i<sizeof invalid/sizeof invalid[0];i++) if(!rejected(invalid[i],4)) { fprintf(stderr,"invalid segment %u accepted\n",i);return 2; }
    }
    if(mode==-1 || mode==1) {
        s_taddr=0x801ffff0u;s_tsize=16;
        if(!read_equals(0x801ffffcu,12) || !rejected(0x801ffffeu,4) ||
           !rejected(0x801fffffu,UINT32_MAX) || !rejected(0x80200000u,0)) {
            fputs("cross-RAM-end range accepted\n",stderr);return 3;
        }
        s_tsize=32;
        if(!rejected(0x801ffff0u,1)) { fputs("cross-RAM-end EXE accepted\n",stderr);return 6; }
    }
    if(mode==-1 || mode==2) {
        s_taddr=0xc0010000u;
        if(!rejected(0x80010004u,4)) { fputs("invalid EXE base segment accepted\n",stderr);return 4; }
    }
    if(mode==-1) {
        s_taddr=0x00010000u;s_tsize=32;
        if(!read_equals(0xa0010004u,4) || !rejected(0x80010020u,1)) return 5;
        if(PE_DiscCache_ExeRead(0x80010020u,NULL,0)!=0 ||
           PE_DiscCache_ExeRead(0x80010000u,NULL,4)!=-1 ||
           PE_DiscCache_ExeWord(0x80010004u,NULL)!=-1) return 7;
        uint32_t word=0xabcdef01u;
        if(PE_DiscCache_ExeWord(0xa0010004u,&word)!=0 || word!=0x07060504u) return 8;
        if(PE_DiscCache_ExeWord(0xc0010004u,&word)!=-1 || word!=0x07060504u) return 9;
    }
    puts("synthetic EXE segment and RAM-range controls pass");return 0;
}
