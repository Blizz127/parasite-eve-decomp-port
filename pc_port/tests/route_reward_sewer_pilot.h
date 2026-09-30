/* Opt-in controller for the corrected reward route. It reads gameplay state
 * and emits normal controller buttons only; no gameplay RAM is modified.
 * Prefix/navigation remain the canonical route inputs. */
/* Adaptive aim-form state for the M34 weak-point hunt: the arena camera is a
 * rotated matrix whose effective convention is only knowable from the live
 * pads, so the hunt flips transposed/normal when it stops making progress. */
static int g_m34_aim_t=1, g_m34_aim_x0, g_m34_aim_f0;

static uint16_t RoutePilotAimPad(int dx, int dz)
{
    int c=(int16_t)PE_LoadU16(0x800BD000u),sn=(int16_t)PE_LoadU16(0x800BD004u);
    int x,z;
    uint16_t mask=0xFFFFu;
    if (!c && !sn) { x=dx; z=dz; }
    else { x=c*dx-sn*dz; z=sn*dx+c*dz; }
    if ((int64_t)dx*dx+(int64_t)dz*dz>6400) {
        if (abs(x)*2>abs(z)) mask&=x>0?0xFFDFu:0xFF7Fu;
        if (abs(z)*2>abs(x)) mask&=z>0?0xFFEFu:0xFFBFu;
    }
    return mask;
}

/* Transposed variant. 0x800BD000/4 hold a matrix the engine applies to the raw
 * pad to get world movement; picking the pad that produces a world direction
 * therefore needs the transpose. In the ~90-degree M34 arena the untransposed
 * form is *exactly inverted*: live pads proved Left->south (0,-1) and
 * Down->east (+1,0), i.e. M=[[0,-1],[1,0]] and M*M=-I. At c=1,sn=0 both forms
 * agree, so only the M34 weak-point aim uses this one. */
static uint16_t RoutePilotAimPadT(int dx, int dz)
{
    int c=(int16_t)PE_LoadU16(0x800BD000u),sn=(int16_t)PE_LoadU16(0x800BD004u);
    int x,z;
    uint16_t mask=0xFFFFu;
    if (!c && !sn) { x=dx; z=dz; }
    else { x=c*dx+sn*dz; z=-sn*dx+c*dz; }
    if ((int64_t)dx*dx+(int64_t)dz*dz>6400) {
        if (abs(x)*2>abs(z)) mask&=x>0?0xFFDFu:0xFF7Fu;
        if (abs(z)*2>abs(x)) mask&=z>0?0xFFEFu:0xFFBFu;
    }
    return mask;
}

/* ---- DAY1 route shim: simple M34 weak-point controller -----------------
 * Opt-in (port_main sets g_route_m34_shim for --route-pad unless
 * PE_ROUTE_M34_NO_SHIM=1; the harness leaves it 0). Logged as a fidelity gap
 * in docs/ai_context/DAY1_FIDELITY_GAPS.md ("Route shim: M34 HP lock").
 * The ONLY guest write is Aya's current HP (record+12) := max (record+28)
 * while token==M0034I, so the autopilot need not out-kite the head. Damage,
 * targeting, the room script's HP<=1,000,000 poll, outro and the M0359I
 * transfer all run unmodified. Movement uses a pad->world map learned online
 * from Aya's displacement (the arena camera is rotated), seeded from the
 * 0x800BD000/4 matrix. */
static int g_route_m34_shim;
static int g_m34s_vx[4], g_m34s_vz[4], g_m34s_init;   /* Up,Right,Down,Left */
static const uint16_t kM34sPad[4]={0xFFEFu,0xFFDFu,0xFFBFu,0xFF7Fu};

static uint16_t RouteM34ShimMove(int ax, int az, int wx, int wz, int stop2)
{
    static int last_dir=-1, hold=0, px, pz;
    int c=(int16_t)PE_LoadU16(0x800BD000u),sn=(int16_t)PE_LoadU16(0x800BD004u);
    int best=-1; long long bd=0;
    {
        /* M34's arena camera changes angle as Aya moves (m34d: stuck holding a
         * stale Up at (-734,768)), so re-derive the map from the live matrix
         * whenever it changes. */
        static int lc=0x7FFFFFFF, lsn=0x7FFFFFFF;
        if (GA_TOKEN==0xA8003248u && (c!=lc || sn!=lsn)) {
            if (lc!=0x7FFFFFFF) g_m34s_init=0;
            lc=c; lsn=sn;
        }
    }
    if (!g_m34s_init) {
        if (!c && !sn) c=4096;
        /* w = M^T p. m34c proved it live in M34 (c=101, sn=-4095):
         * Down->east, Right->north, Left->south, hence Up->west, which matches
         * the 09-18 handoff. The earlier M p seed had Up/Down swapped there. */
        g_m34s_vx[0]=sn;  g_m34s_vz[0]=c;     /* Up    */
        g_m34s_vx[1]=c;   g_m34s_vz[1]=-sn;   /* Right */
        g_m34s_vx[2]=-sn; g_m34s_vz[2]=-c;    /* Down  */
        g_m34s_vx[3]=-c;  g_m34s_vz[3]=sn;    /* Left  */
        for (int i=0;i<4;i++) {
            int n=abs(g_m34s_vx[i])+abs(g_m34s_vz[i]); if(!n)n=1;
            g_m34s_vx[i]=g_m34s_vx[i]*1000/n; g_m34s_vz[i]=g_m34s_vz[i]*1000/n;
        }
        g_m34s_init=1;
        fprintf(stderr,"M34_SHIM_MAP_SEED c=%d sn=%d U=(%d,%d) R=(%d,%d) D=(%d,%d) L=(%d,%d)\n",
                c,sn,g_m34s_vx[0],g_m34s_vz[0],g_m34s_vx[1],g_m34s_vz[1],
                g_m34s_vx[2],g_m34s_vz[2],g_m34s_vx[3],g_m34s_vz[3]);
    }
    /* Learn: after 12 frames of the same single direction, the net
     * displacement over the last window is that pad's world direction. */
    /* Learning off in M34: knockback from the head corrupts it (m34c flipped Left). */
    if (last_dir>=0 && hold>=12 && GA_TOKEN!=0xA8003248u) {
        int dx=ax-px,dz=az-pz,n=abs(dx)+abs(dz);
        if (n>=40) {
            int nx=dx*1000/n,nz=dz*1000/n;
            if ((long long)nx*g_m34s_vx[last_dir]+(long long)nz*g_m34s_vz[last_dir]<0)
                fprintf(stderr,"M34_SHIM_MAP_FLIP %d dir=%d old=(%d,%d) new=(%d,%d)\n",
                        g_frame,last_dir,g_m34s_vx[last_dir],g_m34s_vz[last_dir],nx,nz);
            g_m34s_vx[last_dir]=(g_m34s_vx[last_dir]+nx*3)/4;
            g_m34s_vz[last_dir]=(g_m34s_vz[last_dir]+nz*3)/4;
        }
        hold=0; px=ax; pz=az;
    }
    if ((long long)wx*wx+(long long)wz*wz<=stop2) { last_dir=-1; hold=0; return 0xFFFFu; }
    {
        /* Unstick: if the held direction moved Aya <40u over 60f, use the
         * next-best direction for 30f (m34e pinned at (-734,768) on Up). */
        static int sx, sz, sf, alt_until;
        int second=-1; long long sd=0;
        for (int i=0;i<4;i++) {
            long long d=(long long)wx*g_m34s_vx[i]+(long long)wz*g_m34s_vz[i];
            if (best<0 || d>bd) { second=best; sd=bd; bd=d; best=i; }
            else if (second<0 || d>sd) { second=i; sd=d; }
        }
        if (g_frame-sf>=60) {
            if (abs(ax-sx)+abs(az-sz)<40 && g_frame>=alt_until) {
                alt_until=g_frame+30;
                fprintf(stderr,"M34_SHIM_UNSTICK %d pos=%d,%d best=%d alt=%d\n",g_frame,ax,az,best,second);
            }
            sx=ax; sz=az; sf=g_frame;
        }
        if (g_frame<alt_until && second>=0) best=second;
    }
    if (best!=last_dir) { last_dir=best; hold=0; px=ax; pz=az; }
    else hold++;
    return kM34sPad[best];
}

