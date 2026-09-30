#include "pe_timer1.h"
static void test_DAY1_timer1(void)
{
    TEST("DAY1_timer1");
    ResetTestState();PE_Callback_Init();
    PeIrqGeneration generation=PE_IRQ_Generation();
    PE_Timer1_HBlankEdge(generation);
    ASSERT(PE_Timer1_ReadCounter()==0,"uninitialized timer must not count");
    func_80073C94();
    ASSERT(PE_Timer1_ReadCounter()==0 && PE_Timer1_ReadMode()==0x507,"ResetCallback must initialize timer1 mode107");
    for(unsigned i=0;i<300;i++)PE_Timer1_HBlankEdge(generation);
    ASSERT(PE_Timer1_ReadCounter()==0,"sync3 must wait for VBlank");
    ASSERT(!PE_Timer1_VBlankEdge(generation-1),"stale VBlank must not release timer");
    PE_Timer1_HBlankEdge(generation);
    ASSERT(PE_Timer1_ReadCounter()==0,"stale VBlank changed timer gating");
    PE_Timer1_VBlankEdge(generation);
    for(unsigned i=0;i<65535;i++)PE_Timer1_HBlankEdge(generation);
    ASSERT(PE_Timer1_ReadCounter()==65535,"timer must count HBlank edges after sync");
    ASSERT(PE_Timer1_ReadMode()==0x1507 && PE_Timer1_ReadMode()==0x507,"mode read must clear reached-FFFF flag");
    ASSERT(!PE_Timer1_HBlankEdge(generation-1) && PE_Timer1_ReadCounter()==65535,"stale HBlank must be inert");
    PE_Timer1_HBlankEdge(generation);
    ASSERT(PE_Timer1_ReadCounter()==0 && PE_Timer1_ReadMode()==0xD07,"counter wrap must reach reset target zero");
    PE_Timer1_VBlankEdge(generation);
    PE_Timer1_HBlankEdge(generation);
    PE_Timer1_VBlankEdge(generation);
    ASSERT(PE_Timer1_ReadCounter()==1,"later VBlank must not reset sync3 counter");
    func_80073C94();
    ASSERT(PE_Timer1_ReadCounter()==1,"guarded ResetCallback must not reprogram timer");
    ASSERT(PE_LoadU32(0x800956ACu)==0 && PE_IRQ_ReadStatus()==0,"timer edges must not fabricate CPU dispatch");
    PE_Timer1_InitializeVBlankCounter();
    PE_Timer1_HBlankEdge(generation);
    ASSERT(PE_Timer1_ReadCounter()==0 && PE_Timer1_ReadMode()==0x507,"mode write must reset counter and sync latch");
    PE_Sdk_ResetState();
    ASSERT(!PE_Timer1_VBlankEdge(generation) && PE_Timer1_ReadCounter()==0,"SDK reset must invalidate old edges");
    PASS();
}
