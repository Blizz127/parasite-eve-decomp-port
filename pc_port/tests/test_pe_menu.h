#include "retail_pe_menu_cases.h"

static void test_INV19_retail_pe_menu(void)
{
    unsigned k,i,failures=0;
    TEST_RETAIL_DISC1("INV19_retail_pe_menu"); TEST_RETAIL_FIXUPS(RETAILFIX_pe_menu);
    for (k=0;k<sizeof(INV19_pe_menu_cases)/sizeof(INV19_pe_menu_cases[0]);k++) {
        const uint32_t *args=INV19_pe_menu_cases[k].args;
        uint32_t a=args[0],result=0u;
        uint64_t hash;
        ResetTestState();func_80071A64(1u);
        for (i=0;i<sizeof(INV19_pe_menu_common)/sizeof(INV19_pe_menu_common[0]);i++)
            PE_StoreU32(0x80000000u+INV19_pe_menu_common[i][0],INV19_pe_menu_common[i][1]);
        for (i=INV19_pe_menu_cases[k].first;i<INV19_pe_menu_cases[k].end;i++)
            PE_StoreU32(0x80000000u+INV19_pe_menu_patches[i][0],INV19_pe_menu_patches[i][1]);
        D_8009D1A0=PE_LoadU32(0x8009D1A0u);D_8009D018=PE_LoadU32(0x8009D018u);
        D_8009D03C=PE_LoadU32(0x8009D03Cu);
        D_8009D048=PE_LoadU32(0x8009D048u);D_8009D04C=PE_LoadU32(0x8009D04Cu);
        D_8009D050=PE_LoadU32(0x8009D050u);D_8009D054=PE_LoadU32(0x8009D054u);
        D_8009D058=PE_LoadU32(0x8009D058u);D_8009D064=PE_LoadU32(0x8009D064u);
        D_8009D250=PE_LoadU32(0x8009D250u);g_pe_gte.h=256;
        switch (INV19_pe_menu_cases[k].entry) {
        case 0:func_80055610();break;
        case 1:result=(uint32_t)func_8006346C(a);break;
        case 2:func_80046ABC(a);break;
        case 3:result=(uint32_t)func_80046B58(a,args[1]);break;
        case 4:func_80046C20();break;
        case 5:func_80057B70((int32_t)a);break;
        case 6:func_80046DBC(a,args[1]);break;
        case 7:result=(uint32_t)func_8004FC3C((int32_t)a);break;
        case 8:func_80050B48(a);break;
        case 9:func_80061044((int32_t)a,(int32_t)args[1]);break;
        case 10:func_8004FC80(a);break;
        case 11:func_800634D4(a,args[1],(int32_t)args[2],args[3]);break;
        case 12:func_80062FEC();break;
        case 13:result=(uint32_t)func_80044E98(a,args[1]);break;
        case 14:result=(uint32_t)func_80043DA4(a,args[1]);break;
        case 15:func_8005E30C();break;
        }
        PE_StoreU32(0x8009D018u,D_8009D018);
        PE_StoreU32(0x8009D1A0u,D_8009D1A0);
        PE_StoreU32(0x8009D048u,D_8009D048);PE_StoreU32(0x8009D050u,D_8009D050);
        PE_StoreU32(0x8009D058u,D_8009D058);PE_StoreU32(0x8009D064u,D_8009D064);
        hash=hit_camera_hash(INV19_pe_menu_ranges,sizeof(INV19_pe_menu_ranges)/sizeof(INV19_pe_menu_ranges[0]));
        if (hash!=INV19_pe_menu_cases[k].hash || result!=INV19_pe_menu_cases[k].result) {
            fprintf(stderr,"PE menu %u hash %016llX/%016llX result %08X/%08X\n",k,
                (unsigned long long)hash,(unsigned long long)INV19_pe_menu_cases[k].hash,
                result,INV19_pe_menu_cases[k].result);
            if (getenv("PE_INV19_DUMP")) {
                char path[100];FILE *out;
                snprintf(path,sizeof(path),"/tmp/pe-inv19-native-%u.bin",k);out=fopen(path,"wb");
                if (out) {for (i=0;i<0x200000u;i++) fputc(PE_LoadU8(0x80000000u+i),out);fclose(out);}
            }
        }
        if (hash!=INV19_pe_menu_cases[k].hash || result!=INV19_pe_menu_cases[k].result ||
            g_stub_order_count || PE_Port_ShouldStop()) failures++;
    }
    ASSERT(!failures,"PE menu original comparisons or native call graphs failed");
    PASS();
}

