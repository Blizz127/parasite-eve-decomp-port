/* Isolated response-only mailbox controls; no disc is attached. */
#define main cd_irq_fixture_unused_main
#include "test_cd_irq_shellopen.c.in"
#undef main

static void check(int condition, unsigned which)
{
    if (!condition) { fprintf(stderr,"response case %u failed\n",which); exit(1); }
}
int main(void)
{
    static const uint8_t packet[2]={0x29,0x73};
    static const uint8_t replacement[17]={0};
    PE_RamInit();
    for (unsigned i=0;i<6;i++) {
        PE_CdReg_Reset();
        if (i<4) {
            int empty_result;
            switch(i) {
            case 0: empty_result=PE_CdReg_PushResponse(0,replacement,1); break;
            case 1: empty_result=PE_CdReg_PushResponse(8,replacement,1); break;
            case 2: empty_result=PE_CdReg_PushResponse(2,replacement,17); break;
            default: empty_result=PE_CdReg_PushResponse(2,NULL,1); break;
            }
            check(!empty_result,i);
        }
        check(PE_CdReg_PushResponse(5,packet,2)==1,i);
        PE_CdReg_WriteU8(PE_CDREG_BASE,1);
        check((PE_CdReg_ReadU8(PE_CDREG_BASE+3)&7u)==5u,i);
        int accepted;
        switch(i) {
        case 0: accepted=PE_CdReg_PushResponse(0,replacement,1); break;
        case 1: accepted=PE_CdReg_PushResponse(8,replacement,1); break;
        case 2: accepted=PE_CdReg_PushResponse(2,replacement,17); break;
        case 3: accepted=PE_CdReg_PushResponse(2,NULL,1); break;
        case 4: accepted=PE_CdReg_PushResponse(2,replacement,1); break;
        default: accepted=PE_CdReg_PushResponse(2,NULL,0); break;
        }
        check(!accepted,i);
        check((PE_CdReg_ReadU8(PE_CDREG_BASE+3)&7u)==5u,i);
        PE_CdReg_WriteU8(PE_CDREG_BASE+3,7);
        check((PE_CdReg_ReadU8(PE_CDREG_BASE+3)&7u)==0,i);
        check(!PE_CdReg_PushResponse(2,replacement,1),i);
        check((PE_CdReg_ReadU8(PE_CDREG_BASE)&0x20u)!=0,i);
        check(PE_CdReg_ReadU8(PE_CDREG_BASE+1)==packet[0],i);
        check(PE_CdReg_ReadU8(PE_CDREG_BASE+1)==packet[1],i);
        check((PE_CdReg_ReadU8(PE_CDREG_BASE)&0x20u)==0,i);
        PE_CdReg_WriteU8(PE_CDREG_BASE+3,7);
        check((PE_CdReg_ReadU8(PE_CDREG_BASE+3)&7u)==0,i);
        check(PE_CdReg_PushResponse(2,replacement,1)==1,i);
    }
    PE_RamDestroy(); puts("6 synthetic CD mailbox rejection controls pass"); return 0;
}