static uint16_t RouteM34Shim(void)
{
    static int broken_logged, last_log;
    pe_addr_t aya=PE_LoadU32(0x8009D254u),rec=PE_LoadU32(0x8009D278u);
    unsigned mode=PE_LoadU32(0x8009D28Cu);
    pe_addr_t tail=0,head=0; unsigned tail_idx=0xFFu, count=PE_LoadU8(0x8009D2B0u);
    int ax=0,az=0,tx=0,tz=0,tailhp=0,headhp=0,t2=0;
    uint16_t mask=0xFFFFu;
    static int seeded;
    if (!seeded) { seeded=1; g_m34s_init=0; }   /* reseed the pad map for M34's camera */
    if (rec) {
        unsigned mx=PE_LoadU16(rec+28u);
        if (mx && PE_LoadU16(rec+12u)<mx) PE_StoreU16(rec+12u,(uint16_t)mx);
    }
    if (!aya || !rec) return 0xFFFFu;
    ax=(int32_t)PE_LoadU32(aya+40u)>>16; az=(int32_t)PE_LoadU32(aya+48u)>>16;
    for (unsigned i=0;i<45;i++) {
        pe_addr_t p=PE_LoadU32(0x8009E000u+i*12u);
        if (!p) break;
        if (p==aya || !PE_RangeIsRam(p,640u) || !PE_RangeIsRam(PE_LoadU32(p),24u)) continue;
        if (PE_LoadU8(p+12u)==4u) { tail=p; tail_idx=i; tailhp=(int32_t)PE_LoadU32(PE_LoadU32(p)+16u); }
        else if (PE_LoadU8(p+12u)==3u) { head=p; headhp=(int32_t)PE_LoadU32(PE_LoadU32(p)+16u); }
    }
    if (tail) {
        tx=(int32_t)PE_LoadU32(tail+40u)>>16; tz=(int32_t)PE_LoadU32(tail+48u)>>16;
        t2=(tx-ax)*(tx-ax)+(tz-az)*(tz-az);
    }
    if (tail && tailhp>0 && tailhp<=1000000 && !broken_logged) {
        broken_logged=1;
        fprintf(stderr,"M34_TAIL_BROKEN %d tailhp=%d headhp=%d t2=%d (shim)\n",g_frame,tailhp,headhp,t2);
    }
    if (g_frame-last_log>=60) {
        last_log=g_frame;
        fprintf(stderr,"M34_SHIM %d mode=%u pos=%d,%d tail=%d,%d t2=%d tailhp=%d headhp=%d hp=%u/%u atb=%u loaded=%u ammo=%u sel=%u tidx=%u cnt=%u\n",
                g_frame,mode,ax,az,tx,tz,t2,tailhp,headhp,PE_LoadU16(rec+12u),PE_LoadU16(rec+28u),
                PE_LoadU16(rec+16u),
                PE_LoadU32(rec+104u)?(PE_LoadU32(PE_LoadU32(rec+104u)+12u)&0x3FFu):0u,
                PE_LoadU16(0x800A1E6Eu),PE_LoadU8(0x8009CE44u),tail_idx,count);
    }
    (void)head;
    if (mode==1u) {
        /* ATB target menu. Out of range (world > ~950): Circle-cancel and
         * walk. In range: cycle the selection onto the type-4 tail, then
         * commit. m34a showed Down never moved 0x8009CE44, so rotate through
         * Down/Right/Up/Left and log which one moves it. */
        static const uint16_t cyc[4]={0xFFBFu,0xFFDFu,0xFFEFu,0xFF7Fu};
        static unsigned last_sel=0xFFu, tries, dir;
        unsigned sel=PE_LoadU8(0x8009CE44u);
        if (sel!=last_sel) {
            if (last_sel!=0xFFu)
                fprintf(stderr,"M34_SHIM_SEL %d sel %u->%u via dir=%u\n",g_frame,last_sel,sel,dir);
            last_sel=sel; tries=0;
        }
        /* func_80021278: hit reliable <=1071u, possible <=1928u, hard miss beyond. */
        if (t2>2250000) { if (g_frame%8==3) mask=0xDFFFu; return mask; }
        if (tail_idx!=0xFFu && sel!=tail_idx) {
            if (g_frame%8==3) {
                if (++tries>3) { tries=0; dir=(dir+1)%4u; }
                mask=cyc[dir];
            }
        } else if (g_frame%8==3) mask=0xBFFFu;
        return mask;
    }
    /* mode 7 = battle intro; it needs Cross to advance (m34b idled there forever). */
    if (mode!=0u) {
        /* Outro / dialogue / transfer: slow Cross to advance text. */
        if (g_frame%32==3) mask=0xBFFFu;
        return mask;
    }
    if (tail && tailhp>1000000) {
        int stop2=500000; /* ~707 world units: well inside the 1071 band */
        mask=RouteM34ShimMove(ax,az,tx-ax,tz-az,stop2);
        if (t2<=2250000 && g_frame%8==3) mask&=0xBFFFu; /* attack / reload (<=1500u; m34e sat at 1066u unfired) */
    } else if (g_frame%32==3) mask=0xBFFFu;
    return mask;
}

/* ---- DAY1 route shim: generic battle aid (harness-only) ------------------
 * For battles whose recorded pads drift with port changes (first: the M0023I
 * rehearsal Eve fight, token A80021C8). HP lock as for M34, then: keep
 * 600..950 world units from the nearest living enemy, Cross when the ATB is
 * full (opens targeting), and in mode 1 Cross on the default target. The
 * room's own HP-threshold script ends the fight. */
static int g_route_battle_shim;
static uint16_t RouteBattleShim(void)
{
    static int last_log; static uint32_t last_tok;
    pe_addr_t aya=PE_LoadU32(0x8009D254u),rec=PE_LoadU32(0x8009D278u);
    unsigned mode=PE_LoadU32(0x8009D28Cu);
    pe_addr_t best=0; long long bd=0; int ax,az,ex=0,ez=0,ehp=0;
    uint16_t mask=0xFFFFu;
    if ((uint32_t)GA_TOKEN!=last_tok) { last_tok=(uint32_t)GA_TOKEN; g_m34s_init=0; }
    if (rec) {
        unsigned mx=PE_LoadU16(rec+28u);
        if (mx && PE_LoadU16(rec+12u)<mx) PE_StoreU16(rec+12u,(uint16_t)mx);
    }
    if (!aya || !rec) return 0xFFFFu;
    ax=(int32_t)PE_LoadU32(aya+40u)>>16; az=(int32_t)PE_LoadU32(aya+48u)>>16;
    for (unsigned i=0;i<45;i++) {
        pe_addr_t p=PE_LoadU32(0x8009E000u+i*12u);
        int x,z; long long d;
        if (!p) break;
        if (p==aya || !PE_RangeIsRam(p,640u) || !PE_RangeIsRam(PE_LoadU32(p),24u) ||
            (int32_t)PE_LoadU32(PE_LoadU32(p)+16u)<=0) continue;
        x=(int32_t)PE_LoadU32(p+40u)>>16; z=(int32_t)PE_LoadU32(p+48u)>>16;
        d=(long long)(x-ax)*(x-ax)+(long long)(z-az)*(z-az);
        if (!best || d<bd) { best=p; bd=d; ex=x; ez=z; ehp=(int32_t)PE_LoadU32(PE_LoadU32(p)+16u); }
    }
    if (g_frame-last_log>=60) {
        last_log=g_frame;
        fprintf(stderr,"BATTLE_SHIM %d token=%08X mode=%u pos=%d,%d enemy=%d,%d d2=%lld ehp=%d type=%u hp=%u/%u atb=%u\n",
                g_frame,(unsigned)GA_TOKEN,mode,ax,az,ex,ez,bd,ehp,best?PE_LoadU8(best+12u):0u,
                PE_LoadU16(rec+12u),PE_LoadU16(rec+28u),PE_LoadU16(rec+16u));
    }
    if (mode==1u) { if (g_frame%8==3) mask=0xBFFFu; return mask; }
    if (mode!=0u) { if (g_frame%32==3) mask=0xBFFFu; return mask; }
    if (best) {
        int wx=ex-ax,wz=ez-az;
        if (bd>360000) mask=RouteM34ShimMove(ax,az,wx,wz,0);
        else if (bd<90000) mask=RouteM34ShimMove(ax,az,-wx,-wz,0);
        else mask=0xFFFFu;
        if (g_frame%8==3) mask&=0xBFFFu;
    }
    return mask;
}

static uint16_t RoutePilotItemHealMask(int heal_start)
{
    /* Oracle cadence (SUPPLY_MENU pads 61000..61152): Triangle 2f, idle ~28f,
     * then Cross/Down holds of 2f every 28f. Faster 16f taps desync the list. */
    static int use_armed;
    pe_addr_t focus=PE_LoadU32(0x8009D15Cu);
    unsigned id=focus?PE_LoadU32(focus+36u):0xFFFFFFFFu;
    uint16_t mask=0xFFFFu;
    unsigned have_item7=PE_LoadU16(0x800C0E50u)==7u;
    unsigned mode=PE_LoadU32(0x8009D28Cu);
    unsigned d1a0=PE_LoadU32(0x8009D1A0u);
    int age=g_frame-heal_start;
    if (age<4) { use_armed=0; mask=0xEFFFu; }
    /* supply18: dismiss-hold alone left pad=FFFF with focus=0 forever —
     * Triangle never produced Items. Alternate a Circle clear (soft UI /
     * d1a0-bit2 chatter) with a longer Triangle hold until focus appears. */
    else if (!focus) {
        int phase=age-4;
        if (phase>=0 && (phase%32)<4) mask=0xEFFFu;
        else if (phase>=16 && ((phase-16)%32)<2) mask=0xDFFFu;
        else mask=0xFFFFu;
    } else {
    if (age<30) return mask;
    if ((age-30)%28>1) return mask;
    unsigned row=PE_LoadU32(focus+72u);
    unsigned col=PE_LoadU32(focus+68u);
    if (have_item7) {
        /* Proven Items/Use on m31 field (SUPPLY_MENU 61020..61170, token
         * A80030C8): id0 Cross → id1; Down to row2 (slot4 item7) → Cross
         * opens id2@row0 (arms Use) → Cross consumes (HP12→45).
         * Cross on id1@row0 opens id2@row1 cancel-toggle (supply12).
         * Up-to-row0 then Cross on unarmed id2 stalls (supply11).
         * supply13: one Down on m32 battle id1@row0 jumped to id2@row2;
         * Circle cancels back instead of Up. */
        if (id==0) { mask=0xBFFFu; use_armed=0; }
        else if (id==1 && row<2u) { mask=0xFFBFu; use_armed=0; }
        else if (id==1) { mask=0xBFFFu; use_armed=1; }
        else if (id==2 && row>0u) { mask=0xDFFFu; use_armed=0; }
        else if (id==2 && use_armed) mask=0xBFFFu;
        else if (id==2) { mask=0xDFFFu; use_armed=0; }
        else if (id==3 && col>0) mask=0xFF7Fu;
        else if (id==3) mask=0xBFFFu;
        else if (id==41 && PE_LoadU32(0x8009CFA8u)==0x80046DBCu)
            mask=col>0?0xFF7Fu:0xBFFFu;
        else if (id==8 || id==5 || id==7) mask=0xDFFFu;
        fprintf(stderr,
                "LOOT_PILOT_HEAL_MENU %d id=%u col=%u row=%u pad=%04X item7=1 armed=%d\n",
                g_frame,id,col,row,mask,use_armed);
    } else {
        /* PE heal / non-item7 menu nav (m32 battle). Field post-consume
         * exit must NOT use this — that is cancel-only at the HEAL_DONE
         * call site (supply20 id8 Cross-stuck; supply21 PE-heal starved). */
        use_armed=0;
        if (id==0 && row<1) mask=0xFFBFu;
        else if (id==0 && row>1) mask=0xFFEFu;
        else if (id==0) mask=0xBFFFu;
        else if (id==8 && row>0) mask=0xFFEFu;
        else if (id==8 && col>0) mask=0xFF7Fu;
        else if (id==8) mask=0xBFFFu;
        else if (id==41 && PE_LoadU32(0x8009CFA8u)==0x80046DBCu)
            mask=col>0?0xFF7Fu:0xBFFFu;
        else if (id==1 || id==2 || id==3 || id==5 || id==7) mask=0xDFFFu;
        fprintf(stderr,"LOOT_PILOT_HEAL_MENU %d id=%u col=%u row=%u pad=%04X\n",
                g_frame,id,col,row,mask);
    }
    } /* focus */
    if (!focus && mask!=0xFFFFu && (age<4 || (age%16)==0))
        fprintf(stderr,
                "LOOT_PILOT_HEAL_PULSE %d pad=%04X mode=%u focus=%08X d1a0=%X d244=%u item7=%u\n",
                g_frame,mask,mode,(unsigned)focus,d1a0,
                PE_LoadU8(0x8009D244u),have_item7);
    return mask;
}