/* DAY2: field-menu page constructor reached from func_80043DA4 command 5.
 * Original authority: asm/disc1/37CD0.s 0x8004AD9C..0x8004AE1C (32 words)
 * and the original call site asm/disc1/340EC.s:487 (jal 0x8004AD9C with
 * a0 = the menu list, after func_80062F3C(0x2D/0x18/0x12)). The page really
 * constructs its window and list through the native allocators; the two
 * callback identities it stores (0x8004AE1C input handler, 0x8004FF30 list
 * callback) are the recorded next residual for this page. */
static void test_DAY2_field_menu_page_4ad9c(void)
{
    pe_addr_t node,window,list;
    unsigned i,found;

    TEST("DAY2_field_menu_page_4ad9c");
    ResetTestState();HostFB_Init();PE_GPU_Init();
    func_80062F9C();                          /* menu node pool reset */
    /* func_8005DAB4(0x20) = 0x80092C88; the list spec supplies the column
     * count (+8) and visible row count (+12) 647D0 divides by. */
    PE_StoreU32(0x80092C88u+8u,1u);
    PE_StoreU32(0x80092C88u+12u,4u);
    PE_StoreU32(0x80092C88u+24u,12u);         /* per-row height */
    func_8004AD9C(0x80140000u);
    ASSERT(!PE_Port_ShouldStop()&&!g_stub_order_count,"page constructor boundary");
    window=list=0u;
    for (node=PE_LoadU32(0x8009D154u);node;node=PE_LoadU32(node)) {
        if (PE_LoadU32(node+44u)==0x8004AE1Cu) window=node;
        if (PE_LoadU32(node+48u)==0x8004FF30u) list=node;
    }
    ASSERT(window!=0u&&list!=0u,"page window/list missing");
    ASSERT(PE_LoadU32(window+32u)==1u&&PE_LoadU32(window+36u)==0x20u&&
        PE_LoadU32(window+4u)==0x80140000u,"page window identity");
    ASSERT(PE_LoadU32(list+32u)==2u&&PE_LoadU32(list+4u)==window&&
        PE_LoadU32(list+44u)==0x80063E0Cu,"page list identity");
    /* func_800647D0(list,4) published rows=4 and kept the four visible. */
    ASSERT(PE_LoadU32(list+88u)==4u&&PE_LoadU32(list+56u)==4u,
        "page item count not published");
    found=0u;
    for (i=0u;i<4u;i++) if (PE_LoadU32(window+8u+i*4u)==list) found=1u;
    ASSERT(found,"page list not attached to its window");
    PASS();
}

/* DAY2: field-menu list draw wrapper 0x8004FF30 -> func_800638D8(list, 0x80050C50).
 * Original authority: matched C leaf src/func_8004FF30.c (10 words,
 * [0x8004FF30,0x8004FF58)); hand adapter
 * pc_port/game/boot/func_8004FF30_port.c (bare func_* as a value is a guest
 * code address, so gen_decomp_ports.py skips the leaf, rules E5/E6).
 * The list is parked with zero visible/total rows so the generic renderer
 * exercises the wrapper->renderer path without invoking the per-cell text
 * draw 0x80050C50 (which would need the PE.IMG string archive). Three checks:
 * direct wrapper entry, equivalence with func_800638D8(list, 0x80050C50),
 * and the route's tree-walker dispatch func_80062830(list) via slot +0x30. */
static void test_DAY2_field_menu_draw_4ff30(void)
{
    pe_addr_t node,list;
    uint32_t d164,d168,pool,d164b,d168b,poolb;

    TEST("DAY2_field_menu_draw_4ff30");
    /* 1. Direct wrapper entry. */
    ResetTestState();HostFB_Init();PE_GPU_Init();
    func_80062F9C();
    PE_StoreU32(0x80092C88u+8u,1u);
    PE_StoreU32(0x80092C88u+12u,4u);
    PE_StoreU32(0x80092C88u+24u,12u);
    func_8004AD9C(0x80140000u);
    ASSERT(!PE_Port_ShouldStop()&&!g_stub_order_count,"page constructor boundary");
    list=0u;
    for (node=PE_LoadU32(0x8009D154u);node;node=PE_LoadU32(node))
        if (PE_LoadU32(node+48u)==0x8004FF30u) list=node;
    ASSERT(list!=0u,"page list missing");
    PE_StoreU32(0x8009D12Cu,0x800A2270u);
    PE_StoreU32(0x8009D124u,0u);PE_StoreU32(0x8009D128u,0u);
    PE_StoreU32(0x8009D100u,0x80160000u);PE_StoreU32(0x8009D104u,0x80160000u);
    PE_StoreU32(0x8009D11Cu,0x80185000u);
    PE_StoreU32(list+56u,0u);PE_StoreU32(list+88u,0u);
    func_8004FF30(list);
    ASSERT(!PE_Port_ShouldStop()&&!g_stub_order_count,"4FF30 draw boundary");
    d164=PE_LoadU32(0x8009D164u);d168=PE_LoadU32(0x8009D168u);
    pool=PE_LoadU32(0x8009D100u);
    ASSERT(d164==PE_LoadU32(list+60u)&&d168==PE_LoadU32(list+64u),
        "renderer did not publish cell size");
    ASSERT(pool!=0x80160000u,"renderer allocated no draw packets");

    /* 2. Equivalence with the direct renderer call carrying the recorded
     * callback identity. */
    ResetTestState();HostFB_Init();PE_GPU_Init();
    func_80062F9C();
    PE_StoreU32(0x80092C88u+8u,1u);
    PE_StoreU32(0x80092C88u+12u,4u);
    PE_StoreU32(0x80092C88u+24u,12u);
    func_8004AD9C(0x80140000u);
    list=0u;
    for (node=PE_LoadU32(0x8009D154u);node;node=PE_LoadU32(node))
        if (PE_LoadU32(node+48u)==0x8004FF30u) list=node;
    PE_StoreU32(0x8009D12Cu,0x800A2270u);
    PE_StoreU32(0x8009D124u,0u);PE_StoreU32(0x8009D128u,0u);
    PE_StoreU32(0x8009D100u,0x80160000u);PE_StoreU32(0x8009D104u,0x80160000u);
    PE_StoreU32(0x8009D11Cu,0x80185000u);
    PE_StoreU32(list+56u,0u);PE_StoreU32(list+88u,0u);
    func_800638D8(list,0x80050C50u);
    ASSERT(!PE_Port_ShouldStop()&&!g_stub_order_count,"direct renderer boundary");
    d164b=PE_LoadU32(0x8009D164u);d168b=PE_LoadU32(0x8009D168u);
    poolb=PE_LoadU32(0x8009D100u);
    ASSERT(d164b==d164&&d168b==d168&&poolb==pool,
        "wrapper diverges from func_800638D8(list, 0x80050C50)");

    /* 3. Route dispatch: the tree walker reads slot +0x30 and must reach the
     * wrapper natively (before the leaf it stopped at PE_MenuDrawCallback). */
    ResetTestState();HostFB_Init();PE_GPU_Init();
    func_80062F9C();
    PE_StoreU32(0x80092C88u+8u,1u);
    PE_StoreU32(0x80092C88u+12u,4u);
    PE_StoreU32(0x80092C88u+24u,12u);
    func_8004AD9C(0x80140000u);
    list=0u;
    for (node=PE_LoadU32(0x8009D154u);node;node=PE_LoadU32(node))
        if (PE_LoadU32(node+48u)==0x8004FF30u) list=node;
    PE_StoreU32(0x8009D12Cu,0x800A2270u);
    PE_StoreU32(0x8009D124u,0u);PE_StoreU32(0x8009D128u,0u);
    PE_StoreU32(0x8009D100u,0x80160000u);PE_StoreU32(0x8009D104u,0x80160000u);
    PE_StoreU32(0x8009D11Cu,0x80185000u);
    PE_StoreU32(list+56u,0u);PE_StoreU32(list+88u,0u);
    func_80062830(list);
    ASSERT(!PE_Port_ShouldStop()&&!g_stub_order_count,"62830 draw dispatch boundary");
    ASSERT(PE_LoadU32(0x8009D164u)==d164&&PE_LoadU32(0x8009D168u)==d168,
        "dispatch did not reach the 4FF30 renderer");
    PASS();
}