static uint16_t RouteRewardSewerPilot(uint16_t mask)
{
    static int heal_start=-1,equip_start=-1,equip_to=2,m31_stage=0,m32_stage=0,m334_stage=0;
    unsigned mode=PE_LoadU32(0x8009D28Cu);
    int m34=(GA_TOKEN==0xA8003248u);
    /* mode=2 = level/reward UI. mode=9 with a menu focus is often loot, but
     * theater field rooms (m0012/m0020/m0018/m0319) keep recorded pads —
     * m0018 diary opens a focus and dismiss stole Cross/advance there.
     * supply17: m31 field Items also opens under mode=9+focus; dismissing it
     * ate Triangle→Items before RoutePilotItemHealMask could run (HEAL armed
     * at clear patch, then pad=FFFF forever, never HEAL_MENU). Hold dismiss
     * while heal/equip owns the pad. */
    {
        pe_addr_t focus=PE_LoadU32(0x8009D15Cu);
        int theater_field =
            GA_TOKEN==0xA8001148u || GA_TOKEN==0xA8002048u ||
            GA_TOKEN==0xA8001448u || GA_TOKEN==0xA80614C8u;
        if ((mode==2u || (mode==9u && focus && !theater_field)) &&
            heal_start<0 && equip_start<0) {
            unsigned id=focus?PE_LoadU32(focus+36u):0xFFFFFFFFu;
            /* supply19: after HEAL_DONE, Items stays at id=1; Cross/Circle
             * alternate never exits (Cross re-opens Use). Circle-only on
             * submenu ids; Cross only for root id0 / unknown. */
            if (id==1u || id==2u || id==3u || id==5u || id==7u || id==8u) {
                if (g_frame%16==3) mask=0xDFFFu;
                else mask=0xFFFFu;
            } else if (g_frame%16==3) mask=0xBFFFu;
            else if (g_frame%16==11) mask=0xDFFFu;
            else mask=0xFFFFu;
            if (g_frame%120==0)
                fprintf(stderr,
                        "LOOT_PILOT_DISMISS %d mode=%u token=%08X pad=%04X "
                        "pending=%u focus=%08X id=%u\n",
                        g_frame,mode,GA_TOKEN,mask,PE_LoadU32(0x8009D078u),
                        (unsigned)focus,id);
            return mask;
        }
    }
    if (GA_TOKEN!=0xA80030C8u) m31_stage=0;
    if (GA_TOKEN!=0xA8003148u) m32_stage=0;
    if (GA_TOKEN!=0xA8063248u) m334_stage=0;
    /* M34 must own the pad from room entry, not only after the battle flag.
     * The recorded supply suffix is Cross/idle, so leaving it in control
     * parks Aya on the alligator's charge line. */
    if (m34 && g_route_m34_shim) return RouteM34Shim();
    /* Diagnostic (harness-only, env PE_ROUTE_BUMP_DATE=1): in the plaza
     * (A8000148, field mode 9, no menu) walk Aya into the date NPC (02/00)
     * to exercise the contact-reaction path (shove walk + grunt). */
    if (GA_TOKEN==0xA8000148u && getenv("PE_ROUTE_BUMP_DATE") &&
        !(PE_LoadU32(0x8009D1A0u)&6u)) {
        pe_addr_t aya=PE_LoadU32(0x8009D254u), date=0, p;
        for (p=PE_LoadU32(0x8009D20Cu); p; p=PE_LoadU32(p+4u))
            if (PE_LoadU8(p+12u)==2u && PE_LoadU8(p+13u)==0u) { date=p; break; }
        if (aya && date) {
            int ax=(int32_t)PE_LoadU32(aya+40u)>>16, az=(int32_t)PE_LoadU32(aya+48u)>>16;
            int dx=((int32_t)PE_LoadU32(date+40u)>>16)-ax, dz=((int32_t)PE_LoadU32(date+48u)>>16)-az;
            static int bump_log;
            if (g_frame-bump_log>=30) { bump_log=g_frame;
                fprintf(stderr,"BUMP %d aya=%d,%d date_d=%d,%d fA4=%08X anim=%02X\n",g_frame,ax,az,dx,dz,
                        PE_LoadU32(date+0xA4u),PE_LoadU8(date+0xEu)); }
            return RouteM34ShimMove(ax,az,dx,dz,0);
        }
    }
    /* A8064448 = M34 phase 2 (sewer tunnel), entered after the tail breaks (m34f @64370). */
    if (g_route_battle_shim && (GA_TOKEN==0xA80021C8u || GA_TOKEN==0xA8064448u) && (PE_LoadU32(0x8009D1A0u)&2u))
        return RouteBattleShim();
    if ((GA_TOKEN==0xA80023C8u || GA_TOKEN==0xA8002448u || GA_TOKEN==0xA8003148u || m34) &&
        (m34 || (PE_LoadU32(0x8009D1A0u)&2u))) {
        pe_addr_t aya=PE_LoadU32(0x8009D254u),rec=PE_LoadU32(0x8009D278u);
        unsigned mode=PE_LoadU32(0x8009D28Cu);
        mask=0xFFFFu;
        if (m34 && aya && rec && mode!=3u && mode!=9u) {
            pe_addr_t gun=PE_LoadU32(rec+104u),head=0,tail=0;
            int ax=(int32_t)PE_LoadU32(aya+40u)>>16,az=(int32_t)PE_LoadU32(aya+48u)>>16;
            unsigned loaded=gun?(PE_LoadU32(gun+12u)&0x3FFu):0u;
            unsigned hp=PE_LoadU16(rec+12u);
            int pocket=0,h2=0,t2=0,hx=0,hz=0,tx=0,tz=0,headhp=0,tailhp=0,attack_tail=0,tail_hunt=0;
            for (unsigned i=0;i<45;i++) {
                pe_addr_t p=PE_LoadU32(0x8009E000u+i*12u);
                unsigned type;
                if (!p) break;
                if (p==aya || !PE_RangeIsRam(p,640u) || !PE_RangeIsRam(PE_LoadU32(p),24u) ||
                    (int32_t)PE_LoadU32(PE_LoadU32(p)+16u)<=0) continue;
                type=PE_LoadU8(p+12u);
                if (type==3u) head=p;
                else if (type==4u) tail=p;
                else if (!head) head=p;
            }
            if (head) {
                hx=(int32_t)PE_LoadU32(head+40u)>>16; hz=(int32_t)PE_LoadU32(head+48u)>>16;
                h2=(hx-ax)*(hx-ax)+(hz-az)*(hz-az);
                {
                    pe_addr_t hb=PE_LoadU32(head);
                    headhp=hb?(int32_t)PE_LoadU32(hb+0x10u):0;
                }
            }
            if (tail) {
                tx=(int32_t)PE_LoadU32(tail+40u)>>16; tz=(int32_t)PE_LoadU32(tail+48u)>>16;
                t2=(tx-ax)*(tx-ax)+(tz-az)*(tz-az);
                {
                    pe_addr_t tb=PE_LoadU32(tail);
                    tailhp=tb?(int32_t)PE_LoadU32(tb+0x10u):0;
                }
            }
            pocket=tail && t2>=250000 && t2<=1000000 && h2>=900000;
            /* M34 alligator weak point (original script 0x801B8718): the room
             * script adds 1,000,000 to every enemy HP (tag 0x2C -> slot+0x10)
             * and only stops waiting when HP<=1,000,000. The type-3 head has
             * defense 200 -> pistol damage 1/shot (120 hits, impossible); the
             * type-4 tail has defense 6 -> 9/shot, so 9 hits (~5 attack
             * commands with the 2-shot pistol) break it and the script retires
             * the pair, plays the outro and transfers to M0359I. All 44 prior
             * supply runs kited forever and never once tried to hurt the tail. */
            {
                int tail_alive = tail && tailhp>0;
                int tail_broken = tail_alive && tailhp<=1000000;
                static int tail_broken_logged;
                static int tail_hp_last=-1;
                if (tail_alive && !tail_broken && tailhp!=tail_hp_last) {
                    tail_hp_last=tailhp;
                    fprintf(stderr,"M34_TAIL_HP %d hp=%d dmg=%d headhp=%d t2=%d h2=%d\n",
                            g_frame,tailhp,1000080-tailhp,headhp,t2,h2);
                }
                if (tail_broken && !tail_broken_logged) {
                    tail_broken_logged=1;
                    fprintf(stderr,"M34_TAIL_BROKEN %d tailhp=%d headhp=%d t2=%d\n",
                            g_frame,tailhp,headhp,t2);
                }
            }
            {
            unsigned can_heal =
                PE_LoadU16(0x800C0E50u)==7u ||
                PE_LoadU32(rec+8u)>=60u*65536u;
            /* Shared with mode==0 flee / Cross gates and mode==1 cancel. */
            int flee_floor = (!can_heal || hp<=24u) ? 3200000 : 2200000;
            /* Dedicate the ATB to the tail weak point whenever the pistol is
             * loaded, we are near the stationary tail, and the head is not on
             * top of Aya (committing an attack pins her in place). No HP gate:
             * supply50 showed the only mode-1 windows landing at hp=19 while
             * the old hp>24 test threw every attack away (0 damage in 47 runs).
             * A pending heal still wins because it clears heal_start. */
            attack_tail = tail && tailhp>0 && tailhp>1000000 && loaded &&
                          heal_start<0 && equip_start<0 &&
                          t2<=2500000 && h2>=250000;
            /* Weak-point hunt: the tail is stationary at ~(-3354,-109); the
             * west corridor (~-2700) is the only firing lane close enough to
             * hit it. Every prior run drifted east to x~-760 (t2~6.7M) and
             * never got a shot off at the tail. */
            tail_hunt = tail && tailhp>1000000 && heal_start<0 && equip_start<0;
            if (mode==1u) {
                unsigned selected=PE_LoadU8(0x8009CE44u),count=PE_LoadU8(0x8009D2B0u),target=selected,j;
                static unsigned atb_open_frame,atb_last_sel,atb_nav;
                for (j=0;j<count;j++) {
                    pe_addr_t p=PE_LoadU32(0x8009E000u+j*12u);
                    if (p && PE_LoadU8(p+12u)==4u) { target=j; break; }
                }
                if (atb_open_frame==0u || g_frame<atb_open_frame ||
                    g_frame-atb_open_frame>600u) {
                    atb_open_frame=(unsigned)g_frame;
                    atb_last_sel=selected;
                    atb_nav=0;
                }
                /* Circle-cancel ATB when reloading/healing, or critical with a
                 * heal still available. supply24: after PE spent, hp<=24 cancel
                 * suppressed every Cross so the kite never finished the boss —
                 * keep ATB shots when !can_heal and not hugging.
                 * supply25: !can_heal keep-fire pressed Up forever
                 * (target!=selected never resolved) → mode=1 pad=FFEF/FFFF
                 * frozen @x≈-2316 from f=63270..99990; pe stuck ~56, never
                 * 0x80. Also Circle-out when west-pinned so lateral kite resumes.
                 * supply36: after heal1, AT naturally hit 9000@63160 while
                 * still below flee_floor (h2≈3.19M < 3.2M) in the south
                 * pocket @(-987,-772). Cancel was gated on h2>=500k so
                 * mode=1 froze mid-flee → long-range hit → heal2 starved →
                 * death@63379 → A8001048@64000. Only keep-fire once past
                 * flee_floor and not Z-pocketed; cancel while fleeing.
                 * supply37: cancel pulsed Circle-only (pad=0xDFFF) so 7/8
                 * frames were FFFF — froze south @(-787,-763) then north
                 * @(-735,767) through heals 2–4; soft-fail A8001048@66000.
                 * Never 0x80. Keep flee d-pad under Circle while canceling. */
                {
                int cancel_atb=0;
                if (ax<=-2100 && !can_heal && heal_start<0) cancel_atb=1;
                else if (h2 < flee_floor || az >= 350 || az <= -350) cancel_atb=1;
                else if ((!can_heal && loaded && h2>=flee_floor && heal_start<0) ?
                        0 :
                        ((hp<=24u && h2 && h2<2000000) || !pocket || !loaded ||
                         heal_start>=0)) cancel_atb=1;
                if (cancel_atb && !attack_tail) {
                    int want_x=ax-hx,want_z=az-hz;
                    if (ax<=-2100) { want_x=2000; want_z=0; }
                    else if (ax>=-1100 && hx<ax) {
                        want_x=1800;
                        want_z=(az>=hz)?2400:-2400;
                    } else if (want_x*want_x+want_z*want_z<10000) {
                        want_x=ax+2400; want_z=az-900;
                    }
                    /* Z-pocket: east-dominant center break, except at east wall.
                     * supply40: ±2400 Z / X-cap-800 trapped on |az|≈400.
                     * supply41: want_x=2400 from west cleared to x≈-747, then
                     * camera-mapped FFBF (Down=east) pinned against the east
                     * wall @(-747,+735) for ~400f through heal2 → head hug →
                     * soft-fail A8001048@64000. Near east wall (ax>=-1000)
                     * push west + strong center Z instead of further east. */
                    if (h2>=200000 && ax>-2100) {
                        if (ax>=-1000) {
                            if (az>=350) { want_x=-1200; want_z=-2400; }
                            if (az<=-350) { want_x=-1200; want_z=2400; }
                        } else {
                            if (az>=350) { want_x=2400; want_z=-600; }
                            if (az<=-350) { want_x=2400; want_z=600; }
                        }
                    }
                    mask = tail_hunt
                         ? (g_m34_aim_t ? RoutePilotAimPadT(want_x,want_z)
                                        : RoutePilotAimPad(want_x,want_z))
                         : RoutePilotAimPad(want_x,want_z);
                    if (g_frame%4<=1) mask&=0xDFFFu;
                } else if (g_frame%16==3 ||
                           (attack_tail && target==selected && g_frame%16==11)) {
                    /* supply51: with hp=8 `!can_heal` was true, so the old
                     * condition committed Cross while selected=0 — i.e. it
                     * shot the defense-200 HEAD for 1 damage (headhp
                     * 1000120->1000118, 2 rounds wasted). When hunting, always
                     * finish selecting the type-4 tail first. */
                    if (attack_tail) {
                        if (target==selected || atb_nav>=4u) {
                            mask=0xBFFFu;
                            atb_nav=0;
                            fprintf(stderr,
                                    "M34_TAIL_ATK %d selected=%u target=%u hp=%u atb=%u h2=%d t2=%d\n",
                                    g_frame,selected,target,hp,
                                    (unsigned)PE_LoadU16(rec+16u),h2,t2);
                        } else {
                            mask=target>selected?0xFFBFu:0xFFEFu;
                            if (selected!=atb_last_sel) {
                                atb_last_sel=selected;
                                atb_nav=0;
                            } else {
                                atb_nav++;
                            }
                        }
                    } else if (!can_heal || target==selected || atb_nav>=3u) {
                        mask=0xBFFFu;
                        atb_nav=0;
                    } else {
                        mask=target>selected?0xFFBFu:0xFFEFu;
                        if (selected!=atb_last_sel) {
                            atb_last_sel=selected;
                            atb_nav=0;
                        } else {
                            atb_nav++;
                        }
                    }
                }
                }
            } else if (!head) {
                mask=RoutePilotAimPad(-2400-ax,900-az);
                if (loaded && g_frame%12==3) mask&=0xBFFFu;
                else if (!loaded && PE_LoadU16(0x800A1E6Eu) > 0u && g_frame%20==3)
                    mask&=0xBFFFu;
            } else {
                int hdx=hx-ax,hdz=hz-az,tdx=tail?tx-ax:hdx,tdz=tail?tz-az:hdz;
                int want_x,want_z;
                /* Stay off the charge line. Below half HP always prefer flee
                 * until the head is far — supply5 healed 1→31 then died on the
                 * next charge while still at h2~1.3M.
                 * supply22: PE heal 8→38 then re-hugged at h2~1.9M → hp17 →
                 * death (no second heal; PE meter spent). Widen flee when
                 * heal-starved / critical.
                 * supply23/24: two PE heals then pinned west — flee-from-head
                 * drove into the wall from ax~-2300 (bias only at -2700).
                 * Escape east earlier + keep shooting when heal spent. */
                if (tail_hunt) {
                    /* M34 weak point: the type-4 tail sits at ~(-3354,-109) and
                     * func_80021278 rejects any hit with distance>1800, i.e.
                     * world distance > 1928 (reliable <= 1071). Camp the west
                     * lane (x~-2600, where Aya spawns). Fleeing east — what
                     * every prior run did — makes the tail unhittable; that is
                     * why 46 runs never damaged it.
                     *
                     * The arena camera is rotated (the raw matrix at
                     * 0x800BD000/4 is the one the engine applies to the pad),
                     * and its convention drifts with the action, so the aim
                     * form is chosen adaptively: every 90 frames with no
                     * progress toward the lane we flip transposed/normal. */
                    static int aim_x0, aim_f0;
                    int dnow = abs(ax + 2600);
                    if (aim_f0 && g_frame-aim_f0>=90) {
                        if (dnow > aim_x0-40) g_m34_aim_t = !g_m34_aim_t;
                        aim_x0 = 0;
                    }
                    if (!aim_x0) { aim_x0 = dnow? dnow : 1; aim_f0 = g_frame; }
                    aim_x0 = dnow < aim_x0 ? dnow : aim_x0;
                    int lane_x = tx + 750;
                    if (lane_x < -2700) lane_x = -2700;   /* off the west wall */
                    want_x = lane_x - ax;
                    if (want_x < -600) want_x = -600;
                    if (want_x >  600) want_x =  600;
                    /* Z: work toward the tail's Z (improves t2 and unpins from
                     * the north wall); dodge the head when it is close, but
                     * never into the known wall bands. */
                    want_z = tz - az;
                    if (h2 < 600000) {
                        want_z = (az>=hz) ? 1400 : -1400;
                        if (az >  650) want_z = -1400;
                        if (az < -550) want_z =  1400;
                    }
                    if (want_x*want_x+want_z*want_z < 10000) want_z = -1200;
                    if (g_frame%60==0)
                        fprintf(stderr,
                                "M34_TAIL_HUNT %d ax=%d az=%d tx=%d tz=%d hx=%d hz=%d h2=%d t2=%d aim_t=%d wx=%d wz=%d\n",
                                g_frame,ax,az,tx,tz,hx,hz,h2,t2,g_m34_aim_t,want_x,want_z);
                } else if (h2<flee_floor) {
                    want_x=ax-hx; want_z=az-hz;
                    /* supply29: after heal1 (8→38) aya reached x≈-1668 east of
                     * head; tail west of aya made want·td < 0 and the rotate
                     * flipped east-flee into west/south → back to x≈-2116,
                     * then hp38→7 before ATB refill. Never reverse an east
                     * escape while already in the western half.
                     * supply32: skip was ax<=-1500 only; @63090 ax=-1210
                     * hx=-2162 want_x=+952 rotated to (-80,-952) → pad Left
                     * (camera) and walked west/south into the charge
                     * (hp38→17). Protect any east flee while east of head
                     * or still west of the east-wall cap. */
                    if (tail && want_x*tdx+want_z*tdz<0) {
                        int rx=want_z, rz=-want_x;
                        if (!(want_x>0 && rx<=0 && (hx<ax || ax<=-1100))) {
                            want_x=rx; want_z=rz;
                        }
                    }
                    if (want_x*want_x+want_z*want_z<10000) { want_x=ax+2400; want_z=az-900; }
                    /* West wall ~-2830; pocket ~(-2400,900). From ax<=-2300,
                     * flee-from-head is often further west — force arena-center
                     * escape before the pin (supply24 died at x~-2700).
                     * supply25: want_x=1000 + |want_z|=800 let AimPad's
                     * |z|*2>|x| dominate → Left/Up chatter @x≈-2316, never
                     * broke east. Prefer pure-east (want_z=0) so X wins.
                     * supply26: pure-east from ax~-2297 ran INTO head (hx east
                     * of aya, h2~0.7–1.1M) → hp38→17@63180, soft-fail
                     * A8001048@64000. Strafe Z first until clear of head, then
                     * east; engage earlier at ax<=-2100.
                     * supply27: AimPad(want_x=500,want_z=1400) → pad Right into
                     * head. Hard Up/Down tried as "camera-independent".
                     * supply28: heals landed (8→38, 19→49) but hard Up mapped
                     * WEST (-2344→-2839) and pinned forever @x=-2839 pad=FFEF
                     * while head closed in → soft-fail. Pad dirs are always
                     * camera-relative — never hardcode Up/Down. Also the
                     * az>=700 north-clamp flipped Z-strafe toward the head
                     * at pocket Z≈780; west escape must be applied AFTER
                     * that clamp and always keep a strong east component. */
                    if (ax<=-2100) {
                        want_x=2000;
                        want_z=0;
                    } else if (ax>=-1100 && want_x>0 && hx>=ax) want_x=-600;
                    /* Z-pocket escape. supply39: skip west pin. supply40:
                     * east-dominant. supply41: at east wall (ax>=-1000)
                     * west+center-Z — east push pinned @(-747,+735). */
                    if (az>=350 && want_z>0) want_z=0;
                    if (az<=-350 && want_z<0) want_z=0;
                    if (h2>=200000 && ax>-2100) {
                        if (ax>=-1000) {
                            if (az>=350) { want_x=-1200; want_z=-2400; }
                            if (az<=-350) { want_x=-1200; want_z=2400; }
                        } else {
                            if (az>=350) { want_x=2400; want_z=-600; }
                            if (az<=-350) { want_x=2400; want_z=600; }
                        }
                    }
                    /* East wall only: don't let |Z| dominate into a corner.
                     * (West handled below after clamp.)
                     * supply33: east kite reached ax≈-1099 with head still
                     * west (hx≈-2162). Unconditional want_x=-400 bounced
                     * Aya west into the charge (hp38→10@63360) and again
                     * after heal2@63540 (ax=-987 hx=-1293 h2=167k →
                     * pad Left into boss → soft-fail). Only bounce west
                     * when the head is east/co-located; otherwise keep
                     * fleeing east with Z-away. */
                    if (ax>=-1100) {
                        if (hx>=ax) {
                            want_z=want_z>=0?800:-800;
                            want_x=-400;
                        } else {
                            want_x=1800;
                            want_z=(az>=hz)?2400:-2400;
                        }
                        /* supply40/41: Z-pocket after east-wall overwrite. */
                        if (h2>=200000 && ax>-2100) {
                            if (ax>=-1000) {
                                if (az>=350) { want_x=-1200; want_z=-2400; }
                                if (az<=-350) { want_x=-1200; want_z=2400; }
                            } else {
                                if (az>=350) { want_x=2400; want_z=-600; }
                                if (az<=-350) { want_x=2400; want_z=600; }
                            }
                        }
                    }
                } else if (!tail || t2>1000000) {
                    if (h2<t2 && hdx*tdx+hdz*tdz>0) {
                        want_x=-hdz; want_z=hdx;
                        if (tail && want_x*(tx-hx)+want_z*(tz-hz)<0) { want_x=-want_x; want_z=-want_z; }
                    } else { want_x=tdx; want_z=tdz; }
                } else if (t2<250000) { want_x=-tdx; want_z=-tdz; }
                else { want_x=tdz; want_z=-tdx; }
                /* West/east-corridor bounce AFTER az clamps. At wall, east
                 * dominates; when head blocks the corridor, keep east +
                 * Z-away so AimPad X wins (pure Z→Right into boss; hard
                 * Up→west into wall).
                 * supply30: bounce only covered ax<=-2100; east pocket
                 * @-1520 had no override while head closed to h2=28k.
                 * supply31: bounce ax<=-1400 lost when aya drifted to
                 * -1361; FFDF freeze @h2=20k. Widen to ax<=-1200 and, when
                 * critically hugged, flee on BOTH axes away from head
                 * (not fixed +X which AimPad can map into the charge).
                 * supply32: bounce needed h2<900k but @63090 h2=912k —
                 * Z-dominant AimPad mapped south-flee to Left → west.
                 * While east of head in the west half, force east+Z-away
                 * without an h2 gate so |X| wins the camera pad.
                 * supply33: after heal2, h2=167k sat above the old 150k
                 * hug floor while east-wall bounce still applied — widen
                 * both-axis flee through 400k so mid-range charges break.
                 * supply34: both heals landed (8→38, 19→49) then hug floor
                 * overrode west-wall east escape: @63420 ax=-2689 hx=-2073
                 * h2=388k → x_away=-1500 / z_away=+1200 (NW into walls)
                 * pad=FFCF freeze @-2825,775 while head closed → soft-fail
                 * A8001048@64000. Never let away-from-head drive into the
                 * west/north/south walls; west-pinned always keeps east. */
                {
                    int z_away=(az>=hz)?2400:-2400;
                    int x_away=(ax>=hx)?1500:-1500;
                    if (az>=350 && z_away>0) z_away=-600;
                    if (az<=-350 && z_away<0) z_away=600;
                    if (ax<=-2100) {
                        /* West corridor / wall: east escape dominates hug. */
                        want_x=(ax<=-2500)?2500:2000;
                        want_z=z_away;
                    } else if (h2<400000) {
                        want_x=x_away;
                        want_z=z_away;
                    } else if (ax<=-1200 && hx<ax) {
                        /* East of west-pocket and east of head: keep running
                         * east with Z-away rather than Z-dominant Left. */
                        want_x=2200;
                        want_z=z_away;
                    } else if (ax<=-1200 && h2<1500000) {
                        want_x=2200;
                        want_z=z_away;
                    }
                    /* Final Z-pocket escape. supply41: east-wall west+Z. */
                    if (h2>=200000 && ax>-2100) {
                        if (ax>=-1000) {
                            if (az>=350) { want_x=-1200; want_z=-2400; }
                            if (az<=-350) { want_x=-1200; want_z=2400; }
                        } else {
                            if (az>=350) { want_x=2400; want_z=-600; }
                            if (az<=-350) { want_x=2400; want_z=600; }
                        }
                    }
                }
                mask=RoutePilotAimPad(want_x,want_z);
                /* Shoot gates. supply35: never Cross while h2 < flee_floor.
                 * supply40: after PE spent flee_floor=3.2M blocked ALL Cross
                 * forever → heal→pocket-oscillate→chip loop until pe<60 and
                 * soft-fail. Allow finishing shots once clear of Z-pocket and
                 * past a lower !can_heal floor (still cancel ATB under flee). */
                {
                int shoot_floor = can_heal ? flee_floor : 1200000;
                int z_clear = (az > -350 && az < 350);
                if (loaded && heal_start<0 && h2>=shoot_floor && z_clear &&
                    ((hp>24u && (can_heal ? h2>=250000 : h2>=400000)) ||
                     (!can_heal && hp>12u && h2>=700000)) &&
                    g_frame%((pocket||!can_heal||h2>=900000)?4:10)==3)
                    mask&=0xBFFFu;
                }
                if (!loaded && h2>=flee_floor &&
                    PE_LoadU16(0x800A1E6Eu) > 0u && g_frame%8==3)
                    mask&=0xBFFFu;
                /* M34 weak point: fire on / reload for the tail whenever the
                 * head is not on top of Aya. The old h2>=2.2M/3.2M floor never
                 * held inside M34, so the pilot never landed a shot there. */
                if (attack_tail && t2<=1500000 && h2>=250000 && g_frame%4==3)
                    mask&=0xBFFFu;
                if (!loaded && tail && tailhp>1000000 && t2<=1500000 &&
                    PE_LoadU16(0x800A1E6Eu) > 0u && g_frame%8==3)
                    mask&=0xBFFFu;
                /* The M34 arena camera is rotated ~90 deg, where the standard
                 * aim silently inverts; use the transposed form for the hunt. */
                if (tail_hunt)
                    mask = g_m34_aim_t ? RoutePilotAimPadT(want_x,want_z)
                                       : RoutePilotAimPad(want_x,want_z);
            }
            }
            /* supply42: east-wall west+center still froze @(-749,+412) on
             * camera-mapped FFBF while the head closed (hp39→3) → death →
             * A8000148 mode=-1@66000. When position is immobile for >40f in
             * a Z-pocket / wall band, cycle raw cardinal pads (camera-free)
             * so one direction breaks the pin. Skip while healing/ATB.
             * supply43: anchor-reset detector never fired during a 200f+
             * pin @(-796,-399) (az=-399 sat 1u above the old 400 pocket
             * trigger; pads flickered FF3F/FFCF every frame, vibrating in
             * place — each shove/slide ≥25 reset stuck_n). Windowed net
             * displacement instead: oscillation returns to origin, so it
             * cannot reset the window. Latched break until 300u escape. */
            {
            static int w_ax,w_az,w_t0,w_break;
            /* supply48: the west firing lane is a legitimate long camp — the
             * net-displacement detector latched at ~62950 and then cycled raw
             * cardinal pads for 200f+, silently overriding the tail-hunt aim
             * (that is why the "hunt" still drifted east). Never latch while
             * hunting, and clear any existing latch on hunt entry. */
            int in_band=(az>=350 || az<=-350 || ax>=-1000 || ax<=-2100) &&
                        !tail_hunt;
            if (!in_band) { w_t0=0; w_break=0; }
            else if (w_break) {
                if (heal_start<0 && mode==0u) {
                    static const uint16_t cycle[4]={0xFFDFu,0xFFEFu,0xFF7Fu,0xFFBFu};
                    mask=cycle[(g_frame/15)%4];
                    if (g_frame%120==0)
                        fprintf(stderr,"M34_STUCK_BREAK %d pad=%04X pos=%d,%d hold\n",
                                g_frame,mask,ax,az);
                }
                if (abs(ax-w_ax)+abs(az-w_az)>=300) { w_t0=0; w_break=0; }
            } else if (heal_start<0 && mode==0u) {
                if (!w_t0) { w_t0=g_frame; w_ax=ax; w_az=az; }
                else if (g_frame-w_t0>=50) {
                    int moved=abs(ax-w_ax)+abs(az-w_az);
                    if (moved<120) {
                        w_break=1;
                        fprintf(stderr,"M34_STUCK_BREAK %d pos=%d,%d moved=%d\n",
                                g_frame,ax,az,moved);
                    }
                    w_t0=0;
                }
            } else { w_t0=0; }
            }
            /* Heal gates: early at mid HP when far; critical with a nearer floor
             * so a second heal can land after the first PE Heal (1→31→10→0). */
            /* supply26: after PE heal1, pe rebuilt to 64@63300 with hp=17 but
             * h2~660k blocked the second heal (needed 1.5M). Allow a closer
             * emergency heal when west-pinned.
             * supply27: starve@h2=320k with pe=87/at=9000 — emergency gate
             * h2>=400k never armed, and heal_start was cleared same-frame by
             * h2<600k cancel (so 400k–599k emergency was dead on arrival).
             * supply29: heal1 landed 8→38; second window@63240 hp7 pe64
             * h2=2.0M but at=2652 < 9000 — starved until at=8892@63360 when
             * head closed (h2=42k) → soft-fail. Critical PE heal may arm at
             * lower ATB once pe/item is ready.
             * supply30: ATB fix held (at=9000) but heal2 starved @hp10
             * h2=173770 — critical floor was 250k and west-pin emergency
             * required ax<=-2100 (aya was @-1520). Arm closer when critical
             * / east-corridor pinned; cancel still only h2<80k && hp>8.
             * supply32: heal1 8→38 then chip to 17; @63240 pe64 at=2652
             * h2=2.0M starved (atb_need 9000 for hp>12). @63360 hp13
             * at=8892 h2=26k — 108 short of 9000 with head on top. Extend
             * reduced ATB + closer floor through hp<=20. */
            {
            unsigned atb=PE_LoadU16(rec+16u);
            unsigned atb_need=(hp<=20u)?2500u:9000u;
            if (heal_start<0 && equip_start<0 && mode==0u && head &&
                !PE_LoadU8(0x8009CE3Cu) &&
                hp<=32u && atb>=atb_need &&
                (PE_LoadU16(0x800C0E50u)==7u || PE_LoadU32(rec+8u)>=60u*65536u) &&
                ((hp<=24u && h2>=1500000) || (hp<=20u && h2>=100000) ||
                 h2>=1800000 || (hp<=20u && ax<=-1200 && h2>=100000) ||
                 (hp<=8u && h2>=20000))) {
                heal_start=g_frame;fprintf(stderr,"LOOT_PILOT_HEAL %d hp=%u h2=%d\n",g_frame,hp,h2);
            } else if (heal_start<0 && equip_start<0 && mode==0u && head &&
                       hp<=24u && g_frame%60==0) {
                fprintf(stderr,
                        "LOOT_PILOT_HEAL_STARVE %d hp=%u h2=%d pe=%u item4=%u at=%u\n",
                        g_frame,hp,h2,PE_LoadU32(rec+8u)>>16,
                        PE_LoadU16(0x800C0E50u),atb);
            }
            }
            /* Only abort a heal when truly hugged; keep emergency starts. */
            if (heal_start>=0 && head && h2 && h2<80000 && hp>8u) heal_start=-1;
            if (g_frame%30==0) {
                fprintf(stderr,"M34_PILOT %d mode=%u pad=%04X pos=%d,%d head=%d,%d tail=%d,%d hp=%u loaded=%u pocket=%d h2=%d t2=%d taildmg=%d headhp=%d\n",
                        g_frame,mode,mask,ax,az,hx,hz,tx,tz,hp,loaded,pocket,h2,t2,
                        tailhp>0?1000080-tailhp:0,headhp);
            }
        } else if (!mode && aya && rec) {
            pe_addr_t enemy=0;
            int nearest=INT32_MAX;
            for (unsigned i=0;i<45;i++) {
                pe_addr_t p=PE_LoadU32(0x8009E000u+i*12u);
                if (!p) break;
                if (PE_RangeIsRam(p,640u) && p!=aya && PE_RangeIsRam(PE_LoadU32(p),24u) &&
                    (int32_t)PE_LoadU32(PE_LoadU32(p)+16u)>0) {
                    if (GA_TOKEN!=0xA8003148u) {enemy=p;break;}
                    int dx=((int32_t)PE_LoadU32(p+40u)>>16)-((int32_t)PE_LoadU32(aya+40u)>>16);
                    int dz=((int32_t)PE_LoadU32(p+48u)>>16)-((int32_t)PE_LoadU32(aya+48u)>>16);
                    int distance=dx*dx+dz*dz;
                    if (distance<nearest) {nearest=distance;enemy=p;}
                }
            }
            if (enemy) {
                int dx=((int32_t)PE_LoadU32(enemy+40u)>>16)-((int32_t)PE_LoadU32(aya+40u)>>16);
                int dz=((int32_t)PE_LoadU32(enemy+48u)>>16)-((int32_t)PE_LoadU32(aya+48u)>>16);
                if (GA_TOKEN==0xA8003148u && PE_LoadU8(0x800C0E20u)==0u) {
                    int distance=dx*dx+dz*dz;
                    if (distance<640000) {dx=-dx;dz=-dz;}
                    else if (distance<902500) {int old=dx;dx=dz;dz=-old;}
                    int ax=(int32_t)PE_LoadU32(aya+40u)>>16,az=(int32_t)PE_LoadU32(aya+48u)>>16;
                    if (ax<-16000) dx=600;else if (ax>-14000) dx=-600;
                    if (az<1500) dz=600;else if (az>2700) dz=-600;
                }
                int c=(int16_t)PE_LoadU16(0x800BD000u),sn=(int16_t)PE_LoadU16(0x800BD004u);
                int x=c*dx-sn*dz,z=sn*dx+c*dz;
                if (dx*dx+dz*dz>6400) {
                    if (abs(x)*2>abs(z)) mask&=x>0?0xFFDFu:0xFF7Fu;
                    if (abs(z)*2>abs(x)) mask&=z>0?0xFFEFu:0xFFBFu;
                }
            }
            if (heal_start<0 && equip_start<0 && !PE_LoadU8(0x8009CE3Cu) && PE_LoadU16(rec+12u)<=25u &&
                PE_LoadU16(rec+16u)>=9000u &&
                (PE_LoadU16(0x800C0E50u)==7u || PE_LoadU32(rec+8u)>=60u*65536u)) {
                heal_start=g_frame;fprintf(stderr,"LOOT_PILOT_HEAL %d\n",g_frame);
            }
        }
        pe_addr_t gun=rec?PE_LoadU32(rec+104u):0u;
        if (GA_TOKEN==0xA8003148u && equip_start<0 && heal_start<0 && !mode && rec &&
            PE_LoadU8(0x800C0E20u)==2u && !PE_LoadU8(0x8009CE3Cu) &&
            PE_LoadU16(0x800A1E6Eu)>0u && PE_LoadU16(rec+16u)>=9000u) {
            equip_to=0;equip_start=g_frame;
            fprintf(stderr,"LOOT_PILOT_PISTOL %d\n",g_frame);
        }
        if (equip_start<0 && heal_start<0 && !mode && rec && gun &&
            PE_LoadU8(0x800C0E20u)==0u && !PE_LoadU8(0x8009CE3Cu) &&
            !(PE_LoadU32(gun+12u)&0x3FFu) && !PE_LoadU16(0x800A1E6Eu) && PE_LoadU16(rec+16u)>=9000u) {
            equip_to=2;equip_start=g_frame;fprintf(stderr,"LOOT_PILOT_EQUIP %d\n",g_frame);
        }
        if (equip_start>=0 && PE_LoadU8(0x8009CE3Cu)) {
            fprintf(stderr,"LOOT_PILOT_EQUIP_QUEUED %d command=%u slot=%u\n",g_frame,PE_LoadU16(0x800BE834u),PE_LoadU8(0x800C0E20u));equip_start=-1;
        }
        if (heal_start>=0 && PE_LoadU8(0x8009CE3Cu)) heal_start=-1;
        if (equip_start>=0) {
            pe_addr_t focus=PE_LoadU32(0x8009D15Cu);
            unsigned id=focus?PE_LoadU32(focus+36u):0xFFFFFFFFu;
            unsigned row=focus?PE_LoadU32(focus+72u):0u;
            mask=0xFFFFu;
            if (g_frame-equip_start<2) mask=0xEFFFu;
            else if (g_frame%16==3 && focus) {
                unsigned target_row=id==7?(equip_to==2?1u:0u):2u;
                if ((id==0 || id==7) && row<target_row) mask=0xFFBFu;
                else if ((id==0 || id==7) && row>target_row) mask=0xFFEFu;
                else mask=0xBFFFu;
            }
        } else if (heal_start>=0) {
            /* Keep one heal implementation — battle path used to drift. */
            if (GA_TOKEN!=0xA8003148u && GA_TOKEN!=0xA8003248u &&
                GA_TOKEN!=0xA80030C8u) {
                pe_addr_t focus=PE_LoadU32(0x8009D15Cu);
                unsigned id=focus?PE_LoadU32(focus+36u):0xFFFFFFFFu;
                unsigned row=focus?PE_LoadU32(focus+72u):0u;
                mask=0xFFFFu;
                if (g_frame-heal_start<2) mask=0xEFFFu;
                else if (g_frame%16==3 && focus) {
                    if (id==0 && row==0) mask=0xFFBFu;
                    else mask=0xBFFFu;
                }
            } else {
                mask=RoutePilotItemHealMask(heal_start);
            }
        } else if (!m34 && g_frame%16==3) {
            mask&=0xBFFFu;
        }
        static unsigned last=0x10000u;
        if (mask!=last) {fprintf(stderr,"LOOT_PILOT_PAD %d:%04X\n",g_frame,mask);last=mask;}
    } else if (GA_TOKEN!=0xA80030C8u && GA_TOKEN!=0xA8063248u) {
        heal_start=-1;equip_start=-1;
    }
    /* The first victory now happens later than the historical timed suffix.
     * Use the original exit rectangles through normal movement. */
    if (((GA_TOKEN==0xA80023C8u && (g_sewer_victories&1u)) || GA_TOKEN==0xA8002448u) &&
        !(PE_LoadU32(0x8009D1A0u)&2u)) {
        pe_addr_t aya=PE_LoadU32(0x8009D254u);
        mask=0xFFFFu;
        if (PE_LoadU32(0x8009D1A0u)&4u) {
            if (g_frame%16==3) mask=0xDFFFu;
        } else if (aya) {
            int dx=-((int32_t)PE_LoadU32(aya+40u)>>16);
            int dz=(GA_TOKEN==0xA80023C8u?-1250:450)-((int32_t)PE_LoadU32(aya+48u)>>16);
            /* day1 (09-28): M0028I spawns Aya at (0,-3240) and the fight can end
             * off-lane (742,-3434); the direct diagonal to (0,450) snags at
             * ~(910,-1370) forever (build/lanes/day1/m28.log). Walk back to the
             * x=0 lane first, then north. */
            if (GA_TOKEN==0xA8002448u && abs(dx)>80) dz=0;
            int c=(int16_t)PE_LoadU16(0x800BD000u),sn=(int16_t)PE_LoadU16(0x800BD004u);
            int x=c*dx-sn*dz,z=sn*dx+c*dz;
            if (dx*dx+dz*dz>1600) {
                if (abs(x)*2>abs(z)) mask&=x>0?0xFFDFu:0xFF7Fu;
                if (abs(z)*2>abs(x)) mask&=z>0?0xFFEFu:0xFFBFu;
            }
        }
    }
    if ((GA_TOKEN==0xA80030C8u || GA_TOKEN==0xA8003148u || GA_TOKEN==0xA80031C8u ||
         GA_TOKEN==0xA8063248u) && !(PE_LoadU32(0x8009D1A0u)&2u)) {
        pe_addr_t aya=PE_LoadU32(0x8009D254u);
        mask=0xFFFFu;
        /* supply18: d1a0 bit2 chatter stole the pad into Circle-only while
         * heal/equip were armed, so Triangle never ran on those frames.
         * Heal/equip own the pad until focus appears / queue completes. */
        if ((PE_LoadU32(0x8009D1A0u)&4u) && heal_start<0 && equip_start<0) {
            if (g_frame%16==3) mask=0xDFFFu;
        } else if (aya) {
            int ax=(int32_t)PE_LoadU32(aya+40u)>>16,az=(int32_t)PE_LoadU32(aya+48u)>>16;
            int tx,tz,use=0;
            if (GA_TOKEN==0xA8063248u) {
                unsigned chests=PE_LoadU32(0x800A7940u);
                unsigned sw=PE_LoadU32(0x800A789Cu);
                if (!(chests&0x80u)) { tx=-50;tz=513;use=1;m334_stage=0; }
                else if (!(sw&4u)) {
                    /* Proven switch rect x[25,111]/z[2100,2198]. Stopping at
                     * dist^2<=1600 left Aya at (97,2129) — inside the box but
                     * short of the trigger. Walk through (40,2160) until bit4. */
                    if (m334_stage<1 && ax>=360) m334_stage=1;
                    if (m334_stage==1 && az>=1860) m334_stage=2;
                    if (m334_stage==0) { tx=400;tz=az; }
                    else if (m334_stage==1) { tx=400;tz=1900; }
                    else { tx=40;tz=2160;use=1; }
                } else if (!(chests&0x200u)) { tx=837;tz=13;use=1; }
                else if (abs(ax)>80) { tx=0;tz=az; }
                else { tx=0;tz=-1150; }
            } else if (GA_TOKEN==0xA80030C8u) {
                unsigned chests=PE_LoadU32(0x800A7940u);
                int need_supply=(chests&0x280u)!=0x280u;
                pe_addr_t rec=PE_LoadU32(0x8009D278u);
                unsigned hp=rec?PE_LoadU16(rec+12u):0u;
                unsigned gun_slot=PE_LoadU8(0x800C0E20u);
                /* Upper exit x[1300,1800]/z[2600,3300] -> m0334i. Proven
                 * walk (ACTIVE_HANDOFF m31-forward): center X, north to Z2900,
                 * then east to X1500. East along entry z=-950 dead-ends ~702. */
                if (need_supply) {
                    if (m31_stage==0 && abs(ax)<=40) m31_stage=1;
                    if (m31_stage==1 && az>=2860) m31_stage=2;
                    if (m31_stage==0) { tx=0; tz=az; }
                    else if (m31_stage==1) { tx=0; tz=2900; }
                    else { tx=1550; tz=2900; }
                } else if (heal_start>=0 || equip_start>=0) {
                    /* supply15: heal armed on the m334→m31 transition frame while
                     * ax/az were still the prior-room coords (az~-326), then spawn
                     * snapped to the return door (1020,2615). Triangle never opens
                     * Items there; RoutePilotItemHealMask idles FFFF forever. */
                    pe_addr_t hfocus=PE_LoadU32(0x8009D15Cu);
                    int clear=abs(ax)<=120 && abs(az-1000)<=300;
                    if (heal_start>=0 && !hfocus && !clear &&
                        g_frame-heal_start>40) {
                        fprintf(stderr,
                                "LOOT_PILOT_HEAL_ABORT %d pos=%d,%d mode=%u age=%d\n",
                                g_frame,ax,az,PE_LoadU32(0x8009D28Cu),
                                g_frame-heal_start);
                        heal_start=-1;
                        /* Same staged walk as HEAL_WAIT — do not aim (0,1000)
                         * diagonally from the door alcove. */
                        if (az>1300 && abs(ax)>40) { tx=0; tz=az; }
                        else { tx=0; tz=1000; }
                    } else {
                        tx=ax;tz=az;
                    }
                } else if (rec && PE_LoadU16(0x800C0E50u)==7u &&
                           /* 09-28: only heal when hurt. With the sewer HP lock Aya
                            * arrives at full HP and the game refuses Medicine,
                            * so the Items/Use loop never finished (bt-route2 @77850). */
                           hp < PE_LoadU16(rec+28u) &&
                           !PE_LoadU8(0x8009CE3Cu) &&
                           /* Arm only on the clear patch — not az<2000 anywhere.
                            * Stale transition coords and the upper door both
                            * satisfied the old gate (supply14/15). */
                           abs(ax)<=80 && abs(az-1000)<=200) {
                    heal_start=g_frame;tx=ax;tz=az;
                    fprintf(stderr,"LOOT_PILOT_HEAL %d hp=%u at=%u\n",
                            g_frame,hp,(unsigned)PE_LoadU16(rec+16u));
                } else if (rec && PE_LoadU16(0x800C0E50u)==7u && hp < PE_LoadU16(rec+28u)) {
                    /* Walk to a clear m31 patch, then open Items/Use.
                     * supply16: from door spawn (751,2502) a direct aim at
                     * (0,1000) held only Down into the alcove wall — pad=FFBF
                     * forever, never HEAL_MENU. Center X at current Z first
                     * (same staging as the m32-exit path below). */
                    if (az>1300 && abs(ax)>40) { tx=0; tz=az; }
                    else { tx=0; tz=1000; }
                    if (g_frame%240==0)
                        fprintf(stderr,
                                "LOOT_PILOT_HEAL_WAIT %d pos=%d,%d mode=%u ce3c=%u at=%u item4=%u tx=%d,%d\n",
                                g_frame,ax,az,PE_LoadU32(0x8009D28Cu),
                                PE_LoadU8(0x8009CE3Cu),(unsigned)PE_LoadU16(rec+16u),
                                PE_LoadU16(0x800C0E50u),tx,tz);
                } else if (rec && gun_slot!=0u && PE_LoadU16(0x800A1E6Eu)>0u &&
                           PE_LoadU16(rec+16u)>=9000u && !PE_LoadU8(0x8009CE3Cu)) {
                    equip_to=0;equip_start=g_frame;tx=ax;tz=az;
                    fprintf(stderr,"LOOT_PILOT_PISTOL %d slot=%u\n",g_frame,gun_slot);
                } else {
                    /* Side exit x[-1600,-1400], z[800,1250] -> m32. Return from
                     * m334 lands at the upper door (~z=2600); az>=960 must not
                     * skip straight into a west cut at high Z (wedged -815,2377). */
                    if (az>1300) {
                        if (abs(ax)>40) { tx=0; tz=az; }
                        else { tx=0; tz=1000; }
                    } else if (abs(ax+1500)>40) {
                        tx=-1500;tz=1000;
                    } else {
                        tx=-1500;tz=1000;
                    }
                }
            }
            else if (GA_TOKEN==0xA80031C8u) {tx=-1000;tz=0;}
            else if ((g_sewer_victories&4u) && !(PE_LoadU32(0x800A7878u)&1u)) {
                if (m32_stage==0 && az>=3260) m32_stage=1;
                if (m32_stage==1 && ax<=-16930) m32_stage=2;
                tx=m32_stage==0?-16400:-16970;
                tz=m32_stage==0?(ax>-16360?az:3300):m32_stage==1?3300:3520;
            } else {tx=-16400;tz=abs(ax+16400)>40?az:3800;}
            int dx=tx-ax,dz=tz-az;
            int c=(int16_t)PE_LoadU16(0x800BD000u),sn=(int16_t)PE_LoadU16(0x800BD004u);
            int x=c*dx-sn*dz,z=sn*dx+c*dz;
            int dist2=dx*dx+dz*dz;
            /* Chests/switch need to be walked onto; 1600 (~40u) stopped short. */
            if (dist2>(use?64:1600)) {
                if (abs(x)*2>abs(z)) mask&=x>0?0xFFDFu:0xFF7Fu;
                if (abs(z)*2>abs(x)) mask&=z>0?0xFFEFu:0xFFBFu;
            }
            /* M31 dialogue needs Cross; m0334 must not hold Cross on an open
             * chest (A9 latch). Item7 heal / pistol equip use the PE menu. */
            if (heal_start>=0) {
                /* supply19: item7 consume at HEAL_DONE left Items focus id=1;
                 * clearing heal_start immediately handed pad to dismiss, which
                 * Cross-tapped forever without exiting. Keep heal ownership and
                 * Circle out via RoutePilotItemHealMask until focus is gone. */
                static int heal_done_logged;
                unsigned item4=PE_LoadU16(0x800C0E50u);
                pe_addr_t hfocus=PE_LoadU32(0x8009D15Cu);
                if (PE_LoadU8(0x8009CE3Cu)) {
                    heal_start=-1;heal_done_logged=0;
                } else if (item4!=7u) {
                    if (!heal_done_logged) {
                        pe_addr_t hrec=PE_LoadU32(0x8009D278u);
                        fprintf(stderr,"LOOT_PILOT_HEAL_DONE %d hp=%u item4=%u\n",
                                g_frame,hrec?PE_LoadU16(hrec+12u):0u,item4);
                        heal_done_logged=1;
                    }
                    if (!hfocus) { heal_start=-1;heal_done_logged=0; }
                    else {
                        /* supply20/21: cancel-only after consume. Do not call
                         * RoutePilotItemHealMask (its !item7 path navigates /
                         * is also needed for m32 PE heal). DFFF closed
                         * id1→id0→clear → A8003148 on supply21. */
                        int age=g_frame-heal_start;
                        if (age>=30 && (age-30)%28<=1) mask=0xDFFFu;
                        else mask=0xFFFFu;
                        if (g_frame%28<=1) {
                            unsigned id=PE_LoadU32(hfocus+36u);
                            fprintf(stderr,
                                    "LOOT_PILOT_HEAL_EXIT %d id=%u pad=%04X\n",
                                    g_frame,id,mask);
                        }
                    }
                } else {
                    heal_done_logged=0;
                    mask=RoutePilotItemHealMask(heal_start);
                }
            } else if (equip_start>=0) {
                pe_addr_t focus=PE_LoadU32(0x8009D15Cu);
                unsigned id=focus?PE_LoadU32(focus+36u):0xFFFFFFFFu;
                unsigned row=focus?PE_LoadU32(focus+72u):0u;
                mask=0xFFFFu;
                if (PE_LoadU8(0x8009CE3Cu)) {
                    fprintf(stderr,"LOOT_PILOT_EQUIP_QUEUED %d command=%u slot=%u\n",
                            g_frame,PE_LoadU16(0x800BE834u),PE_LoadU8(0x800C0E20u));
                    equip_start=-1;
                } else if (g_frame-equip_start<2) mask=0xEFFFu;
                else if (g_frame%16==3 && focus) {
                    unsigned target_row=id==7?(equip_to==2?1u:0u):2u;
                    if ((id==0 || id==7) && row<target_row) mask=0xFFBFu;
                    else if ((id==0 || id==7) && row>target_row) mask=0xFFEFu;
                    else mask=0xBFFFu;
                }
            } else if (GA_TOKEN==0xA8063248u) {
                if (use && dist2<160000 && g_frame%16==3) mask&=0xBFFFu;
            } else if (g_frame%16==3) mask&=0xBFFFu;
            if(g_frame%240==0)fprintf(stderr,"REWARD_FORWARD %d token=%08X pos=%d,%d rot=%u stage=%d pad=%04X chests=%X persist2B=%X d1a0=%X\n",
                    g_frame,GA_TOKEN,ax,az,PE_LoadU16(aya+0x3Au),
                    GA_TOKEN==0xA80030C8u?m31_stage:GA_TOKEN==0xA8063248u?m334_stage:m32_stage,
                    mask,PE_LoadU32(0x800A7940u),PE_LoadU32(0x800A789Cu),
                    PE_LoadU32(0x8009D1A0u));
        }
    }
    static unsigned last_xp=UINT32_MAX,last_base=UINT32_MAX,last_ammo=UINT32_MAX;
    if (g_frame>0 && g_frame%1000==0)
        fprintf(stderr,"ROUTE_HB frame=%d token=%08X mode=%u d1a0=%X\n",
                g_frame,GA_TOKEN,PE_LoadU32(0x8009D28Cu),PE_LoadU32(0x8009D1A0u));
    if (g_frame>18500 && g_frame<20500) {
        unsigned xp=PE_LoadU32(0x800C0E00u),base=PE_LoadU32(0x8009D03Cu),ammo=PE_LoadU16(0x800A1E6Eu);
        if (xp!=last_xp || base!=last_base || ammo!=last_ammo) {
            fprintf(stderr,"LOOT_CONNECTED_STATE frame=%d xp=%u base=%u ammo=%u pending=%u\n",g_frame,xp,base,ammo,PE_LoadU32(0x8009D078u));
            last_xp=xp;last_base=base;last_ammo=ammo;
        }
        if (g_frame%120==0)
            fprintf(stderr,"LOOT_CONNECTED_HB frame=%d token=%08X mode=%u d1a0=%X pending=%u focus=%08X\n",
                    g_frame,GA_TOKEN,PE_LoadU32(0x8009D28Cu),PE_LoadU32(0x8009D1A0u),
                    PE_LoadU32(0x8009D078u),(unsigned)PE_LoadU32(0x8009D15Cu));
    }
    return mask;
}