/* DAY2: field-menu input tree rooted at 0x8004AE1C (jtbl_80011034).
 * Original authority: asm/disc1/37CD0.s (handler 0x8004AE1C..0x8004AF3C,
 * 0x120 bytes) and the retail jump table asm/disc1/data/800.rodata.s:1309
 *   jtbl_80011034 = {0x8004AE84, 0x8004AE94, 0x8004AEA4, 0x8004AEB4,
 *                    0x8004AEC4, 0x8004AEC4}
 * (0..3 -> the four sub-page constructors, 4/5 -> the shared close arm).
 * The page is built natively by func_8004AD9C, then the handler is entered
 * with the confirm event 0x10000 and each index.  Everything the four
 * constructors install is checked by identity; the remaining untranslated
 * leaves must be *named* boundary stubs, never the generic default. */

static pe_addr_t menu4ae1c_slot(unsigned offset,uint32_t id)
{
    pe_addr_t node;
    for (node=PE_LoadU32(0x8009D154u);node;node=PE_LoadU32(node))
        if (PE_LoadU32(node+offset)==id) return node;
    return 0u;
}

static void menu4ae1c_build(pe_addr_t *window,pe_addr_t *list)
{
    pe_addr_t node;
    func_80062F9C();                          /* menu node pool reset */
    PE_StoreU32(0x80092C88u+8u,1u);
    PE_StoreU32(0x80092C88u+12u,4u);
    PE_StoreU32(0x80092C88u+24u,12u);
    func_8004AD9C(0x80140000u);
    *window=*list=0u;
    for (node=PE_LoadU32(0x8009D154u);node;node=PE_LoadU32(node)) {
        if (PE_LoadU32(node+44u)==0x8004AE1Cu) *window=node;
        if (PE_LoadU32(node+48u)==0x8004FF30u) *list=node;
    }
}

/* Select list index `index`: func_80063428(list)=column_count*row+column. */
static void menu4ae1c_select(pe_addr_t list,uint32_t index)
{
    PE_StoreU32(list+52u,1u);
    PE_StoreU32(list+72u,0u);
    PE_StoreU32(list+68u,index);
}

static void menu4ae1c_draw_setup(pe_addr_t list)
{
    PE_StoreU32(0x8009D12Cu,0x800A2270u);
    PE_StoreU32(0x8009D124u,0u);PE_StoreU32(0x8009D128u,0u);
    PE_StoreU32(0x8009D100u,0x80160000u);PE_StoreU32(0x8009D104u,0x80160000u);
    PE_StoreU32(0x8009D11Cu,0x80185000u);
    PE_StoreU32(list+56u,0u);PE_StoreU32(list+88u,0u);
}

static void test_DAY2_field_menu_input_4ae1c(void)
{
    static const uint32_t handler[4]={0x8004AFA4u,0x8004B0A4u,0x8004B394u,0x8004B650u};
    static const uint32_t slot30[4] ={0x8004FF58u,0x8004FF80u,0x8004B214u,0x8004B5DCu};
    pe_addr_t window,list,sub_window,sub_list;
    unsigned i;

    TEST("DAY2_field_menu_input_4ae1c");

    /* 1. Jump-table cases 0..3 build their sub-page natively and install the
     *    recorded handler/draw identities. */
    for (i=0u;i<4u;i++) {
        ResetTestState();HostFB_Init();PE_GPU_Init();
        menu4ae1c_build(&window,&list);
        ASSERT(window!=0u&&list!=0u,"page window/list missing");
        menu4ae1c_select(list,i);
        ASSERT(func_8004AE1C(window,0x10000u)==1,"handler did not return 1");
        ASSERT(!PE_Port_ShouldStop()&&!g_stub_order_count,"case dispatch boundary");
        ASSERT(menu4ae1c_slot(44u,handler[i])!=0u,"sub-page window not constructed");
        ASSERT(menu4ae1c_slot(48u,slot30[i])!=0u,"sub-page list/draw not installed");
    }
    /* case 2 is the two-list equipment page: both list draws must exist. */
    ResetTestState();HostFB_Init();PE_GPU_Init();
    menu4ae1c_build(&window,&list);
    menu4ae1c_select(list,2u);
    ASSERT(func_8004AE1C(window,0x10000u)==1,"case 2 return");
    ASSERT(menu4ae1c_slot(48u,0x8004B534u)!=0u&&menu4ae1c_slot(48u,0x8004B55Cu)!=0u,
        "equipment page second list draw missing");
    /* case 3 publishes func_8005E884() into 0x8009D264. */
    ResetTestState();HostFB_Init();PE_GPU_Init();
    menu4ae1c_build(&window,&list);
    menu4ae1c_select(list,3u);
    ASSERT(func_8004AE1C(window,0x10000u)==1,"case 3 return");
    ASSERT(PE_LoadU32(0x8009D264u)==(uint32_t)(int32_t)func_8005E884(),
        "modal case did not publish the alarm-timer word");

    /* 2. index >= 6 takes the shared tail: no sub-page, no boundary. */
    ResetTestState();HostFB_Init();PE_GPU_Init();
    menu4ae1c_build(&window,&list);
    menu4ae1c_select(list,7u);
    ASSERT(func_8004AE1C(window,0x10000u)==1,"index>=6 return");
    ASSERT(!PE_Port_ShouldStop()&&!g_stub_order_count,"index>=6 boundary");
    ASSERT(menu4ae1c_slot(44u,0x8004AFA4u)==0u&&menu4ae1c_slot(44u,0x8004B0A4u)==0u,
        "index>=6 constructed a sub-page");

    /* 3. cancel arm (event&0x40, no confirm) closes without a boundary. */
    ResetTestState();HostFB_Init();PE_GPU_Init();
    menu4ae1c_build(&window,&list);
    ASSERT(func_8004AE1C(window,0x40u)==1,"cancel return");
    ASSERT(!PE_Port_ShouldStop()&&!g_stub_order_count,"cancel boundary");

    /* 4. Items sub-page confirm (func_8004AFA4): the item selection index is
     *    committed through func_80052790 -> D_8009D020. */
    ResetTestState();HostFB_Init();PE_GPU_Init();
    menu4ae1c_build(&window,&list);
    menu4ae1c_select(list,0u);
    ASSERT(func_8004AE1C(window,0x10000u)==1&&!PE_Port_ShouldStop(),"items sub-page");
    sub_window=menu4ae1c_slot(44u,0x8004AFA4u);
    sub_list=menu4ae1c_slot(48u,0x8004FF58u);
    ASSERT(sub_window!=0u&&sub_list!=0u,"items sub-page missing");
    PE_StoreU32(0x8009D020u,0xDEADBEEFu);
    menu4ae1c_select(sub_list,2u);
    ASSERT(func_8004AFA4(sub_window,0x10000u)==1,"4AFA4 confirm return");
    ASSERT(!PE_Port_ShouldStop()&&!g_stub_order_count,"4AFA4 confirm boundary");
    ASSERT(PE_LoadU32(0x8009D020u)==2u,"52790 did not store the item selection");

    /* 5. Escape sub-page confirm (func_8004B0A4) resets resource state through
     *    func_800649D0 instead, so D_8009D020 must stay untouched. */
    ResetTestState();HostFB_Init();PE_GPU_Init();
    menu4ae1c_build(&window,&list);
    menu4ae1c_select(list,1u);
    ASSERT(func_8004AE1C(window,0x10000u)==1&&!PE_Port_ShouldStop(),"escape sub-page");
    sub_window=menu4ae1c_slot(44u,0x8004B0A4u);
    sub_list=menu4ae1c_slot(48u,0x8004FF80u);
    ASSERT(sub_window!=0u&&sub_list!=0u,"escape sub-page missing");
    PE_StoreU32(0x8009D020u,0xDEADBEEFu);
    menu4ae1c_select(sub_list,1u);
    ASSERT(func_8004B0A4(sub_window,0x10000u)==1,"4B0A4 confirm return");
    ASSERT(!PE_Port_ShouldStop()&&!g_stub_order_count,"4B0A4 confirm boundary");
    ASSERT(PE_LoadU32(0x8009D020u)==0xDEADBEEFu,
        "Escape confirm must not store an item slot");

    /* 6. Items sub-page draw wrapper dispatch (0x8004FF58 -> func_800638D8)
     *    with zero rows (no per-cell draw), then the per-cell draw body itself
     *    against a seeded text archive. */
    ResetTestState();HostFB_Init();PE_GPU_Init();
    g_bootstrap_disc=1;func_800698D4();          /* seed the text archive */
    g_stub_count=0;g_stub_order_count=0;         /* seeding logs its own stubs */
    menu4ae1c_build(&window,&list);
    menu4ae1c_select(list,0u);
    ASSERT(func_8004AE1C(window,0x10000u)==1&&!PE_Port_ShouldStop(),"items sub-page draw");
    sub_list=menu4ae1c_slot(48u,0x8004FF58u);
    ASSERT(sub_list!=0u,"items sub-page list missing");
    menu4ae1c_draw_setup(sub_list);
    /* One visible row/column so the renderer walks func_800634D4's cell loop
     * and dispatches the per-cell draw 0x80050C70 through menu_draw_callback
     * (the arm this test adds). */
    PE_StoreU32(sub_list+52u,1u);PE_StoreU32(sub_list+56u,1u);
    PE_StoreU32(sub_list+88u,1u);PE_StoreU32(sub_list+92u,0u);
    func_80062830(sub_list);
    ASSERT(!PE_Port_ShouldStop()&&!g_stub_order_count,"4FF58/50C70 draw dispatch boundary");
    PE_StoreU32(0x8009D020u,0u);
    func_80050C70(sub_list);                     /* focused elsewhere: no highlight */
    ASSERT(!PE_Port_ShouldStop()&&!g_stub_order_count,"50C70 per-cell boundary");
    PE_StoreU32(0x8009D020u,(uint32_t)sub_list);
    func_80050C70(sub_list);                     /* focused: highlight branch */
    ASSERT(!PE_Port_ShouldStop()&&!g_stub_order_count,"50C70 highlight boundary");

    /* 7. The modal window's own draw (0x8004B5DC -> func_8005FCAC /
     *    func_8005ED18) is native too, so the tree has no named boundary left. */
    ResetTestState();HostFB_Init();PE_GPU_Init();
    menu4ae1c_build(&window,&list);
    menu4ae1c_select(list,3u);
    ASSERT(func_8004AE1C(window,0x10000u)==1&&!PE_Port_ShouldStop(),"modal sub-page");
    sub_window=menu4ae1c_slot(44u,0x8004B650u);
    ASSERT(sub_window!=0u,"modal window missing");
    menu4ae1c_draw_setup(sub_window);
    func_80062830(sub_window);
    ASSERT(!PE_Port_ShouldStop()&&!g_stub_order_count,"modal draw boundary");
    ASSERT(CountOrderLog("PE_MenuDrawCallback_8004B5DC")==0,"modal draw still named");

    /* 8. Equipment sub-page (case 2): window draw 0x8004B214, input 0x8004B394
     *    and list1 per-cell draw 0x80050438 all native. */
    ResetTestState();HostFB_Init();PE_GPU_Init();
    menu4ae1c_build(&window,&list);
    menu4ae1c_select(list,2u);
    ASSERT(func_8004AE1C(window,0x10000u)==1&&!PE_Port_ShouldStop(),"equipment page");
    sub_window=menu4ae1c_slot(44u,0x8004B394u);
    ASSERT(sub_window!=0u,"equipment window missing");
    menu4ae1c_draw_setup(sub_window);
    func_8004B214(sub_window);
    ASSERT(!PE_Port_ShouldStop()&&!g_stub_order_count,"equipment window draw boundary");
    func_8004B394(sub_window,0x1000u);
    ASSERT(!PE_Port_ShouldStop()&&!g_stub_order_count,"equipment lane-up boundary");
    func_8004B394(sub_window,0x4000u);
    ASSERT(!PE_Port_ShouldStop()&&!g_stub_order_count,"equipment lane-down boundary");
    func_80050438(0u);
    ASSERT(!PE_Port_ShouldStop()&&!g_stub_order_count,"equipment per-cell draw boundary");

    /* 9. Equipment confirm (0x10000) and cancel (0x40): both close the window,
     *    so each arm gets a fresh page. */
    for (i=0u;i<2u;i++) {
        ResetTestState();HostFB_Init();PE_GPU_Init();
        menu4ae1c_build(&window,&list);
        menu4ae1c_select(list,2u);
        ASSERT(func_8004AE1C(window,0x10000u)==1,"equipment page (2)");
        sub_window=menu4ae1c_slot(44u,0x8004B394u);
        ASSERT(sub_window!=0u,"equipment window missing (2)");
        ASSERT(func_8004B394(sub_window,i?0x40u:0x10000u)==1,"equipment handler return");
        ASSERT(!PE_Port_ShouldStop()&&!g_stub_order_count,"equipment handler boundary");
    }

    /* 10. Modal sub-page input (0x8004B650): direction arms, confirm and cancel
     *     (the last two close the modal, so fresh pages again). */
    for (i=0u;i<4u;i++) {
        static const uint32_t modal_event[4]={0x1000u,0x4000u,0x10000u,0x40u};
        ResetTestState();HostFB_Init();PE_GPU_Init();
        menu4ae1c_build(&window,&list);
        menu4ae1c_select(list,3u);
        ASSERT(func_8004AE1C(window,0x10000u)==1,"modal page (2)");
        sub_window=menu4ae1c_slot(44u,0x8004B650u);
        ASSERT(sub_window!=0u,"modal window missing (2)");
        ASSERT(func_8004B650(sub_window,modal_event[i])==1,"modal handler return");
        ASSERT(!PE_Port_ShouldStop()&&!g_stub_order_count,"modal handler boundary");
    }

    /* 11. Close page (case 4/5 -> func_8005D994): native, and it commits the
     *     full heal through func_8005247C. */
    ResetTestState();HostFB_Init();PE_GPU_Init();
    PE_StoreU32(0x8009D254u,0x80100000u);        /* active actor */
    PE_StoreU32(0x80100000u,0x80110000u);        /* record */
    PE_StoreU16(0x80110000u+0x0Cu,12u);          /* current HP */
    PE_StoreU16(0x80110000u+0x1Cu,45u);          /* max HP */
    func_8005D994(0);
    ASSERT(!PE_Port_ShouldStop()&&!g_stub_order_count,"close page boundary");
    ASSERT(PE_LoadU8(0x800C0E0Au)==0x62u,"close page row byte");
    ASSERT(PE_LoadU32(0x800C0E24u)==0xFFFFFu,"close page fixed-point constant");
    /* func_8005247C -> func_8005218C recomputes the row capacity through
     * func_80052F24, which clamps to 50. */
    ASSERT(PE_LoadU8(0x800C0E0Cu)<=50u,"close page capacity clamp");
    for (i=0u;i<7u;i++)
        ASSERT(PE_LoadU16(0x800C0E28u+i*2u)==0x3E8u,"close page stat row (index 0)");
    /* func_8005218C recomputes the record's max at +0x1C, then func_8005247C
     * commits it to HP (+0x0C) and the gauge mirror (+0x0E); the stale HP=12
     * must be overwritten. */
    ASSERT(PE_LoadU16(0x80110000u+0x0Cu)==PE_LoadU16(0x80110000u+0x1Cu),
        "close page heal commit");
    ASSERT(PE_LoadU16(0x80110000u+0x0Eu)==PE_LoadU16(0x80110000u+0x1Cu),
        "close page gauge mirror");

    /* 12. The jump-table case-4 arm reaches that close page natively. */
    ResetTestState();HostFB_Init();PE_GPU_Init();
    menu4ae1c_build(&window,&list);
    menu4ae1c_select(list,4u);
    ASSERT(func_8004AE1C(window,0x10000u)==1,"close case return");
    ASSERT(!PE_Port_ShouldStop()&&!g_stub_order_count,"close case boundary");

    /* 13. Modal-draw leaves.  func_8005FCAC prints a signed number: three cells
     *     for 0 (no icon), sign icon + two cells for a negative OR a positive
     *     (retail 0x8005FCF0: both sign arms share `jal func_8005EB64` with
     *     `li s2,2` in its delay slot), advancing x by 5 per cell.  The old hand
     *     port drew three cells for a positive; portverify round 6. */
    ResetTestState();HostFB_Init();PE_GPU_Init();
    menu4ae1c_build(&window,&list);
    menu4ae1c_draw_setup(list);
    PE_StoreU32(0x8009D124u,0x64u);
    PE_StoreU32(0x8009D128u,0x32u);
    func_8005FCAC(0);
    ASSERT(!PE_Port_ShouldStop()&&!g_stub_order_count,"5FCAC zero boundary");
    ASSERT(PE_LoadU32(0x8009D124u)==0x64u+15u,"5FCAC zero = three cells");
    func_8005FCAC(-12);
    ASSERT(PE_LoadU32(0x8009D124u)==0x64u+15u+5u+10u,"5FCAC negative = icon+two");
    func_8005FCAC(7);
    ASSERT(PE_LoadU32(0x8009D124u)==0x64u+15u+5u+10u+5u+10u,"5FCAC positive = icon+two");
    ASSERT(PE_LoadU32(0x8009D128u)==0x32u,"5FCAC leaves y alone");

    /* 14. func_8005ED18 builds and links one 0x28-byte primitive per call. */
    PE_StoreU32(0x8009D100u,0x80160000u);
    PE_StoreU32(0x8009D104u,0x80160000u);
    PE_StoreU32(0x8009D11Cu,0x80185000u);
    PE_StoreU32(0x8009D110u,0x11223344u);
    PE_StoreU32(0x8009D114u,0x55667788u);
    PE_StoreU32(0x8009D10Cu,0u);
    PE_StoreU32(0x8009D124u,0x20u);
    PE_StoreU32(0x8009D128u,0x30u);
    PE_StoreU32(0x80185000u,0u);            /* empty ordering table */
    func_8005ED18(0x7Bu,2u);
    ASSERT(!PE_Port_ShouldStop()&&!g_stub_order_count,"5ED18 boundary");
    ASSERT(PE_LoadU32(0x8009D100u)==0x80160028u,"5ED18 reserves 0x28 bytes");
    /* The +7 byte is the GPU command, the top byte of the colour word at +4
     * (same packing as PE_MenuPacketColor/PE_MenuPacketLink); the +3 byte is the
     * word count, the top byte of the link word at +0. */
    ASSERT(PE_LoadU32(0x80160004u)==0x2C223344u,"5ED18 undimmed colour word");
    ASSERT(PE_LoadU8(0x80160003u)==9u&&PE_LoadU8(0x80160007u)==0x2Cu,"5ED18 prim header");
    ASSERT(PE_LoadU16(0x80160008u)==0x20u&&PE_LoadU16(0x8016000Au)==0x30u,
        "5ED18 origin");
    ASSERT(PE_LoadU32(0x80160000u)==0x09000000u,"5ED18 link word");
    ASSERT(PE_LoadU32(0x80185000u)==0x00160000u,"5ED18 links into the OT");
    /* Second call: the dim flag selects the halved colour and the previous
     * packet becomes the new head's link. */
    PE_StoreU32(0x8009D10Cu,1u);
    func_8005ED18(0x7Bu,0u);
    ASSERT(PE_LoadU32(0x8016002Cu)==0x2C667788u,"5ED18 dimmed colour word");
    ASSERT(PE_LoadU32(0x8009D100u)==0x80160050u,"5ED18 second reservation");
    ASSERT(PE_LoadU32(0x80160028u)==0x09160000u,"5ED18 second link word");
    ASSERT(PE_LoadU32(0x80185000u)==0x00160028u,"5ED18 second link");

    PASS();
}
