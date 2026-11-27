/* Vector check for callers, from pe_cd_leaf_vectors.py modes:
 *   modetype  SpuSetReverbModeType  pe_plat_spu_modetype.c
 *   errmath   _err_math             pe_plat_math_err.c
 *   fidma     _spu_FiDMA            pe_plat_spu_fidma.c
 *   cardclear _card_clear           pe_plat_card_clear.c
 *   qsort     qsort                 pe_plat_qsort.c (+ pe_plat_libc.c, the
 *             swap provider; its unrelated references stay unresolved)
 *   cdgen     seven generated libcd/libds callers  pe_plat_cd_gen.c
 *   cdgen2    four more                            pe_plat_cd_gen2.c
 *   cdgen3    DsFlush / 0x80081268  pe_plat_cd_gen3.c / pe_plat_cd_gen4.c
 *   cdgen5    six libcd/libds + three libgpu rows  pe_plat_cd_gen5.c / pe_plat_gpu_gen.c
 *   cdgen6    text_130 (frame-pointer arguments)   pe_plat_cd_gen6.c
 *   cdgen7    text_13CC (CD data callback)         pe_plat_cd_gen7.c
 *   cdgen8    0x800840DC (libcard slot callback)   pe_plat_card_gen8.c
 *   cdgen9    text_2700 0x80077144 (ResetGraph core) pe_plat_gpu_gen9.c
 *   cdgen10   DS_status 0x800809E0 (TTY printout)   pe_plat_ds_gen10.c
 *   cdgen11   0x80080F98 / 0x8008214C (libds callback words) pe_plat_ds_gen11.c
 *   cdgen12   SetGraphDebug 0x80074BB8 (log vector)  pe_plat_gpu_gen12.c
 *   cdgen13   InitCARD / StartCARD 0x8007DDD4 / 0x8007DE40 pe_plat_card_gen13.c
 *   cdgen14   slot callers 0x80082B08 / 0x80083DF0    pe_plat_card_gen14.c
 *   cdgen15   leaves 0x800719C4 / 0x8007E4E0           pe_plat_gpu_gen15.c / pe_plat_card_gen15.c
 *   cdgen16   libds text_904 0x80081D18 (read + poll)  pe_plat_ds_gen16.c
 *   cdgen17   _dirFailAuto 0x80084EB0 (record-reset slot) pe_plat_card_gen17.c
 *   cdgen18   text_34 0x80084B78 (card-object dispatch)  pe_plat_pad_gen18.c
 *   cdgen19   firstfile 0x800727B4 / text_19C 0x80072950 (device table) pe_plat_api_gen19.c
 *   cdgen20   card-object methods 0x80083F04 / 0x80083F44 / 0x8008401C pe_plat_card_gen20.c
 *   cdgen21   SIO wait 0x800834E8 (SIO_STAT bit 7 poll)  pe_plat_card_gen21.c
 *   cdgen22   SIO exchange 0x800830DC / write 0x800832B4    pe_plat_card_gen22.c
 *   cdgen23   libpad padif transfer 0x80084168              pe_plat_pad_gen23.c
 *   cdgen24   libpad text_770 0x80083014 (card transfer step) pe_plat_pad_gen24.c
 *   cdgen25   libcard MemCard_TimerCallback 0x80082B70          pe_plat_card_gen25.c
 *   cdgen26   libpad text_108 0x80084C4C (record step)        pe_plat_pad_gen26.c
 *   cdgen28   libds text_FC 0x80081E70 (DS ready callback)    pe_plat_ds_gen28.c
 *   cdgen29   libcd CD_getsector2 0x8007C044 (status wait, DMA3 start) pe_plat_cd_gen29.c
 *   cdgen30   libds text_360 0x8007E8F4 (pending-command dispatch) pe_plat_ds_gen30.c
 *   cdgen31   libds text_5F4 0x8007EB88 (command-queue push)   pe_plat_ds_gen31.c
 *   cdgen32   libcd StSetStream 0x8007C304 (stream setup)   pe_plat_cd_gen32.c
 *   cdgen33   libds DsDataCallback 0x800824F0 / DsSystemStatus 0x8007F72C, libcd text_1688 0x8007C13C
 *             pe_plat_cd_gen33.c
 *   cdgen34   libpad PadStartCom 0x80082CF0 (communication start)  pe_plat_pad_gen34.c
 *   cdgen38   libds DsGetDiskType 0x80082314                  pe_plat_ds_gen38.c
 *   cdgen39   libds text_170 0x8007E704 (queue flush)          pe_plat_ds_gen39.c
 *   cdgen39   libds text_3D0 0x8007E964 (command complete / error) pe_plat_ds_gen39.c
 *   retry     CD command retry (800800F4), pe_plat_cd_retry.c
 *   poll      CD read poll retail core (800811E4), pe_plat_cd_poll.c
 * Each build line links the checker with some native files and passes
 * CHK_HAVE_<FILE> for each one: call() cases are compiled only for linked
 * natives, and a stub for a bound callee is compiled only when the file that
 * defines that callee is not linked (#ifndef CHK_HAVE_<FILE>).  Run from the
 * repo root, one line at a time:
 *   cc -std=c99 -Wall -Wextra -Werror -DCHK_HAVE_SPU_MODETYPE -DCHK_HAVE_MATH_ERR -DCHK_HAVE_SPU_FIDMA -DCHK_HAVE_CARD_CLEAR -DCHK_HAVE_QSORT -DCHK_HAVE_CD_GEN -Dfunc_8007DDC4=chk_new_card -Dfunc_8007DDB4=chk_card_write -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_spu_caller_check.c pc_port/platform/pe_plat_spu_modetype.c pc_port/platform/pe_plat_math_err.c pc_port/platform/pe_plat_spu_fidma.c pc_port/platform/pe_plat_card_clear.c pc_port/platform/pe_plat_qsort.c pc_port/platform/pe_plat_libc.c pc_port/platform/pe_plat_cd_gen.c -Wl,--unresolved-symbols=ignore-all -o check
 *     (modetype v3, errmath, fidma v2, cardclear, qsort, cdgen v2 vectors)
 *   cc -std=c99 -Wall -Wextra -Werror -DCHK_HAVE_CD_GEN2 -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_spu_caller_check.c pc_port/platform/pe_plat_cd_gen2.c -o check
 *     (cdgen2 v3 vectors)
 *   cc -std=c99 -Wall -Wextra -Werror -DCHK_HAVE_CD_GEN3 -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_spu_caller_check.c pc_port/platform/pe_plat_cd_gen3.c -o check
 *     (cdgen3 vectors, DsFlush cases: grep '^8007F69C ' vectors.txt > v_flush.txt)
 *   cc -std=c99 -Wall -Wextra -Werror -DCHK_HAVE_CD_GEN4 -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_spu_caller_check.c pc_port/platform/pe_plat_cd_gen4.c -o check
 *     (cdgen3 vectors, 0x80081268 cases: grep '^80081268 ' vectors.txt > v_81268.txt)
 *   cc -std=c99 -Wall -Wextra -Werror -DCHK_HAVE_CD_GEN5 -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_spu_caller_check.c pc_port/platform/pe_plat_cd_gen5.c -o check
 *     (cdgen5 vectors, libcd/libds cases:
 *      grep -E '^(8007FB44|8007FCBC|80080CDC|8008227C|80082204|8007F608) ' vectors.txt > v_cd.txt)
 *   cc -std=c99 -Wall -Wextra -Werror -DCHK_HAVE_GPU_GEN -Dfunc_80076C34=chk_76c34 -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_spu_caller_check.c pc_port/platform/pe_plat_gpu_gen.c -o check
 *     (cdgen5 vectors, libgpu cases:
 *      grep -E '^(80077548|80077A00|80076C10) ' vectors.txt > v_gpu.txt)
 *   cc -std=c99 -Wall -Wextra -Werror -DCHK_HAVE_CD_GEN6 -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_spu_caller_check.c pc_port/platform/pe_plat_cd_gen6.c -o check
 *     (cdgen6 vectors; frame pointers compared as offsets "F+n")
 *   cc -std=c99 -Wall -Wextra -Werror -DCHK_HAVE_CD_GEN7 -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_spu_caller_check.c pc_port/platform/pe_plat_cd_gen7.c -o check
 *     (cdgen7 vectors; the callback sentinel 0x80130980 is compared by target, $a0, $a1)
 *   cc -std=c99 -Wall -Wextra -Werror -DCHK_HAVE_CARD_GEN8 -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_spu_caller_check.c pc_port/platform/pe_plat_card_gen8.c -o check
 *     (cdgen8 vectors; the slot sentinel 0x801309C0 is compared by target, $a0, $a1)
 *   cc -std=c99 -Wall -Wextra -Werror -DCHK_HAVE_GPU_GEN9 -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_spu_caller_check.c pc_port/platform/pe_plat_gpu_gen9.c -o check
 *     (cdgen9 vectors; SetIntrMask / fill / text_2B04 are stubbed under their pe_plat names)
 *   cc -std=c99 -Wall -Wextra -Werror -DCHK_HAVE_DS_GEN10 -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_spu_caller_check.c pc_port/platform/pe_plat_ds_gen10.c -o check
 *     (cdgen10 vectors; printf calls match when the vector's format and consumed
 *      arguments are a field-wise prefix of the native's PE_Printf_Guest call)
 *   cc -std=c99 -Wall -Wextra -Werror -DCHK_HAVE_DS_GEN11 -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_spu_caller_check.c pc_port/platform/pe_plat_ds_gen11.c -o check
 *     (cdgen11 vectors; callback sentinels 0x801309E0 / 0x80130A00 reach PE_GuestCall)
 *   cc -std=c99 -Wall -Wextra -Werror -DCHK_HAVE_GPU_GEN12 -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_spu_caller_check.c pc_port/platform/pe_plat_gpu_gen12.c -o check
 *     (cdgen12 vectors; the log-vector sentinel 0x80130A40 reaches PE_GuestCall with four arguments)
 *   cc -std=c99 -Wall -Wextra -Werror -DCHK_HAVE_CARD_GEN13 -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_spu_caller_check.c pc_port/platform/pe_plat_card_gen13.c -o check
 *     (cdgen13 vectors; callees without a port implementation reach PE_Decomp_BoundaryFrom)
 *   cc -std=c99 -Wall -Wextra -Werror -DCHK_HAVE_CARD_GEN14 -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_spu_caller_check.c pc_port/platform/pe_plat_card_gen14.c -o check
 *     (cdgen14 vectors; slot sentinels 0x80130A80 (no arguments) / 0x80130AC0 (one) reach PE_GuestCall)
 *   cc -std=c99 -Wall -Wextra -Werror -DCHK_HAVE_CARD_GEN15 -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_spu_caller_check.c pc_port/platform/pe_plat_gpu_gen15.c pc_port/platform/pe_plat_card_gen15.c -o check
 *     (cdgen15 vectors)
 *   cc -std=c99 -Wall -Wextra -Werror -DCHK_HAVE_DS_GEN16 -Dfunc_800811E4=chk_cd_read_poll -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_spu_caller_check.c pc_port/platform/pe_plat_ds_gen16.c -o check
 *     (cdgen16 vectors; the poll returns positive k times, then a final value: RETS k | pos << 8 | final << 12)
 *   cc -std=c99 -Wall -Wextra -Werror -DCHK_HAVE_CARD_GEN17 -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_spu_caller_check.c pc_port/platform/pe_plat_card_gen17.c -o check
 *     (cdgen17 vectors; the slot goes through PE_Card_ICall with one argument)
 *   cc -std=c99 -Wall -Wextra -Werror -DCHK_HAVE_PAD_GEN18 -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_spu_caller_check.c pc_port/platform/pe_plat_pad_gen18.c -o check
 *     (cdgen18 vectors; the object's +0x14 hook goes through PE_GuestCall with one argument)
 *   cc -std=c99 -Wall -Wextra -Werror -DCHK_HAVE_API_GEN19 -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_spu_caller_check.c pc_port/platform/pe_plat_api_gen19.c -o check
 *     (cdgen19 vectors; strcmp returns 0 only for the name pointer in RETS, BIOS firstfile goes through
 *      PE_Decomp_BoundaryFrom with two arguments, the saved hook 0x80130B00 through PE_GuestCall with three)
 *   cc -std=c99 -Wall -Wextra -Werror -DCHK_HAVE_CARD_GEN20 -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_spu_caller_check.c pc_port/platform/pe_plat_card_gen20.c -o check
 *     (cdgen20 vectors; slot sentinel 0x80130C00 reaches PE_GuestCall with one argument; 0x80130C40 / 0x80130C80
 *      with none: the original sets no $a0 for them, so the recorded $a0 is leftover and is not compared)
 *   cc -std=c99 -Wall -Wextra -Werror -DCHK_HAVE_CARD_GEN21 -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_spu_caller_check.c pc_port/platform/pe_plat_card_gen21.c -o check
 *     (cdgen21 vectors; SIO register reads replayed in order; the timeout check 0x80084FE4 returns 0 for j calls,
 *      then a final value: RETS j | f << 8; it takes no argument, so only its target is compared)
 *   cc -std=c99 -Wall -Wextra -Werror -DCHK_HAVE_CARD_GEN22 -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_spu_caller_check.c pc_port/platform/pe_plat_card_gen22.c -o check
 *     (cdgen22 vectors; SIO / I_STAT / RCNT2 register reads replayed in order, stores in order; 0x80084FC4
 *      compared with its argument, 0x80084FE4 by target as in cdgen21)
 *   cc -std=c99 -Wall -Wextra -Werror -DCHK_HAVE_PAD_GEN23 -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_spu_caller_check.c pc_port/platform/pe_plat_pad_gen23.c -o check
 *     (cdgen23 vectors; slot sentinels 0x80130D00 / 0x80130D40 / 0x80130D80 / 0x80130DC0 (one argument) and
 *      0x801309C0 (two) reach PE_Card_ICall, aliases compared as recorded; 0x800832B4 / 0x800830DC /
 *      0x800834E8 replay the scripted sequences and memory writes coded in RETS, and the 0x80130D40 slot
 *      writes both pair words on its call m (v3; see the CHK_HAVE_PAD_GEN23 stubs and blk23); 0x800834E8 /
 *      0x80083578 take no argument, so only their targets are compared)
 *   cc -std=c99 -Wall -Wextra -Werror -DCHK_HAVE_PAD_GEN24 -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_spu_caller_check.c pc_port/platform/pe_plat_pad_gen24.c -o check
 *     (cdgen24 vectors; step-table sentinels 0x80130E00 + 0x40 * k and the hook 0x80130DC0 reach PE_Card_ICall
 *      with one argument, compared by target and $a0; both may rewrite 0x8009B768 as coded in RETS (step24 /
 *      hook24 stubs); 0x800834E8 is compared by target only; the native is void, so no return is compared)
 *   cc -std=c99 -Wall -Wextra -Werror -DCHK_HAVE_CARD_GEN25 -Dpe_plat_pad_sio_start=chk_card_sector -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_spu_caller_check.c pc_port/platform/pe_plat_card_gen25.c -o check
 *     (cdgen25 vectors; 0x80082E00 (pe_plat_pad_sio_start since cdgen35, bound in pe_plat/pad.h; -D double chk_card_sector) and 0x80083014 (pe_plat_pad_transfer_step) are
 *      stubbed with the sequences and absolute memory writes coded in RETS (sector25 / xfer25 / hook25); the
 *      hook 0x80130DC0 reaches PE_Card_ICall with one argument)
 *   cc -std=c99 -Wall -Wextra -Werror -DCHK_HAVE_PAD_GEN26 -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_spu_caller_check.c pc_port/platform/pe_plat_pad_gen26.c -o check
 *     (cdgen26 vectors; the reset slot 0x801310C0 reaches PE_Card_ICall and the +0x18 hook 0x80131100
 *      PE_GuestCall, both with one argument; they and 0x80083644 (pe_plat_pad_recv_step) replay the absolute
 *      obj+0x46 / +0x36 writes coded in RETS (state26); 0x80084F8C is pe_plat_pad_record_busy; the native is
 *      void, so no return is compared)
 *   cc -std=c99 -Wall -Wextra -Werror -DCHK_HAVE_DS_GEN28 -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_spu_caller_check.c pc_port/platform/pe_plat_ds_gen28.c -o check
 *     (cdgen28 vectors; the stack buffer passed to 0x80080AE4, 0x80080C48 and the DS_CB3 callback 0x80130A00
 *      is compared frame-relative ("F+n"); 0x80080C48 returns the live *0x8009B6EC plus a coded delta, and the
 *      callback / 0x80082204 replay the absolute block writes coded in RETS (pos28 / cb28 / blk28); the
 *      DsCommand / DsDataCallback are the pe_plat_cd_queue_command_host / pe_plat_ds_data_callback stand-ins below,
 *      forwarding to chk_ds_command / chk_ds_data_callback; the native is void)
 *   cc -std=c99 -Wall -Wextra -Werror -DCHK_HAVE_CD_GEN29 -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_spu_caller_check.c pc_port/platform/pe_plat_cd_gen29.c -o check
 *     (cdgen29 vectors; no callees; register reads (the status byte) replayed in order and register stores
 *      compared in order; PE_CdLoad* / PE_CdStore* are the plain value accesses here, the poll tick is a
 *      no-op, the device reads detached, and a stuck-wait stop fails the case)
 *   cc -std=c99 -Wall -Wextra -Werror -DCHK_HAVE_DS_GEN30 -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_spu_caller_check.c pc_port/platform/pe_plat_ds_gen30.c -o check
 *     (cdgen30 vectors; 0x8007FBF0 (pe_plat_ds_status_lane) and 0x8007FB44 (pe_plat_cd_try_issue_cmd) are
 *      stubbed: recorded with their arguments, returning the RETS values)
 *   cc -std=c99 -Wall -Wextra -Werror -DCHK_HAVE_DS_GEN31 -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_spu_caller_check.c pc_port/platform/pe_plat_ds_gen31.c -o check
 *     (cdgen31 vectors; copy8 0x80080998 (pe_plat_cd_copy8) is stubbed: recorded with dst, src; the native
 *      is void)
 *   cc -std=c99 -Wall -Wextra -Werror -DCHK_HAVE_CD_GEN32 -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_spu_caller_check.c pc_port/platform/pe_plat_cd_gen32.c -o check
 *   cc -std=c99 -Wall -Wextra -Werror -DCHK_HAVE_PAD_GEN34 -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_spu_caller_check.c pc_port/platform/pe_plat_pad_gen34.c -o check
 *     (cdgen34 vectors, libpad PadStartCom 0x80082CF0: the five callees are stubbed here, each making the absolute
 *      write its RETS code selects; 0x8007E1E4 / 0x8007E1F4 are the pe_plat_bios_sys_enq/deq_int_rp host services, stubbed with two arguments; the slot
 *      word 0x8009B728 reaches PE_Card_ICall with one argument)
 *   cc -std=c99 -Wall -Wextra -Werror -DCHK_HAVE_PAD_GEN35 -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_spu_caller_check.c pc_port/platform/pe_plat_pad_gen35.c -o check
 *     (cdgen35 vectors, SIO0 start routine 0x80082E00: the slots 0x8009B744 / 0x8009B748 go through PE_Card_ICall
 *      (sentinels 0x80130D40 / 0x80130D80 or their KUSEG aliases) and may rewrite one later-read word on their
 *      m-th call (pe_cd_leaf_vectors.py _mut35; RETS code bits: 0-1 return, 2-4 kind, 5-8 value, 9-10 cell,
 *      11-13 m); 0x80083578 takes no argument, 0x80084FC4 one.)
 *     (cdgen32 vectors; range setter 0x8007C544 is stubbed with three arguments; native is void)
 *   cc -std=c99 -Wall -Wextra -Werror -DCHK_HAVE_CD_GEN33 -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_spu_caller_check.c pc_port/platform/pe_plat_cd_gen33.c -o check
 *     (cdgen33 vectors; callees 0x80073CF4 (pe_plat_dma_callback), 0x8007F778 (pe_plat_cd_queue_len),
 *      0x8007FBF0 (pe_plat_ds_status_lane) and 0x8007AAB4 (pe_plat_cd_acknowledge_host) are stubbed;
 *      the acknowledge worker returns POS33[p] k times then FIN33[f] and on its first call writes the
 *      register byte IDX33[w] through the pointer word 0x8009B27C, as pe_cd_leaf_vectors.py _ack33_model)
 *   cc -std=c99 -Wall -Wextra -Werror -DCHK_HAVE_DS_GEN38 -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_spu_caller_check.c pc_port/platform/pe_plat_ds_gen38.c -o check
 *     (cdgen38 vectors; callees 0x8007F0C8 (pe_plat_cd_command_sequence_host, record pointer frame-relative),
 *      0x8007F72C (pe_plat_ds_system_status, coded sequence), 0x8007FBF0, 0x80080B44 (frame-relative record),
 *      0x80081DF8 and 0x800822AC are stubbed; reads of the plain word 0x800B28F8 are replayed in order like
 *      register reads (pe_cd_leaf_vectors.py MEMSEQ); the poll tick is a no-op, a stuck-wait stop fails the case)
 *   cc -std=c99 -Wall -Wextra -Werror -DCHK_HAVE_DS_GEN39 -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_spu_caller_check.c pc_port/platform/pe_plat_ds_gen39.c -o check
 *     (cdgen39 vectors; the queue push 0x8007EB88 (pe_plat_ds_text_5f4) is stubbed with three arguments and may rewrite
 *      the count word 0x800A3608 on its call m (RETS code m | v << 4); callbacks go through PE_Cd_DispatchDataCallback,
 *      compared as target, status, response)
 *     (cdgen39 also: callees 0x8007E5C4 (pe_plat_ds_cq_pop; may rewrite the count word), 0x8007EB88
 *      (pe_plat_ds_text_5f4), 0x8007FB44 (pe_plat_cd_try_issue_cmd), 0x8007FBF0 (two-value lane sequence) and
 *      0x80080998 (pe_plat_cd_copy8) are stubbed; callbacks go through PE_Cd_DispatchDataCallback)
 *   ./check vectors.txt   (each line checks only entries it links; split mixed files first)
 * (-Wl,--unresolved-symbols=ignore-all only covers pe_plat_libc.c's
 * references unrelated to the swap provider.)
 * _spu_FiDMA: _spu_Fw1ts 0x8007DCAC is compared by name only; the transfer
 * callback goes through PE_GuestHook_Call and is compared by target and $a0.
 * _card_clear: the -D flags map the BIOS card stubs onto this tool's test
 * doubles; _new_card 0x8007DDC4 is compared by $a0, _card_write 0x8007DDB4
 * by $a0..$a2 (it returns the RETS value).
 * qsort: the comparator goes through PE_GuestHook_Call and is modelled here
 * as in the harness; calls are compared by target, $a0, $a1.
 * SpuSetReverb (`spureverb`): pe_plat_state.c also defines the stubbed
 * callee, so link an extracted copy of pe_plat_spu_set_reverb_enabled:
 *   { echo '#include <stdint.h>'; echo '#include "pe_guest_ram.h"';
 *     echo 'int pe_plat_spu_heap_contains(uint32_t address);';
 *     sed -n '/^int pe_plat_spu_set_reverb_enabled(/,/^}/p' pc_port/platform/pe_plat_state.c; } > setrev.c
 *   (the first line with -DPE_CHECK_SETREVERB and setrev.c added)
 * Memory, register stores and register reads as in pe_cd_leaf_vectors_check.c.
 * The four callees are stubbed here.  They must be called in the original's
 * order (the CALL list), with the original's arguments:
 *   0x80085BB4 _SpuIsInAllocateArea_(a0) -> returns the RETS value;
 *   0x8008D140 _spu_setReverbAttr(record) -> the 68 bytes at the record
 *     must equal the bytes the original passed (its record lived in its own
 *     stack frame; a native builds it in a PE_GuestFrame);
 *   0x8008D610 SpuClearReverbWorkArea(a0) -> returns 0;
 *   0x8007DAE0 _spu_FsetRXX(a0, a1, a2);
 *   0x80073A34 DeliverEvent(class, spec) -> PE_Event_Deliver (BIOS B(07h)
 *     host model); both arguments compared.
 * Each CALL token ends in @n: the original had made its first n logged
 * stores before that call.  The code under test must have written exactly
 * the same set of distinct bytes (outside the frame) by then -- compared by
 * count of distinct bytes, so store widths may differ.
 * Writes inside the guest-frame arena [PE_GUEST_FRAME_LO, HI) are the
 * native's locals: allowed, readable back, and not compared. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <setjmp.h>
#include "pe_guest_ram.h"
#include "pe_plat/audio.h"
#include "pe_plat/libc.h"
#include "pe_sdk.h"
#include "pe_guestcode.h"
#include "pe_plat/state.h"
#include "pe_spu_dma.h"
#define MEM 0x200000u
static uint8_t ram[MEM], want[MEM];
static uint32_t readable[MEM], wrote[MEM], want_set[MEM], cur;
static uint32_t wlog[4096], nwlog, xlog[4096], nxlog;
static int bad_read;
static void rd(pe_addr_t a, unsigned n)
{
    if (a >= PE_GUEST_FRAME_LO && a < PE_GUEST_FRAME_HI) {
        unsigned j;
        for (j = 0; j < n; j++) if (readable[(a + j) & (MEM - 1)] != cur) bad_read = 1;
        return;
    }
    unsigned i;
    for (i = 0; i < n; i++) {
        uint32_t b = (a + i) & (MEM - 1);
        if (readable[b] == cur) continue;
#ifndef CHK_STRICT_READS
        /* a byte the code under test stored itself earlier in this case may be read back (the original does the same and
         * the generators no longer log such own-written loads); -DCHK_STRICT_READS restores the old rule */
        if (wrote[b] == cur) continue;
#endif
        bad_read = 1;
    }
}
/* Register stores by the code under test, in order. */
static uint32_t mlog[256][3], nmlog;
static void mmio(pe_addr_t a, unsigned n, uint32_t v)
{
    if ((a >> 16) == 0x1F80u && nmlog < 256) {
        mlog[nmlog][0] = a; mlog[nmlog][1] = n; mlog[nmlog][2] = v; nmlog++;
    }
}
static int in_frame(pe_addr_t a) { return a >= PE_GUEST_FRAME_LO && a < PE_GUEST_FRAME_HI; }
static unsigned nstores;
static void wr(pe_addr_t a, unsigned n)
{
    if (!in_frame(a)) nstores++;
    if (in_frame(a)) {
        unsigned i;
        for (i = 0; i < n; i++) readable[(a + i) & (MEM - 1)] = cur;
        return;
    }
    unsigned i;
    for (i = 0; i < n; i++) {
        uint32_t b = (a + i) & (MEM - 1);
        if (wrote[b] != cur) { wrote[b] = cur; if (nwlog < 4096) wlog[nwlog++] = b; }
    }
}
/* Register reads are hardware inputs: they are replayed in the original's
 * order (same address and size), each returning the value the original
 * read; reads beyond, or out of, that sequence fail the case. */
static uint32_t rq[8192][3], nrq, rq_pos;   /* fidma timeout: 3842 reads */
/* A native wait loop that disagrees with the replayed register reads would
 * spin forever on the 0 returned past the sequence: after 1M such reads the
 * case is abandoned (counted as a mismatch) instead of hanging the run. */
static jmp_buf case_escape; static int case_escape_armed; static unsigned long runaway_reads;
static int mmio_read(pe_addr_t a, unsigned n, uint32_t *v)
{
#ifdef CHK_HAVE_DS_GEN38
    /* cdgen38: the plain word 0x800B28F8 is polled for an asynchronous event (MEMSEQ model); its reads replay in order */
    if ((a >> 16) != 0x1F80u && a != 0x800B28F8u) return 0;
#else
    if ((a >> 16) != 0x1F80u) return 0;
#endif
    if (rq_pos >= nrq || rq[rq_pos][0] != a || rq[rq_pos][1] != n) {
        bad_read = 1; *v = 0;
        if (case_escape_armed && ++runaway_reads > 1000000ul) longjmp(case_escape, 1);
        return 1;
    }
    *v = rq[rq_pos++][2];
    return 1;
}
uint8_t PE_LoadU8(pe_addr_t a) { uint32_t m; if (mmio_read(a, 1, &m)) return (uint8_t)m; rd(a, 1); return ram[a & (MEM - 1)]; }
uint16_t PE_LoadU16(pe_addr_t a) { uint16_t v; uint32_t m; if (mmio_read(a, 2, &m)) return (uint16_t)m; rd(a, 2); memcpy(&v, ram + (a & (MEM - 1)), 2); return v; }
uint32_t PE_LoadU32(pe_addr_t a) { uint32_t v; if (mmio_read(a, 4, &v)) return v; rd(a, 4); memcpy(&v, ram + (a & (MEM - 1)), 4); return v; }
void PE_StoreU8(pe_addr_t a, uint8_t v) { mmio(a, 1, v); wr(a, 1); ram[a & (MEM - 1)] = v; }
void PE_StoreU16(pe_addr_t a, uint16_t v) { mmio(a, 2, v); wr(a, 2); memcpy(ram + (a & (MEM - 1)), &v, 2); }
void PE_StoreU32(pe_addr_t a, uint32_t v) { mmio(a, 4, v); wr(a, 4); memcpy(ram + (a & (MEM - 1)), &v, 4); }
/* Guest frames: a LIFO arena, as the port's. */
static pe_addr_t frame_top = PE_GUEST_FRAME_HI;
pe_addr_t PE_GuestFrame_Enter(uint32_t size)
{
    frame_top = (frame_top - size) & ~7u;
    if (frame_top < PE_GUEST_FRAME_LO) bad_read = 1;
    return frame_top;
}
void PE_GuestFrame_Leave(pe_addr_t base, uint32_t size) { (void)size; frame_top = base; }

/* Callee stubs: record the call as text tokens comparable with the CALL list. */
#define CMP_ASC  0x80130900u   /* qsort comparator sentinels (pe_cd_leaf_vectors.py qsort) */
#define CMP_DESC 0x80130940u
#define MAX_CALLS 1024   /* qsort: up to 533 comparator calls */
static char got_calls[MAX_CALLS][300]; static unsigned ngot; static uint32_t rets_val;
/* cdgen vectors: RETS lists one scripted return per callee, in this order. */
static uint32_t rets_list[12]; static unsigned nrets; static uint32_t cur_entry;
#ifdef CHK_HAVE_CD_READ_STREAM
int pe_plat_cd_read_stream(uint32_t,uint32_t);
#endif
static const struct { uint32_t entry, callee[12]; } k_cdgen[] = {
    {0x80081314u,{0x8007F0C8u,0x800824C8u,0x800824F0u}},
    {0x80080164u, {0x80080220u, 0x80080404u, 0x8008068Cu, 0x8008080Cu}},
    {0x8007FCFCu, {0x8007B558u,0x8007B9ECu,0x80080950u}},
    {0x80080D5Cu, {0x8007EE84u,0x8007F418u}},
    {0x80080DC4u, {0x8007EE84u,0x8007F418u}},
    {0x8007F994u, {0x80073D58u, 0x8007BAC0u, 0x8007BBFCu, 0x8007FA2Cu, 0x800812F4u}},
    {0x800800F4u, {0x8007B558u, 0x8007B9ECu}},
    {0x800811E4u, {0x80073A44u, 0x8007F608u, 0x80081268u}},
    {0x8007C304u, {0x8007C544u}},
    {0x8007E704u, {0x8007EB88u}},
    {0x8007E964u, {0x8007E5C4u, 0x8007EB88u, 0x8007FB44u, 0x8007FBF0u, 0x80080998u}},
    {0x80082314u, {0x8007F0C8u, 0x8007F72Cu, 0x8007FBF0u, 0x80080B44u, 0x80081DF8u, 0x800822ACu}},
    {0x800824F0u, {0x80073CF4u}}, {0x8007F72Cu, {0x8007F778u, 0x8007FBF0u}}, {0x8007C13Cu, {0x8007AAB4u}},
    {0x8007EB88u, {0x80080998u}},
    {0x8007E8F4u, {0x8007FBF0u, 0x8007FB44u}},
    {0x800813E8u, {0x8007C564u, 0}}, {0x800822BCu, {0x800824C8u, 0x800824DCu}},
    {0x80082400u, {0x80081D74u, 0}}, {0x80080F64u, {0x80081D74u, 0}},
    {0x8007FB04u, {0x8007B9ECu, 0x80073D58u}}, {0x8007BBB0u, {0x80073C94u, 0x80073CC4u}},
    {0x8007C2A0u, {0x8007AA34u, 0x8007A930u}},
    {0x800808BCu, {0x8007B9ECu, 0, 0}}, {0x80081D74u, {0x800824C8u, 0x800824DCu, 0}},
    {0x80081DF8u, {0x800824C8u, 0x800824DCu, 0x8007EE84u}}, {0x800819D8u, {0x80072A54u, 0, 0}},
    {0x8007F69Cu, {0x800808BCu, 0x80081DF8u, 0x80080930u}},
    {0x80081268u, {0x8007F69Cu, 0x800822BCu, 0x800824F0u, 0x8007FCBCu, 0x8007EE84u}},
    {0x8007FB44u, {0x8007FCFCu}}, {0x8007FCBCu, {0x8007FCFCu}}, {0x80080CDCu, {0x8007FC28u}},
    {0x8008227Cu, {0x800824C8u}},
    {0x80082204u, {0x800824C8u, 0x8007FC28u, 0x80080C48u, 0x8007FC18u, 0x8007FC44u, 0x8007F0C8u}},
    {0x8007F608u, {0x8007FC88u, 0x80080998u}}, {0x80077A00u, {0x80073CF4u}},
    {0x80076C10u, {0x80076C34u}},
    {0x80082444u, {0x80080AE4u, 0x80071A04u, 0x80081DF8u}},
    {0x800840DCu, {0x801309C0u, 0x800832B4u}},
    {0x80077144u, {0x80073E10u, 0x80077A28u, 0x80077548u}},
    {0x800809E0u, {0x80071A74u, 0x800822ACu}},
    {0x80080F98u, {0x80073A44u, 0x80080B04u, 0x80081268u, 0x80080AE4u, 0x801309E0u}},
    {0x8008214Cu, {0x8007F778u, 0x80082204u, 0x800824C8u, 0x800824DCu, 0x80130A00u}},
    {0x80074BB8u, {0x80130A40u}},
    {0x8007DDD4u, {0x80073C74u, 0x80072714u, 0x8007DEB0u, 0x8007E314u, 0x8007E4E0u, 0x8007E3DCu,
                   0x8007E470u, 0x80072724u}},
    {0x8007DE40u, {0x80072714u, 0x8007E324u, 0x80073C74u, 0x80072724u}},
    {0x80082B08u, {0x80130A80u}}, {0x80083DF0u, {0x80130AC0u}},
    {0x80081D18u, {0x80080B44u, 0x80080E34u, 0x800811E4u}},
    {0x80084EB0u, {0x80130AC0u}},
    {0x80084B78u, {0x80083E50u, 0x800835C0u, 0x80130AC0u}},
    {0x800727B4u, {0x80072A54u, 0x80072A64u}}, {0x80072950u, {0x80072A54u, 0x80130B00u}},
    {0x80083F04u, {0x80130C00u, 0x800830DCu}}, {0x80083F44u, {0x80130C40u, 0x80130C80u, 0x80130C00u, 0x800832B4u}},
    {0x8008401Cu, {0x80130C00u, 0x800832B4u}}, {0x800834E8u, {0x80084FE4u}},
    {0x800830DCu, {0x80084FE4u}}, {0x800832B4u, {0x80084FC4u, 0x80084FE4u}},
    {0x80083014u, {0x80130E00u, 0x80130DC0u, 0x80084FC4u, 0x800834E8u}},
    {0x80082B70u, {0x80082E00u, 0x80130DC0u, 0x80083014u}},
    {0x80084C4Cu, {0x80084F8Cu, 0x801310C0u, 0x80131100u, 0x80083644u}},
    {0x80081E70u, {0x8007FC28u, 0x80080C48u, 0x8007FC18u, 0x800824F0u, 0x80080AE4u, 0x80130A00u, 0x800824C8u,
                   0x800824DCu, 0x8007F778u, 0x80082204u, 0x8007EE84u}},
    {0x80084168u, {0x80130D00u, 0x801309C0u, 0x800832B4u, 0x80084FC4u, 0x800834E8u, 0x80130D40u,
                   0x80130D80u, 0x800830DCu, 0x80083578u, 0x80130DC0u}},
    {0x80082E00u, {0x80083578u, 0x80084FC4u, 0x80130D40u, 0x80130D80u}},
    {0x80082CF0u, {0x80072714u, 0x80072724u, 0x80073C84u, 0x8007E1E4u, 0x8007E1F4u, 0x80131240u, 0x80131280u}},
};
static uint32_t ret_of(uint32_t fn)
{
    unsigned i, j;
    for (i = 0; i < sizeof k_cdgen / sizeof k_cdgen[0]; i++)
        if (k_cdgen[i].entry == cur_entry)
            for (j = 0; j < 12 && j < nrets; j++)
                if (k_cdgen[i].callee[j] == fn) return rets_list[j];
    return rets_val;
}
/* Pointers into the caller's own stack frame: the original's lie in the
 * harness stack window, the native's in the guest-frame arena.  Both sides
 * are written as offsets from the case's first such pointer ("F+n"). */
static uint32_t want_fbase, got_fbase; static int want_fset, got_fset;
static void fmt_want_ptr(char *o, size_t n, uint32_t v)
{
    if (v >= 0x801FE000u && v < 0x801FF000u) {
        if (!want_fset) { want_fbase = v; want_fset = 1; }
        snprintf(o, n, "F%+d", (int)(v - want_fbase));
    } else snprintf(o, n, "%08X", v);
}
static void fmt_got_ptr(char *o, size_t n, uint32_t v)
{
    if (v >= PE_GUEST_FRAME_LO && v < PE_GUEST_FRAME_HI) {
        if (!got_fset) { got_fbase = v; got_fset = 1; }
        snprintf(o, n, "F%+d", (int)(v - got_fbase));
    } else snprintf(o, n, "%08X", v);
}
static void note(const char *t)
{
    if (ngot < MAX_CALLS) snprintf(got_calls[ngot], sizeof got_calls[ngot], "%s@%u", t, nwlog);
    ngot++;
}
#ifdef CHK_HAVE_CD_STATUS_CALLBACK

static void status_callback_callee(uint32_t fn, uint32_t status, uint32_t response)
{
 char t[100];snprintf(t,sizeof t,"%08X,%08X,%08X",fn,status,response);note(t);
 if (nrets==6) {
  uint32_t i=rets_list[4];
  static const uint32_t states[]={0,31,32,0x11f,0x80000000u};
  static const uint32_t timers[]={0,1,2,0x7fffffffu,0x80000000u,0xffffffffu};
  if (fn==0x8008080Cu) {
   PE_StoreU8(0x8009B56Cu,(uint8_t)(i*37+7));
   PE_StoreU32(0x8009B570u,states[(i+1)%5]);
   PE_StoreU32(0x8009B598u,i^0x13579bdfu);
   PE_StoreU32(0x8009B59Cu,i^0x2468ace0u);
  } else {
   PE_StoreU32(0x8009B570u,i^0xa5a5a5a5u);
   PE_StoreU32(0x8009B598u,timers[i%6]);
   PE_StoreU32(0x8009B59Cu,i^0x11223344u);
  }
 }
}
void pe_plat_cd_decode_status(int a,uint32_t b){status_callback_callee(0x8008080Cu,a,b);}
void pe_plat_cd_command_complete(uint32_t a,uint32_t b){status_callback_callee(0x80080220u,a,b);}
void pe_plat_cd_command_progress(unsigned char a,int b){status_callback_callee(0x80080404u,a,b);}
void pe_plat_cd_unexpected_status(uint32_t a,uint32_t b){status_callback_callee(0x8008068Cu,a,b);}
#endif
#ifdef CHK_HAVE_CD_COMPLETE
void Bootstrap_ReturnVoid4Indirect(const char *s,const char *c,uintptr_t t,uintptr_t a,uintptr_t b,uintptr_t d,uintptr_t e)
{(void)s;(void)c;(void)t;(void)a;(void)b;(void)d;(void)e;bad_read=1;}
#endif

#if defined(CHK_HAVE_CD_COMMAND_WAIT) || defined(CHK_HAVE_CD_ISSUE_STEP)
int pe_plat_cd_issue_step(int,uint32_t,uint32_t);
int pe_plat_cd_command_wait(int,uint32_t,uint32_t);
static unsigned wait_polls;
uint32_t pe_test_ds_command_model(uint32_t a,uint32_t b,uint32_t c,uint32_t d){char t[100];snprintf(t,sizeof t,"8007EE84,%08X,%08X,%08X,%08X",a,b,c,d);note(t);return ret_of(0x8007EE84u);}
int pe_test_ds_sync_model(uint32_t a,uint32_t b){char t[80];snprintf(t,sizeof t,"8007F418,%08X,%08X",a,b);note(t);return (int32_t)(wait_polls++<rets_list[2]?rets_list[3]:ret_of(0x8007F418u));}
#endif
#ifdef CHK_HAVE_CD_ISSUE_WRAPPER
int pe_plat_cd_issue_wrapper(uint32_t,uint32_t);
static void wrapper_mutation(uint32_t fn){
 if(nrets!=5)return;
 uint32_t i=rets_list[3];
 if(fn==0x8007B9ECu)PE_StoreU32(0x8009B598u,i^0x11223344u);
 else if(fn==0x80080950u){
  static const uint8_t flags[]={0,1,2,255};static const uint32_t table[]={0,1,0xffffffffu};uint32_t cmd=i%32;
  PE_StoreU8(0x8009B558u,(uint8_t)cmd);PE_StoreU8(0x8009B58Bu,flags[(i/32)%4]);PE_StoreU32(0x8009B5A4u+4*cmd,table[i%3]);
 }else if(fn==0x8007B558u)PE_StoreU8(0x8009B558u,(uint8_t)(i*37+19));
}
void pe_plat_cd_flush(void){note("8007B9EC");wrapper_mutation(0x8007B9ECu);}
void pe_plat_cd_copy4(uint32_t d,uint32_t s){char t[80];snprintf(t,sizeof t,"80080950,%08X,%08X",d,s);note(t);wrapper_mutation(0x80080950u);}
int pe_test_cd_command_model(uint32_t a,uint32_t b,uint32_t c,uint32_t d){char t[100];snprintf(t,sizeof t,"8007B558,%08X,%08X,%08X,%08X",a,b,c,d);note(t);wrapper_mutation(0x8007B558u);return (int32_t)ret_of(0x8007B558u);}
#endif
#ifdef CHK_HAVE_CD_INIT_STEP
/* Oracle-only doubles: compile initializer with
 * -Dfunc_8007BBFC=pe_test_cd_sync_model -Dfunc_8007FE24=pe_test_cd_tick_model. */
void pe_test_cd_tick_model(void){bad_read=1;}
int PE_Callback_Bind(uint32_t target,void (*host)(void))
{(void)target;(void)host;bad_read=1;return -1;}
static void init_step_mutation(uint32_t fn) {
 if (cur_entry!=0x8007F994u || nrets!=7) return;
 uint32_t i=rets_list[5];
 if(fn==0x8007BBFCu)PE_StoreU32(0x800A36A8u,i^0x11223344u);
 else if(fn==0x8007BAC0u)PE_StoreU32(0x800A36A4u,i^0x22334455u);
 else if(fn==0x8007FA2Cu){PE_StoreU32(0x800A36A8u,i^0x33445566u);PE_StoreU32(0x8009AFB4u,i^0x44556677u);}
 else if(fn==0x800812F4u)PE_StoreU32(0x8009AFB8u,i^0x55667788u);
 else if(fn==0x80073D58u){PE_StoreU32(0x8009AFD8u,i^0x66778899u);PE_StoreU32(0x8009B554u,i^0x778899aau);}
}
int pe_test_cd_sync_model(void){note("8007BBFC");init_step_mutation(0x8007BBFCu);return (int32_t)ret_of(0x8007BBFCu);}
int pe_plat_cd_init_vol(void){note("8007BAC0");init_step_mutation(0x8007BAC0u);return (int32_t)ret_of(0x8007BAC0u);}
void pe_plat_cd_initialize_state(void){note("8007FA2C");init_step_mutation(0x8007FA2Cu);}
void pe_plat_cd_set_guarded_state(unsigned int a){char t[64];snprintf(t,sizeof t,"800812F4,%08X",a);note(t);init_step_mutation(0x800812F4u);}
#endif

int pe_plat_spu_heap_contains(uint32_t address)
{ char t[64]; snprintf(t, sizeof t, "80085BB4,%08X", address); note(t); return (int)rets_val; }
/* pe_plat_spu_modetype.c reaches func_80085BB4 through the clean-room binding (spu_b.h). */
int pe_plat_spub_heap_contains(uint32_t address) { return pe_plat_spu_heap_contains(address); }
void pe_plat_spu_set_reverb_attr(uint32_t attr)
{
    char t[256]; unsigned k, o;
    o = (unsigned)snprintf(t, sizeof t, "8008D140,");
    for (k = 0; k < 68 && o + 2 < sizeof t; k++) o += (unsigned)snprintf(t + o, sizeof t - o, "%02X", PE_LoadU8(attr + k));
    note(t);
}
/* retail 0x8008D610 (SpuClearReverbWorkArea): pe_plat_spu_modetype.c reaches it through the clean-room binding (pe_plat/cdx_spu.h) */
int pe_plat_cdxspu_clear_work_area(int mode)
{ char t[64]; snprintf(t, sizeof t, "8008D610,%08X", (uint32_t)mode); note(t); return 0; }
int PE_Port_ShouldStop(void) { return 0; }
void pe_plat_spu_wait_cycles(void)
{ note("8007DCAC"); }
int PE_GuestHook_Call(const char *slot, const char *caller, pe_addr_t fn,
                      unsigned arity, uintptr_t a0, uintptr_t a1,
                      uintptr_t a2, uintptr_t a3, int *ret)
{
    char t[64];
    (void)slot; (void)caller; (void)arity; (void)a2; (void)a3;
    if (fn == CMP_ASC || fn == CMP_DESC) {
        /* qsort comparator model (as the vector harness): the difference of
         * the two elements' first bytes, read raw from current guest RAM. */
        int d = (int)ram[(uint32_t)a0 & (MEM - 1)] - (int)ram[(uint32_t)a1 & (MEM - 1)];
        snprintf(t, sizeof t, "%08X,%08X,%08X", fn, (uint32_t)a0, (uint32_t)a1); note(t);
        if (ret) *ret = fn == CMP_ASC ? d : -d;
        return 1;
    }
    snprintf(t, sizeof t, "%08X,%08X", fn, (uint32_t)a0); note(t);
    if (ret) *ret = 0;
    return 1;
}
/* Test doubles for the BIOS card stubs: the build maps the guest names onto
 * them (-Dfunc_8007DDC4=chk_new_card -Dfunc_8007DDB4=chk_card_write), so
 * this tool defines no guest symbol of its own. */
void chk_new_card(pe_addr_t chan);
int chk_card_write(pe_addr_t chan, int sector, int src);
void chk_new_card(pe_addr_t chan)
{ char t[64]; snprintf(t, sizeof t, "8007DDC4,%08X", chan); note(t); }
int chk_card_write(pe_addr_t chan, int sector, int src)
{ char t[64]; snprintf(t, sizeof t, "8007DDB4,%08X,%08X,%08X", chan, (uint32_t)sector, (uint32_t)src); note(t); return (int)rets_val; }
/* cdgen callees: bound providers are stubbed by their pe_plat names; the
 * three unbound guest functions map onto chk_* doubles with -D. */
int pe_plat_cd_ds_set_ready_callback(int cb)
{ char t[64]; snprintf(t, sizeof t, "800824C8,%08X", (uint32_t)cb); note(t); return (int)ret_of(0x800824C8u); }
int pe_plat_cd_ds_set_start_callback(int cb)
{ char t[64]; snprintf(t, sizeof t, "800824DC,%08X", (uint32_t)cb); note(t); return (int)ret_of(0x800824DCu); }
uint32_t pe_plat_vsync_callbacks(uint32_t slot, uint32_t handler)
{ char t[64]; snprintf(t, sizeof t, "80073D58,%08X,%08X", slot, handler); note(t);
#ifdef CHK_HAVE_CD_INIT_STEP
  init_step_mutation(0x80073D58u);
#endif
  return ret_of(0x80073D58u); }
uint32_t pe_plat_reset_callback(void)
{ note("80073C94"); return ret_of(0x80073C94u); }
uint32_t pe_plat_interrupt_callback(uint32_t source, uint32_t handler)
{ char t[64]; snprintf(t, sizeof t, "80073CC4,%08X,%08X", source, handler); note(t); return ret_of(0x80073CC4u); }
int32_t pe_plat_cd_pos_to_int(pe_plat_guest_addr loc)
{   /* one provider bound to two guest copies (0x8007AA34, 0x80080C48):
     * record the copy this entry's original calls */
    uint32_t fn = cur_entry == 0x80082204u ? 0x80080C48u : 0x8007AA34u;
    char t[64];
#ifdef CHK_HAVE_DS_GEN28
    if (cur_entry == 0x80081E70u) {   /* cdgen28: frame-relative loc; live position word + coded delta */
        static const uint32_t d[8] = {0u, 0u, 1u, 0xFFFFFFFFu, 0x7FFFFFFFu, 0x80000000u, 2u, 0u};
        static unsigned last, n; char p[24]; uint32_t code = ret_of(0x80080C48u), w;
        if (last != cur) { last = cur; n = 0; }
        n++;
        fmt_got_ptr(p, sizeof p, loc);
        snprintf(t, sizeof t, "80080C48,%s", p); note(t);
        memcpy(&w, ram + 0x9B6ECu, 4);
        return (int32_t)(w + (n <= 2u ? d[(code >> (3u * (n - 1u))) & 7u] : 0u));
    }
#endif
    snprintf(t, sizeof t, "%08X,%08X", fn, loc); note(t); return (int32_t)ret_of(fn); }
uint32_t pe_plat_cd_int_to_pos(int32_t lba, pe_plat_guest_addr loc)
{   /* one provider bound to two guest copies (0x8007A930, 0x80080B44); a loc
     * in the caller's frame is written frame-relative (cdgen16) */
    uint32_t fn = (cur_entry == 0x80081D18u || cur_entry == 0x80082314u) ? 0x80080B44u : 0x8007A930u;
    char p[24], t[64]; fmt_got_ptr(p, sizeof p, loc);
    snprintf(t, sizeof t, "%08X,%08X,%s", fn, (uint32_t)lba, p); note(t); return ret_of(fn); }
void chk_st_cd_interrupt(void);
void chk_st_cd_interrupt(void) { note("8007C564"); }
void chk_cd_flush(void);
void chk_cd_flush(void) { note("8007B9EC"); }
#ifndef CHK_HAVE_CD_GEN2
/* DsStartReadySystem is bound (pe_plat_cd_gen2.c): stub it by its pe_plat
 * name everywhere except the mode that checks it. */
int pe_plat_ds_start_ready_system(int a0, int a1)
{ char t[64]; snprintf(t, sizeof t, "80081D74,%08X,%08X", (uint32_t)a0, (uint32_t)a1); note(t); return (int)ret_of(0x80081D74u); }
#endif
uint32_t chk_ds_command(uint32_t command, pe_addr_t param, uint32_t extra, uint32_t callback);
uint32_t chk_ds_command(uint32_t command, pe_addr_t param, uint32_t extra, uint32_t callback)
{ char t[80]; snprintf(t, sizeof t, "8007EE84,%08X,%08X,%08X,%08X", command, param, extra, callback); note(t); return ret_of(0x8007EE84u); }
int32_t pe_plat_bios_strcmp(pe_plat_guest_addr a, pe_plat_guest_addr b)
{
    char t[64]; snprintf(t, sizeof t, "80072A54,%08X,%08X", a, b); note(t);
    if (cur_entry == 0x800727B4u || cur_entry == 0x80072950u)   /* cdgen19 model: RETS holds the matching pointer */
        return a == ret_of(0x80072A54u) ? 0 : (a & 4u) ? -1 : 1;
    return (int32_t)ret_of(0x80072A54u);
}
/* cdgen3/cdgen4 callees.  Natives of other batches are stubbed by their
 * pe_plat names unless their file is linked. */
#ifndef CHK_HAVE_CD_GEN2
void pe_plat_cd_abort_cmd(void) { note("800808BC"); }
void pe_plat_ds_end_ready_system(void) { note("80081DF8"); }
#endif
#ifndef CHK_HAVE_CD_GEN
void pe_plat_ds_er_clear(void) { note("800822BC"); }
#endif
#ifndef CHK_HAVE_CD_GEN3
void pe_plat_ds_flush(void) { note("8007F69C"); }
#endif
void pe_plat_cd_enable_reads(void) { note("80080930"); }
pe_addr_t chk_ds_data_callback(pe_addr_t callback);
pe_addr_t chk_ds_data_callback(pe_addr_t callback)
{ char t[64]; snprintf(t, sizeof t, "800824F0,%08X", callback); note(t); return ret_of(0x800824F0u); }
#ifndef CHK_HAVE_CD_GEN5
int pe_plat_cd_7fcbc(int a0, int a1)
{ char t[64]; snprintf(t, sizeof t, "8007FCBC,%08X,%08X", (uint32_t)a0, (uint32_t)a1); note(t); return (int)ret_of(0x8007FCBCu); }
#endif
/* cdgen5 callees */
int chk_cd_send_cmd(uint32_t cmd, uint32_t data);
int chk_cd_send_cmd(uint32_t cmd, uint32_t data)
{ char t[64]; snprintf(t, sizeof t, "8007FCFC,%08X,%08X", cmd, data); note(t); return (int)ret_of(0x8007FCFCu); }
uint32_t pe_plat_cd_position_ptr(void) { note("8007FC28"); return ret_of(0x8007FC28u); }
uint8_t pe_plat_cd_sys_command_mode(void) { note("8007FC18"); return (uint8_t)ret_of(0x8007FC18u); }
uint8_t pe_plat_cd_sys_position_byte5(void) { note("8007FC44"); return (uint8_t)ret_of(0x8007FC44u); }
int pe_plat_cd_poll_ready(uint32_t result)
{ char t[64]; snprintf(t, sizeof t, "8007FC88,%08X", result); note(t); return (int)ret_of(0x8007FC88u); }
void pe_plat_cd_copy8(uint32_t dst, uint32_t src)
{ char t[64]; snprintf(t, sizeof t, "80080998,%08X,%08X", dst, src); note(t); }
uint32_t pe_plat_dma_callback(uint32_t channel, uint32_t handler)
{ char t[64]; snprintf(t, sizeof t, "80073CF4,%08X,%08X", channel, handler); note(t); return ret_of(0x80073CF4u); }
int chk_76c34(pe_addr_t worker, pe_addr_t argument, int32_t copy_bytes, uint32_t auxiliary);
int chk_76c34(pe_addr_t worker, pe_addr_t argument, int32_t copy_bytes, uint32_t auxiliary)
{ char t[80]; snprintf(t, sizeof t, "80076C34,%08X,%08X,%08X,%08X", worker, argument, (uint32_t)copy_bytes, auxiliary); note(t); return (int)ret_of(0x80076C34u); }
/* cdgen6 callees: buffer arguments are frame-relative (fmt_got_ptr). */
int32_t pe_plat_cd_ds_get_sector(uint32_t buffer, int32_t words)
{ char p[24], t[64]; fmt_got_ptr(p, sizeof p, buffer); snprintf(t, sizeof t, "80080AE4,%s,%08X", p, (uint32_t)words); note(t); return (int32_t)ret_of(0x80080AE4u); }
int32_t pe_plat_bios_strncmp(pe_plat_guest_addr a, pe_plat_guest_addr b, int32_t n)
{ char p[24], t[80]; fmt_got_ptr(p, sizeof p, a); snprintf(t, sizeof t, "80071A04,%s,%08X,%08X", p, b, (uint32_t)n); note(t); return (int32_t)ret_of(0x80071A04u); }
int chk_ds_packet(uint32_t mode, pe_addr_t loc, int count, uint32_t a3, pe_addr_t buf);
int chk_ds_packet(uint32_t mode, pe_addr_t loc, int count, uint32_t a3, pe_addr_t buf)
{ char t[96]; snprintf(t, sizeof t, "8007F0C8,%08X,%08X,%08X,%08X,%08X", mode, loc, (uint32_t)count, a3, buf); note(t); return (int)ret_of(0x8007F0C8u); }
/* Stand-ins for the callees whose guest names pe_plat/cd.h now binds to pe_plat_* providers (the header lines used to map the
 * guest names with -Dfunc_X=chk_*, which redefine those macros under -Werror).  Each forwards to the chk_* double above, so the
 * recorded call text is unchanged; they exist only in the modes that need them and only where no other definition does. */
#ifdef CHK_HAVE_CD_GEN
void pe_plat_cd_stream_handler_host(void);
void pe_plat_cd_stream_handler_host(void) { chk_st_cd_interrupt(); }
#endif
#if (defined(CHK_HAVE_CD_GEN) || defined(CHK_HAVE_CD_GEN2)) && !defined(CHK_HAVE_CD_ISSUE_WRAPPER) && !defined(CHK_HAVE_CD_RETRY)
void pe_plat_cd_flush(void) { chk_cd_flush(); }
#endif
#if defined(CHK_HAVE_CD_GEN2) || defined(CHK_HAVE_CD_GEN4) || defined(CHK_HAVE_DS_GEN28)
uint32_t pe_plat_cd_queue_command_host(uint32_t command, uint32_t param, uint32_t extra, uint32_t callback);
uint32_t pe_plat_cd_queue_command_host(uint32_t command, uint32_t param, uint32_t extra, uint32_t callback)
{ return chk_ds_command(command, param, extra, callback); }
#endif
#if defined(CHK_HAVE_CD_GEN4) || defined(CHK_HAVE_DS_GEN28)
uint32_t pe_plat_ds_data_callback(uint32_t callback) { return chk_ds_data_callback(callback); }
#endif
#ifdef CHK_HAVE_CD_GEN5
int pe_plat_cd_issue_wrapper(uint32_t command, uint32_t data) { return chk_cd_send_cmd(command, data); }
int pe_plat_cd_command_sequence_host(uint32_t mode, uint32_t loc, int count, uint32_t extra, uint32_t callback)
{ return chk_ds_packet(mode, loc, count, extra, callback); }
#endif
/* cdgen7: CD data callbacks go through the host dispatcher; the vectors'
 * sentinel 0x80130980 stands for the callback target. */
int PE_Cd_DispatchDataCallback(pe_addr_t target, uint32_t status, pe_addr_t response)
{ char t[64]; snprintf(t, sizeof t, "%08X,%08X,%08X", target, status, response); note(t); return 1; }
/* cdgen8: libcard slot callbacks go through PE_Card_ICall; the vectors'
 * sentinel 0x801309C0 stands for the slot's target. */
#ifdef CHK_HAVE_PAD_GEN23
static void blk23(void);
#endif
#ifdef CHK_HAVE_DS_GEN28
/* cdgen28 (pe_cd_leaf_vectors.py _cb28_model / _blk28_model). */
static const uint32_t blk28[8] = {0u, 1u, 2u, 0xFFFFFFFFu, 0x80000000u, 0x7FFFFFFFu, 1u, 0u};
static void cb28(void)
{
    uint32_t code = ret_of(0x80130A00u);
    if (code & 1u) PE_StoreU32(0x8009B6F8u, blk28[(code >> 1) & 7u]);
    if ((code >> 4) & 1u) PE_StoreU32(0x8009B6FCu, blk28[(code >> 5) & 7u]);
}
int32_t pe_plat_cd_queue_len(void) { note("8007F778"); return (int32_t)ret_of(0x8007F778u); }
void pe_plat_ds_text_490(void)
{
    uint32_t code = ret_of(0x80082204u);
    note("80082204");
    if (code & 1u) PE_StoreU32(0x8009B6FCu, blk28[(code >> 1) & 7u]);
}
void pe_plat_ds_ready_callback(unsigned char intr, uint32_t result);
#endif
#ifdef CHK_HAVE_CD_GEN29
int pe_plat_cd_getsector2(uint32_t buffer, uint32_t words);
#endif
#ifdef CHK_HAVE_DS_GEN39
void pe_plat_ds_text_3d0(uint32_t status, uint32_t response);
/* 0x8007E5C4: may rewrite the count word 0x800A3608 (RETS code bit 0, value CW36[code >> 4 & 7]). */
void pe_plat_ds_cq_pop(void)
{
    static const uint32_t cw[8] = {0u, 1u, 2u, 5u, 0x100u, 0xFFFFFFFFu, 0x80000000u, 3u};
    uint32_t code = ret_of(0x8007E5C4u);
    note("8007E5C4");
    if (code & 1u) PE_StoreU32(0x800A3608u, cw[(code >> 4) & 7u]);
}
int pe_plat_cd_try_issue_cmd(uint32_t cmd, uint32_t data)
{ char t[64]; snprintf(t, sizeof t, "8007FB44,%08X,%08X", cmd, data); note(t); return (int)ret_of(0x8007FB44u); }
/* 0x8007FBF0: first read LV36[code & 15], later ones LV36[(code >> 4) & 15]. */
int pe_plat_ds_status_lane(int idx)
{
    static const uint32_t lv[10] = {1u, 1u, 1u, 0u, 2u, 3u, 0xFFFFFFFFu, 0x101u, 0x10001u, 0x80000001u};
    static unsigned last_case, calls;
    uint32_t code = ret_of(0x8007FBF0u);
    char t[48];
    if (last_case != cur) { last_case = cur; calls = 0; }
    calls++;
    snprintf(t, sizeof t, "8007FBF0,%08X", (uint32_t)idx); note(t);
    return (int)(calls == 1u ? lv[(code & 15u) % 10u] : lv[((code >> 4) & 15u) % 10u]);
}
#endif
#if defined(CHK_HAVE_DS_GEN39) || defined(CHK_HAVE_DS_GEN39)
void pe_plat_ds_text_170(uint32_t status, uint32_t response);
/* 0x8007EB88 (pe_plat_ds_text_5f4): on call m of a case (RETS code m | v << 4; m == 0: never) it rewrites the count
 * word 0x800A3608 = cnt35[v] (absolute; pe_cd_leaf_vectors.py _ds35_models). */
void pe_plat_ds_text_5f4(uint32_t sequence, uint32_t status, uint32_t response)
{
    static const uint32_t cnt[8] = {0u, 1u, 2u, 3u, 5u, 0xFFFFFFFFu, 0x80000000u, 9u};
    static unsigned last_case, calls;
    uint32_t code = ret_of(0x8007EB88u), m = code & 15u, v = (code >> 4) & 15u;
    char t[96];
    if (last_case != cur) { last_case = cur; calls = 0; }
    calls++;
    snprintf(t, sizeof t, "8007EB88,%08X,%08X,%08X", sequence, status, response); note(t);
    if (m && calls == m) PE_StoreU32(0x800A3608u, cnt[v & 7u]);
}
#endif
#ifdef CHK_HAVE_DS_GEN38
int pe_plat_ds_get_disk_type(void);
/* 0x8007FBF0: first read LANE34[code & 15], every later one LANE34B[(code >> 4) & 15] (pe_cd_leaf_vectors.py _ds34_models). */
int pe_plat_ds_status_lane(int idx)
{
    static const uint32_t l0[9] = {2u, 2u, 2u, 0u, 1u, 3u, 0x102u, 0x10002u, 0xFFFFFFFFu};
    static const uint32_t l1[12] = {0x10u, 0x10u, 0u, 1u, 2u, 0x110u, 0x10010u, 0x1010u, 0x80000010u, 0x20u, 0xFFFFFFFFu, 0x100u};
    static unsigned last_case, calls;
    uint32_t code = ret_of(0x8007FBF0u);
    char t[48];
    if (last_case != cur) { last_case = cur; calls = 0; }
    calls++;
    snprintf(t, sizeof t, "8007FBF0,%08X", (uint32_t)idx); note(t);
    return (int)(calls == 1u ? l0[(code & 15u) % 9u] : l1[((code >> 4) & 15u) % 12u]);
}
/* 0x8007F72C: coded sequence, code = m | idx_i << 4i (pe_cd_leaf_vectors.py _ds34_models). */
int pe_plat_ds_system_status(void)
{
    static const uint32_t sv[16] = {0u, 2u, 4u, 0xFFFFFFFFu, 0x101u, 0x10001u, 0x80000000u, 5u, 3u, 1u,
                                    0x103u, 0x10003u, 0x80000003u, 0x80000001u, 0x1000001u, 0x3000000u};
    static unsigned last_case, calls;
    uint32_t code = ret_of(0x8007F72Cu), m = code & 15u;
    if (last_case != cur) { last_case = cur; calls = 0; }
    calls++;
    note("8007F72C");
    return (int)sv[(code >> (4u * (calls < m ? calls : m))) & 15u];
}
int pe_plat_cd_command_sequence_host(uint32_t mode, uint32_t loc, int count, uint32_t extra, uint32_t callback)
{
    char p[24], t[128];
    fmt_got_ptr(p, sizeof p, loc);
    snprintf(t, sizeof t, "8007F0C8,%08X,%s,%08X,%08X,%08X", mode, p, (uint32_t)count, extra, callback);
    note(t);
    return (int)ret_of(0x8007F0C8u);
}
#endif
#ifdef CHK_HAVE_CD_GEN33
uint32_t pe_plat_ds_data_callback(uint32_t callback);
int pe_plat_ds_system_status(void);
void pe_plat_cd_intr_poll(void);
int32_t pe_plat_cd_queue_len(void) { note("8007F778"); return (int32_t)ret_of(0x8007F778u); }
int pe_plat_ds_status_lane(int idx)
{ char t[48]; snprintf(t, sizeof t, "8007FBF0,%08X", (uint32_t)idx); note(t); return (int)ret_of(0x8007FBF0u); }
/* 0x8007AAB4 (no argument): POS33[p] for the first k calls of a case, then
 * FIN33[f]; on its first call it writes the register byte IDX33[w] through
 * the pointer word 0x8009B27C (w == 8: none).  code = k | p << 4 | f << 8 | w << 12. */
int pe_plat_cd_acknowledge_host(void)
{
    static const uint32_t pos[16] = {1u, 2u, 4u, 6u, 3u, 7u, 0x102u, 0x104u, 0x7FFFFFFFu, 0x100u, 0xFFFFFFFFu,
                                     0x80000000u, 0x10000u, 0xFFu, 0x10002u, 8u};
    static const uint32_t fin[8] = {0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u};
    static const uint8_t idx[8] = {0, 1, 2, 3, 0x10, 0x21, 0xFF, 0x80};
    static unsigned last_case, calls;
    uint32_t code = ret_of(0x8007AAB4u), k = code & 15u, p = (code >> 4) & 15u, f = (code >> 8) & 15u, w = (code >> 12) & 15u;
    if (last_case != cur) { last_case = cur; calls = 0; }
    calls++;
    note("8007AAB4");
    if (calls == 1u && w < 8u) PE_StoreU8(PE_LoadU32(0x8009B27Cu), idx[w]);
    return (int)(calls <= k ? pos[p & 15u] : fin[f & 7u]);
}
#endif
#ifdef CHK_HAVE_CD_GEN32
void pe_plat_cd_set_stream(uint32_t mode, int32_t start, int32_t end, uint32_t callback, uint32_t auxiliary);
void pe_plat_gpuprim_stream_set_range(uint32_t mode, uint32_t start, uint32_t end);
void pe_plat_gpuprim_stream_set_range(uint32_t mode, uint32_t start, uint32_t end)
{ char t[80]; snprintf(t, sizeof t, "8007C544,%08X,%08X,%08X", mode, start, end); note(t); }
#endif
#ifdef CHK_HAVE_DS_GEN31
void pe_plat_ds_text_5f4(uint32_t sequence, uint32_t status, uint32_t response);
#endif
#ifdef CHK_HAVE_DS_GEN30
int pe_plat_ds_text_360(void);
int pe_plat_ds_status_lane(int idx);
int pe_plat_ds_status_lane(int idx)
{ char t[48]; snprintf(t, sizeof t, "8007FBF0,%08X", (uint32_t)idx); note(t); return (int)ret_of(0x8007FBF0u); }
int pe_plat_cd_try_issue_cmd(uint32_t cmd, uint32_t data);
int pe_plat_cd_try_issue_cmd(uint32_t cmd, uint32_t data)
{ char t[64]; snprintf(t, sizeof t, "8007FB44,%08X,%08X", cmd, data); note(t); return (int)ret_of(0x8007FB44u); }
#endif
#ifdef CHK_HAVE_PAD_GEN26
/* cdgen26 (pe_cd_leaf_vectors.py _state26_model): absolute obj+0x46 / +0x36
 * writes by the reset slot, the +0x18 hook and 0x80083644. */
static uint32_t state26(uint32_t sent, uint32_t obj, int ret)
{
    static const uint8_t st[16] = {0, 1, 2, 3, 0x7F, 0x80, 0xFD, 0xFE, 0xFF, 4, 0x10, 0xFC, 2, 1, 0, 0xFF};
    static const uint8_t b36[4] = {0, 1, 0x80, 0xFF};
    static const uint32_t rv[8] = {0u, 1u, 2u, 0xFFFFFFFFu, 0xFFu, 0x100u, 0x80000000u, 0xFFFFFFFEu};
    uint32_t code = ret_of(sent);
    if ((code >> 3) & 1u) PE_StoreU8(obj + 0x46u, st[(code >> 5) & 15u]);
    if ((code >> 4) & 1u) PE_StoreU8(obj + 0x36u, b36[(code >> 9) & 3u]);
    return ret ? rv[code & 7u] : 0u;
}
int pe_plat_pad_record_busy(uint32_t record)
{ char t[64]; snprintf(t, sizeof t, "80084F8C,%08X", record); note(t); return (int)ret_of(0x80084F8Cu); }
int pe_plat_pad_recv_step(uint32_t record)
{ char t[64]; snprintf(t, sizeof t, "80083644,%08X", record); note(t); return (int)state26(0x80083644u, record, 1); }
void pe_plat_pad_record_step(uint32_t obj);
#endif
#ifdef CHK_HAVE_CARD_GEN25
/* cdgen25 (pe_cd_leaf_vectors.py _xfer25_model / _sector25_model /
 * _hook25_model): absolute writes of the record words. */
static const uint32_t seq25[8] = {0xFFFFFFFFu, 0u, 1u, 2u, 3u, 4u, 5u, 6u};
static void hook25(void)
{
    uint32_t code = ret_of(0x80130DC0u);
    if (code & 1u) PE_StoreU32(0x8009B764u, seq25[(code >> 1) & 7u]);
}
int chk_card_sector(uint32_t a0);
int chk_card_sector(uint32_t a0)
{
    static const uint32_t rv[8] = {0u, 1u, 0xFFFFFFFFu, 0x80000000u, 0x42u, 0x7FFFFFFFu, 0x100u, 0xFFFFFFFEu};
    char t[64]; uint32_t code = ret_of(0x80082E00u);
    snprintf(t, sizeof t, "80082E00,%08X", a0); note(t);
    if ((code >> 3) & 1u) PE_StoreU32(0x8009B764u, seq25[(code >> 4) & 7u]);
    return (int)rv[code & 7u];
}
void pe_plat_pad_transfer_step(uint32_t obj)
{
    static const uint32_t bv[8] = {0xFFFFFFFFu, 0u, 1u, 2u, 3u, 5u, 0x80000000u, 4u};
    static unsigned last, n;
    char t[64]; uint32_t code = ret_of(0x80083014u), m = (code & 7u) + 1u, base = (code >> 3) & 7u, j = (code >> 6) & 7u;
    snprintf(t, sizeof t, "80083014,%08X", obj); note(t);
    if (last != cur) { last = cur; n = 0; }
    n++;
    if (n == j) PE_StoreU32(0x8009B778u, bv[(code >> 9) & 7u]);
    PE_StoreU32(0x8009B764u, n >= m ? 0x7FFFFFFFu : seq25[(base + n) % 8u]);
}
int pe_plat_card_timer_callback(void);
#endif
#ifdef CHK_HAVE_PAD_GEN24
/* cdgen24 (pe_cd_leaf_vectors.py _step24_model / _hook24_model): every
 * step-table entry returns STEP24_RET[r] and may rewrite the index word;
 * the error hook may rewrite it too. */
static const uint32_t idx24[16] = {0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u, 0xFFFFFFFFu, 0x7FFFFFFFu, 0x80000000u,
                                   5u, 6u, 0u, 4u};
static uint32_t step24(void)
{
    static const uint32_t rv[8] = {0u, 1u, 0xFFFFFFFFu, 0xFFFFFFFDu, 0x7FFFFFFFu, 0x80000000u, 0x42u, 0xFFFFFFECu};
    uint32_t code = ret_of(0x80130E00u);
    if ((code >> 3) & 1u) PE_StoreU32(0x8009B768u, idx24[(code >> 4) & 15u]);
    return rv[code & 7u];
}
static void hook24(void)
{
    uint32_t code = ret_of(0x80130DC0u);
    if (code & 1u) PE_StoreU32(0x8009B768u, idx24[(code >> 1) & 15u]);
}
#endif
#ifdef CHK_HAVE_PAD_GEN35
/* cdgen35 (pe_cd_leaf_vectors.py _sio35_slot / _mut35): the two slot
 * sentinels return RV35[code & 3] and, on the slot's m-th call of a case,
 * rewrite one word the routine reads later (absolute writes, through
 * PE_Store* like the original's logged ones). */
void pe_plat_card_sio_rx_wait(void) { note("80083578"); }
static uint32_t rec35;   /* the case's record (the entry's argument) */
static uint32_t slot35(uint32_t sent)
{
    uint32_t record = rec35;
    static const uint32_t rv[4] = {0u, 1u, 0xFFFFFFFFu, 0x42u};
    static const uint32_t rc[8] = {0u, 1u, 7u, 0xF0u, 0x100u, 0x12345678u, 0xFFFFFFFFu, 0x80000000u};
    static const uint32_t cells[4] = {0x8009B778u, 0x8009B77Cu, 0x8009B780u, 0x8009B78Cu};
    static const uint32_t wv[13] = {0xFFFFFFFFu, 0xFFFFFFFDu, 0u, 0u, 1u, 2u, 3u, 4u, 6u, 5u, 0x80000000u,
                                    0xFFFFFFFBu, 0u};
    static const uint32_t idx[16] = {0u, 0u, 0u, 1u, 1u, 0xFFFFFFFFu, 4u, 5u, 6u, 7u, 8u, 0xFFFFFFFEu,
                                     0x40000000u, 0x40000001u, 0xC0000000u, 0u};
    static const uint8_t b50[4] = {0, 1, 2, 0xFF}, b36[4] = {0, 1, 2, 0x80};
    static unsigned last[2], n[2];
    uint32_t code = ret_of(sent);
    unsigned w = sent == 0x80130D80u, kind = (code >> 2) & 7u, vi = (code >> 5) & 15u, m = (code >> 11) & 7u;
    if (last[w] != cur) { last[w] = cur; n[w] = 0; }
    n[w]++;
    if (n[w] == m && kind != 7u) {
        switch (kind) {
        case 0: PE_StoreU32(record + 0xCu, rc[vi & 7u]); break;
        case 1: PE_StoreU32(cells[(code >> 9) & 3u], wv[vi % 13u]); break;
        case 2: PE_StoreU32(0x8009B764u, idx[vi]); break;
        case 3: PE_StoreU8(record + 0x50u, b50[vi & 3u]); break;
        case 4: PE_StoreU8(record + 0x36u, b36[vi & 3u]); break;
        case 5: {
            static const uint32_t p3c[4] = {0x80101E10u, 0x80101E00u, 0x80101E01u, 0x80101E20u};
            PE_StoreU32(record + 0x3Cu, p3c[vi & 3u]);
            break; }
        default: {
            static const uint32_t ps[4] = {0x1F801070u, 0x80101F00u, 0x1F801070u, 0x80101F10u};
            PE_StoreU32(0x8009B784u, ps[vi & 3u]);
            break; }
        }
    }
    return rv[code & 3u];
}
int pe_plat_pad_sio_start(uint32_t record);
#endif
#ifdef CHK_HAVE_PAD_GEN34
/* cdgen34 (pe_cd_leaf_vectors.py _mut34 / _pad34_callee): every callee's RETS
 * value is a code for one absolute write made on its call (a slot: its first
 * or second call; a BIOS callee: its only call), through PE_Store* like the
 * original's logged ones; the returned value is ignored by the routine. */
static void write34(unsigned k, uint32_t sent, int is_slot)
{
    static const uint32_t ptrv[4] = {0x1F801070u, 0x80101F00u, 0x80101F40u, 0x80101F80u};
    static const uint32_t argv[12] = {0u, 1u, 2u, 0x1000u, 0xFFFFFF10u, 0xFFFFFF0Fu, 0x7FFFFF10u, 0x80000000u,
                                      0xFFFFFFFFu, 0x12345678u, 0xF0u, 0xEFu};
    static const uint32_t maskv[12] = {0u, 1u, 2u, 0x80u, 0xFFFEu, 0xFFFFu, 0x10000u, 0xFFFFFFFEu, 0xFFFFFFFFu,
                                       0x80000000u, 0x12345678u, 0x7FFFFFFFu};
    static const uint32_t slotv[5] = {0u, 0x80131240u, 0x00131240u, 0x80131280u, 0x00131280u};
    static const uint32_t cntv[4] = {0u, 1u, 0xFFFFFFFFu, 0x12345678u};
    static const uint32_t b75c[4] = {0u, 1u, 0xFFFFFFFFu, 0x42u};
    static unsigned last[8], n[8];
    uint32_t code = ret_of(sent), kind = code & 7u, vi = (code >> 3) & 15u;
    if (last[k] != cur) { last[k] = cur; n[k] = 0; }
    n[k]++;
    if (n[k] != (is_slot ? 1u + ((code >> 7) & 1u) : 1u)) return;
    switch (kind) {
    case 1: PE_StoreU32(0x8009B784u, ptrv[vi & 3u]); break;
    case 2: PE_StoreU32(0x8009B758u, argv[vi % 12u]); break;
    case 3: PE_StoreU32(0x8009B728u, slotv[vi % 5u]); break;
    case 4: PE_StoreU32(ptrv[1u + (vi & 1u) + ((vi >> 1) & 1u)] + 4u, maskv[(vi * 5u + 1u) % 12u]); break;
    case 5: PE_StoreU32(0x8009B75Cu, b75c[vi & 3u]); break;
    case 6: PE_StoreU32(vi & 1u ? 0x800A5AC4u : 0x800A5AC0u, cntv[(vi >> 1) & 3u]); break;
    default: break;
    }
}
#ifndef CHK_HAVE_CARD_GEN13
int pe_plat_bios_enter_critical(void) { note("80072714"); write34(0, 0x80072714u, 0); return 0; }
void pe_plat_bios_exit_critical(void) { note("80072724"); write34(1, 0x80072724u, 0); }
#endif
uint32_t pe_plat_bios_change_clear_rcnt(uint32_t counter, uint32_t flag)
{ char t[64]; snprintf(t, sizeof t, "80073C84,%08X,%08X", counter, flag); note(t); write34(2, 0x80073C84u, 0); return 0; }
/* The kernel interrupt-chain services C(02h) 0x8007E1E4 / C(03h) 0x8007E1F4 are bound host services
 * (pe_plat_bios_intrp.c): stubbed here, recorded with their two arguments like the boundary calls they
 * replaced, each making the RETS-selected write of the original's call. */
#include "pe_plat/bios_intrp.h"
static void intrp34(uint32_t vma, unsigned k, uint32_t pri, uint32_t rec)
{ char t[64]; snprintf(t, sizeof t, "%08X,%08X,%08X", vma, pri, rec); note(t); write34(k, vma, 0); }
int pe_plat_bios_sys_enq_int_rp(uint32_t priority, uint32_t record) { intrp34(0x8007E1E4u, 3u, priority, record); return 0; }
int pe_plat_bios_sys_deq_int_rp(uint32_t priority, uint32_t record) { intrp34(0x8007E1F4u, 4u, priority, record); return 0; }
int pe_plat_pad_start_com(void);
#endif
uint32_t PE_Card_ICall(const char *slot, pe_addr_t fn, unsigned arity, uint32_t a0, uint32_t a1, uint32_t a2)
{   /* recorded with the call's argument count (cdgen8: two, cdgen17: one) */
    char t[64]; (void)slot; (void)a2;
    if (arity == 1u) snprintf(t, sizeof t, "%08X,%08X", fn, a0);
    else snprintf(t, sizeof t, "%08X,%08X,%08X", fn, a0, a1);
    note(t);
#ifdef CHK_HAVE_PAD_GEN23
    if ((fn & 0x1FFFFFFFu) == 0x00130D40u) { blk23(); return 0; }
#endif
#ifdef CHK_HAVE_CARD_GEN25
    if ((fn & 0x1FFFFFFFu) == 0x00130DC0u) { hook25(); return 0; }
#endif
#ifdef CHK_HAVE_PAD_GEN26
    if ((fn & 0x1FFFFFFFu) == 0x001310C0u) return state26(0x801310C0u, a0, 0);
#endif
#ifdef CHK_HAVE_PAD_GEN35
    if ((fn & 0x1FFFFFFFu) == 0x00130D40u) return slot35(0x80130D40u);
    if ((fn & 0x1FFFFFFFu) == 0x00130D80u) return slot35(0x80130D80u);
#endif
#ifdef CHK_HAVE_PAD_GEN34
    if ((fn & 0x1FFFFFFFu) == 0x00131240u) { write34(5, 0x80131240u, 1); return 0; }
    if ((fn & 0x1FFFFFFFu) == 0x00131280u) { write34(6, 0x80131280u, 1); return 0; }
#endif
#ifdef CHK_HAVE_PAD_GEN24
    if ((fn & 0x1FFFFFFFu) >= 0x00130E00u && (fn & 0x1FFFFFFFu) < 0x001310C0u && !(fn & 0x3Fu)) return step24();
    if ((fn & 0x1FFFFFFFu) == 0x00130DC0u) { hook24(); return 0; }
#endif
    return ret_of(arity == 1u ? 0x80130AC0u : 0x801309C0u); }
#ifdef CHK_HAVE_PAD_GEN23
/* cdgen23 sequences (pe_cd_leaf_vectors.py _xch23_model / _ack23_model):
 * the n-th call of a case returns a scripted value and, on call m, makes the
 * callee's memory writes, through PE_Store* like the original's logged ones. */
static unsigned seq23_call(unsigned which)
{
    static unsigned last[4], n[4];
    if (last[which] != cur) { last[which] = cur; n[which] = 0; }
    return ++n[which];
}
static uint32_t xch23(uint32_t fn, unsigned which, uint32_t obj)
{
    static const uint32_t okv[4] = {0u, 0x42u, 0xFFu, 0x7FFFFFFFu};
    static const uint32_t finv[4] = {0xFFFFFFFFu, 0xFFFFFFFEu, 0xFFFFFFECu, 0x80000000u};
    static const uint8_t idx[8] = {0, 1, 7, 8, 0x7F, 0x80, 0xFE, 0xFF};
    uint32_t code = ret_of(fn), k = code & 15u, m = (code >> 8) & 15u;
    unsigned n = seq23_call(which);
    if (n == m) PE_StoreU8(obj + 0x44u, idx[(code >> 12) & 7u]);
    return (k == 15u || n <= k) ? okv[(code >> 4) & 3u] : finv[(code >> 6) & 3u];
}
#endif
#ifndef CHK_HAVE_CARD_GEN22
/* The libcard SIO pair 0x800830DC / 0x800832B4 is bound (pe_plat_card_gen22.c):
 * stubbed by its pe_plat names except in the mode that checks it. */
int pe_plat_card_sio_exchange(uint32_t a0, int a1)
{ char t[64]; snprintf(t, sizeof t, "800830DC,%08X,%08X", a0, (uint32_t)a1); note(t);
#ifdef CHK_HAVE_PAD_GEN23
  return (int)xch23(0x800830DCu, 1u, a0);
#else
  return (int)ret_of(0x800830DCu);
#endif
}
int pe_plat_card_sio_write_byte(uint32_t a0, int a1)
{ char t[64]; snprintf(t, sizeof t, "800832B4,%08X,%08X", a0, (uint32_t)a1); note(t);
#ifdef CHK_HAVE_PAD_GEN23
  return (int)xch23(0x800832B4u, 0u, a0);
#else
  return (int)ret_of(0x800832B4u);
#endif
}
#endif
#ifdef CHK_HAVE_PAD_GEN23
/* cdgen23: the SIO acknowledge wait 0x800834E8 (pe_plat_card_gen21.c in the
 * port) and the RX-ready wait 0x80083578; neither takes an argument. */
int pe_plat_card_sio_ack_wait(void)
{
    static const uint32_t pos[4] = {1u, 0xFFFFFFFFu, 0x80000000u, 0x100u};
    static const uint32_t cnt[8] = {0u, 1u, 2u, 3u, 5u, 0xFFFFFFFFu, 0x80000000u, 8u};
    static const uint32_t pair[16] = {0xFFFFFFFEu, 0xFFFFFFFFu, 0u, 1u, 2u, 3u, 4u, 5u, 6u, 0x7FFFFFFFu,
                                      0x80000000u, 3u, 4u, 1u, 0u, 2u};
    static const uint32_t slots[5][2] = {{0x8009B730u, 0x80130D00u}, {0x8009B72Cu, 0x801309C0u},
        {0x8009B744u, 0x80130D40u}, {0x8009B748u, 0x80130D80u}, {0x8009B724u, 0x80130DC0u}};
    uint32_t code = ret_of(0x800834E8u), k = code & 15u, m = (code >> 8) & 15u;
    unsigned n, i;
    note("800834E8");
    n = seq23_call(2u);
    if (n == m) {
        PE_StoreU32(0x8009B79Cu, cnt[(code >> 12) & 7u]);
        PE_StoreU32(0x8009B77Cu, pair[(code >> 16) & 15u]);
        PE_StoreU32(0x8009B780u, pair[(code >> 16) & 15u]);
        if ((code >> 20) & 1u)   /* absolute forms: bit 21 + i = alias */
            for (i = 0; i < 5; i++)
                PE_StoreU32(slots[i][0], ((code >> (21u + i)) & 1u) ? (slots[i][1] & 0x1FFFFFFFu) : slots[i][1]);
    }
    return (int)((k == 15u || n <= k) ? pos[(code >> 4) & 3u] : 0u);
}
void pe_plat_card_sio_rx_wait(void) { note("80083578"); }

/* cdgen23 v3: the slot 0x8009B744 (sentinel or alias) writes both pair
 * words on its call m (RETS m | pv << 4). */
static void blk23(void)
{
    static const uint32_t pair[16] = {0xFFFFFFFEu, 0xFFFFFFFFu, 0u, 1u, 2u, 3u, 4u, 5u, 6u, 0x7FFFFFFFu,
                                      0x80000000u, 3u, 4u, 1u, 0u, 2u};
    uint32_t code = ret_of(0x80130D40u);
    if (seq23_call(3u) == (code & 15u)) {
        PE_StoreU32(0x8009B77Cu, pair[(code >> 4) & 15u]);
        PE_StoreU32(0x8009B780u, pair[(code >> 4) & 15u]);
    }
}
int pe_plat_pad_transfer(uint32_t obj);
#endif
#ifdef CHK_HAVE_PAD_GEN24
/* cdgen24: the SIO acknowledge wait 0x800834E8 (no argument). */
int pe_plat_card_sio_ack_wait(void) { note("800834E8"); return (int)ret_of(0x800834E8u); }
void pe_plat_pad_transfer_step(uint32_t obj);
#endif
#ifdef CHK_HAVE_GPU_GEN9
/* cdgen9 callees, bound in the port to these pe_plat providers. */
uint16_t pe_plat_irq_set_mask(uint16_t new_mask)
{ char t[64]; snprintf(t, sizeof t, "80073E10,%08X", (uint32_t)new_mask); note(t); return (uint16_t)ret_of(0x80073E10u); }
void pe_plat_fill_bytes(pe_plat_guest_addr dst, uint8_t value, int32_t count)
{ char t[64]; snprintf(t, sizeof t, "80077A28,%08X,%08X,%08X", (uint32_t)dst, (uint32_t)value, (uint32_t)count); note(t); }
#ifndef CHK_HAVE_GPU_GEN
int pe_plat_gpu_2b04(int a0)
{ char t[64]; snprintf(t, sizeof t, "80077548,%08X", (uint32_t)a0); note(t); return (int)ret_of(0x80077548u); }
#endif
#endif
#if defined(CHK_HAVE_DS_GEN10) || defined(CHK_HAVE_DS_GEN38)
/* cdgen10 callees: the port's printf (all seven argument words recorded;
 * the vectors keep only those the format consumes) and 0x800822AC. */
int32_t PE_Printf_Guest(pe_addr_t fmt, uint32_t a1, uint32_t a2, uint32_t a3,
                        uint32_t a4, uint32_t a5, uint32_t a6, uint32_t a7)
{ char t[128]; snprintf(t, sizeof t, "80071A74,%08X,%08X,%08X,%08X,%08X,%08X,%08X,%08X", fmt, a1, a2, a3, a4, a5, a6, a7); note(t); return (int32_t)ret_of(0x80071A74u); }
int32_t pe_plat_cd_error_active(void)
{ note("800822AC"); return (int32_t)ret_of(0x800822ACu); }
#endif
#if defined(CHK_HAVE_DS_GEN11) || defined(CHK_HAVE_CD_POLL)
/* cdgen11 and CD-poll callees: VSync (the port's func_80073A44 is psx_compat.h's inline
 * wrapper of HostFB_VSync, stubbed here), the bound libds providers, and the
 * callback words, which the port calls through PE_GuestCall. */
uint32_t HostFB_VSync(int mode);
uint32_t HostFB_VSync(int mode)
{ char t[64]; snprintf(t, sizeof t, "80073A44,%08X", (uint32_t)mode); note(t); if (cur_entry==0x800811E4u && nrets==5) PE_StoreU32(0x8009B6C4u, rets_list[3]); return ret_of(0x80073A44u); }
int32_t pe_plat_cd_ds_get_sector2(uint32_t buffer, int32_t words)
{ char p[24], t[64]; fmt_got_ptr(p, sizeof p, buffer); snprintf(t, sizeof t, "80080B04,%s,%08X", p, (uint32_t)words); note(t); return (int32_t)ret_of(0x80080B04u); }
int32_t pe_plat_cd_queue_len(void) { note("8007F778"); return (int32_t)ret_of(0x8007F778u); }
#ifndef CHK_HAVE_CD_GEN4
void pe_plat_ds_81268(void) { note("80081268"); if (cur_entry==0x800811E4u && nrets==5) PE_StoreU32(0x8009B6B4u,ret_of(0x80081268u)^0x31415926u); }
#endif
#ifndef CHK_HAVE_CD_GEN5
void pe_plat_ds_text_490(void) { note("80082204"); }
#endif
#endif
#ifdef CHK_HAVE_CD_RETRY
void pe_plat_cd_retry_command(void);
void pe_plat_cd_flush(void) {
    note("8007B9EC");
    if (nrets==7) {
        PE_StoreU32(0x8009B59Cu,rets_list[3]);
        PE_StoreU8(0x8009B558u,(uint8_t)rets_list[2]);
        PE_StoreU32(0x8009B5A4u+4u*rets_list[2],rets_list[4]);
        PE_StoreU32(0x8009B560u,rets_list[5]);
    }
}
/* Test-only command model. Compile the retry provider with
 * -Dfunc_8007B558=pe_test_cd_command_model for this oracle checker. */
int pe_test_cd_command_model(uint32_t cmd,uint32_t data,pe_addr_t dst,uint32_t mode) {
    char t[96];snprintf(t,sizeof t,"8007B558,%08X,%08X,%08X,%08X",cmd,data,dst,mode);
    note(t);return (int32_t)ret_of(0x8007B558u);
}
#endif
#ifdef CHK_HAVE_CD_POLL
void HostFB_GetState(int *v,int *d,int *p,int *m) { (void)v;(void)d;(void)p;(void)m;abort(); }
int pe_plat_cd_read_poll(uint32_t fp);
#ifndef CHK_HAVE_CD_GEN5
int pe_plat_ds_ready(int arg)
{ char t[64]; snprintf(t,sizeof t,"8007F608,%08X",(uint32_t)arg); note(t);
  if (nrets==5) PE_StoreU32(0x8009B6B4u,ret_of(0x8007F608u)^0x27182818u);
  return (int32_t)ret_of(0x8007F608u); }
#endif
#endif
#if defined(CHK_HAVE_CD_GEN29) || defined(CHK_HAVE_CD_GEN33)
/* cdgen29 / cdgen33: the port's CD register wrappers (pe_cdreg.c) decode CD / DMA3
 * addresses into the device model; here every access is the plain value
 * access, so register reads replay and register stores are logged. */
uint8_t PE_CdLoadU8(pe_addr_t a) { return PE_LoadU8(a); }
void PE_CdStoreU8(pe_addr_t a, uint8_t v) { PE_StoreU8(a, v); }
uint32_t PE_CdLoadU32(pe_addr_t a) { return PE_LoadU32(a); }
void PE_CdStoreU32(pe_addr_t a, uint32_t v) { PE_StoreU32(a, v); }
int PE_CdReg_DeviceEnabled(void) { return 0; }
#endif
#if defined(CHK_HAVE_CD_GEN29) || defined(CHK_HAVE_CD_GEN33) || defined(CHK_HAVE_DS_GEN38) || defined(CHK_HAVE_CARD_GEN21) || defined(CHK_HAVE_CARD_GEN22) || defined(CHK_HAVE_PAD_GEN23) || defined(CHK_HAVE_PAD_GEN24)
/* cdgen21 / cdgen22 (and cdgen23 / cdgen24, whose callees are the waits): the port's poll tick (pe_plat_sio_tick.c) only
 * advances the host clock; the vectors have no counterpart, so it is a
 * no-op here. */
void pe_plat_sio_poll_tick(void) { }
#endif
#if defined(CHK_HAVE_CD_INIT_STEP) || defined(CHK_HAVE_CD_COMPLETE) || defined(CHK_HAVE_CD_GEN29) || defined(CHK_HAVE_CD_GEN33) || defined(CHK_HAVE_DS_GEN38) || defined(CHK_HAVE_CARD_GEN22) || defined(CHK_HAVE_PAD_GEN23) || defined(CHK_HAVE_PAD_GEN24)
/* A stuck-wait stop (port contract): no vector exhausts a wait budget, so a
 * call fails the case. */
void PE_Port_RequestStop(int reason);
void PE_Port_RequestStop(int reason) { (void)reason; bad_read = 1; }
#endif
#if defined(CHK_HAVE_CARD_GEN22) || defined(CHK_HAVE_PAD_GEN23) || defined(CHK_HAVE_PAD_GEN24) || defined(CHK_HAVE_PAD_GEN35)
/* cdgen22 / cdgen23 / cdgen24 / cdgen35: the RCNT2 timeout start 0x80084FC4 (bound in pe_plat/pad.h). */
void pe_plat_pad_rc2_wait_set(uint32_t limit)
{ char t[64]; snprintf(t, sizeof t, "80084FC4,%08X", limit); note(t); }
#endif
#if defined(CHK_HAVE_CARD_GEN21) || defined(CHK_HAVE_CARD_GEN22)
/* cdgen21 timeout check 0x80084FE4 (bound in pe_plat/pad.h): 0 for the
 * first j calls of a case, then WAIT_FINAL[f]. */
int pe_plat_pad_rc2_wait_check(void)
{
    static const uint32_t fin[3] = {1u, 0xFFFFFFFFu, 0x200u};
    static unsigned last_case, calls;
    uint32_t code = ret_of(0x80084FE4u), j = code & 0xFFu;
    if (last_case != cur) { last_case = cur; calls = 0; }
    calls++;
    note("80084FE4");
    return (int)(calls <= j ? 0u : fin[((code >> 8) & 0xFu) % 3u]);
}
#endif
#ifdef CHK_HAVE_DS_GEN16
/* cdgen16 read poll (0x800811E4, defined by pe_libcd.c in the port; -D
 * double here): positive for the first k calls of a case, then final. */
int chk_cd_read_poll(pe_addr_t fp);
int chk_cd_read_poll(pe_addr_t fp)
{
    static const uint32_t pos[3] = {1u, 0x7FFFFFFFu, 0x100u}, fin[3] = {0u, 0xFFFFFFFFu, 0x80000000u};
    static unsigned last_case, calls;
    uint32_t code = ret_of(0x800811E4u), k = code & 0xFFu;
    char t[64];
    if (last_case != cur) { last_case = cur; calls = 0; }
    calls++;
    snprintf(t, sizeof t, "800811E4,%08X", fp); note(t);
    return (int)(calls <= k ? pos[((code >> 8) & 0xFu) % 3u] : fin[((code >> 12) & 0xFu) % 3u]);
}
#endif
#if defined(CHK_HAVE_CARD_GEN13) || defined(CHK_HAVE_DS_GEN16) || defined(CHK_HAVE_API_GEN19) || defined(CHK_HAVE_PAD_GEN34)
int PE_Decomp_BoundaryFrom(const char *caller, const char *symbol, unsigned vma, unsigned nargs,
                           uintptr_t a0, uintptr_t a1, uintptr_t a2, uintptr_t a3)
{
    char t[128];
    uintptr_t a[4];
    unsigned i, n;
    (void)caller; (void)symbol;
    a[0] = a0; a[1] = a1; a[2] = a2; a[3] = a3;
    n = (unsigned)snprintf(t, sizeof t, "%08X", vma);
    for (i = 0; i < nargs && i < 4; i++) {
        if (vma == 0x80080E34u && i == 0) {   /* cdgen16: the position record is in the caller's frame */
            char p[24]; fmt_got_ptr(p, sizeof p, (uint32_t)a[i]);
            n += (unsigned)snprintf(t + n, sizeof t - n, ",%s", p);
        } else
            n += (unsigned)snprintf(t + n, sizeof t - n, ",%08X", (uint32_t)a[i]);
    }
    note(t);
#ifdef CHK_HAVE_PAD_GEN34
    if (vma == 0x8007E1E4u) write34(3, vma, 0);     /* cdgen34: the kernel queue calls' RETS codes */
    if (vma == 0x8007E1F4u) write34(4, vma, 0);
#endif
    return (int)ret_of(vma);
}
#endif
#ifdef CHK_HAVE_CARD_GEN13
/* cdgen13 callees: the bound critical-section, pad-clear and pad-state
 * providers, the port's libcard patch hand port (-D double), and the BIOS
 * card entries with no port implementation, which the native reaches
 * through the decomp boundary primitive (recorded with its argument count). */
int pe_plat_bios_enter_critical(void) { note("80072714"); return (int)ret_of(0x80072714u); }
void pe_plat_bios_exit_critical(void) { note("80072724"); }
void pe_plat_bios_change_clear_pad(uint32_t flag)
{ char t[64]; snprintf(t, sizeof t, "80073C74,%08X", flag); note(t); }
int32_t pe_plat_ctl_pad_initialized(void) { note("8007DEB0"); return (int32_t)ret_of(0x8007DEB0u); }
#ifndef CHK_HAVE_CARD_GEN15
void pe_plat_card_copy_patch(void) { note("8007E4E0"); }
#endif
#endif
#ifdef CHK_HAVE_PAD_GEN18
/* cdgen18 callees: the bound libpad providers. */
void pe_plat_pad_cmd_c(uint32_t record, unsigned char arg)
{ char t[64]; snprintf(t, sizeof t, "80083E50,%08X,%08X", record, (uint32_t)arg); note(t); }
void pe_plat_pad_send_step(uint32_t record)
{ char t[64]; snprintf(t, sizeof t, "800835C0,%08X", record); note(t); }
#endif
#if defined(CHK_HAVE_DS_GEN11) || defined(CHK_HAVE_GPU_GEN12) || defined(CHK_HAVE_CARD_GEN14) || defined(CHK_HAVE_PAD_GEN18) || \
    defined(CHK_HAVE_API_GEN19) || defined(CHK_HAVE_CARD_GEN20) || defined(CHK_HAVE_PAD_GEN26) || defined(CHK_HAVE_DS_GEN28)
/* Callback-word sentinels (or their KUSEG aliases) reached through the
 * port's guest dispatcher, recorded with the sentinel's argument count. */
int PE_GuestCall(const char *site, pe_addr_t fn, unsigned arity,
                 uintptr_t a0, uintptr_t a1, uintptr_t a2, uintptr_t a3)
{
    char t[96];
    (void)site; (void)arity;
    if ((fn & 0x1FFFFFFFu) == 0x00130A80u)   /* no arguments */
        snprintf(t, sizeof t, "%08X", fn);
    else if ((fn & 0x1FFFFFFFu) == 0x00130C40u || (fn & 0x1FFFFFFFu) == 0x00130C80u)   /* cdgen20: no argument */
        snprintf(t, sizeof t, "%08X", fn);
    else if ((fn & 0x1FFFFFFFu) == 0x00130AC0u || (fn & 0x1FFFFFFFu) == 0x00130C00u ||
             (fn & 0x1FFFFFFFu) == 0x00131100u)   /* one argument (cdgen26: the +0x18 hook) */
        snprintf(t, sizeof t, "%08X,%08X", fn, (uint32_t)a0);
    else if ((fn & 0x1FFFFFFFu) == 0x00130A40u)   /* four arguments */
        snprintf(t, sizeof t, "%08X,%08X,%08X,%08X,%08X", fn, (uint32_t)a0, (uint32_t)a1, (uint32_t)a2, (uint32_t)a3);
    else if ((fn & 0x1FFFFFFFu) == 0x00130A00u && cur_entry == 0x80081E70u) {   /* cdgen28: a2 is a frame pointer */
        char p[24]; fmt_got_ptr(p, sizeof p, (uint32_t)a2);
        snprintf(t, sizeof t, "%08X,%08X,%08X,%s", fn, (uint32_t)a0, (uint32_t)a1, p);
    }
    else if ((fn & 0x1FFFFFFFu) == 0x00130A00u || (fn & 0x1FFFFFFFu) == 0x00130B00u)   /* three arguments */
        snprintf(t, sizeof t, "%08X,%08X,%08X,%08X", fn, (uint32_t)a0, (uint32_t)a1, (uint32_t)a2);
    else
        snprintf(t, sizeof t, "%08X,%08X,%08X", fn, (uint32_t)a0, (uint32_t)a1);
    note(t);
#ifdef CHK_HAVE_PAD_GEN26
    if ((fn & 0x1FFFFFFFu) == 0x00131100u) return (int)state26(0x80131100u, (uint32_t)a0, 1);
#endif
#ifdef CHK_HAVE_DS_GEN28
    if ((fn & 0x1FFFFFFFu) == 0x00130A00u && cur_entry == 0x80081E70u) { cb28(); return 0; }
#endif
    return (int)ret_of((fn & 0x1FFFFFFFu) | 0x80000000u);
}
#endif
/* printf tokens: the vector's fields (format + consumed arguments) must be a
 * field-wise prefix of the native's, and the store counts must agree. */
static int printf_token_match(const char *got, const char *want)
{
    const char *ga = strchr(got, '@'), *wa = strchr(want, '@');
    size_t wn;
    if (!ga || !wa || strcmp(ga, wa)) return 0;
    wn = (size_t)(wa - want);
    if ((size_t)(ga - got) < wn || strncmp(got, want, wn)) return 0;
    return got[wn] == ',' || got[wn] == '@';
}
void PE_Event_Deliver(uint32_t cls, uint32_t spec)
{ char t[64]; snprintf(t, sizeof t, "80073A34,%08X,%08X", cls, spec); note(t); }
void pe_plat_spu_store_value(int index, unsigned int value, int shift_mode)
{ char t[64]; snprintf(t, sizeof t, "8007DAE0,%08X,%08X,%08X", (uint32_t)index, value, (uint32_t)shift_mode); note(t); }

static int call(uint32_t entry, uint32_t *a, uint32_t *ret)
{
    (void)a; (void)ret;   /* some build lines link only argument-free natives */
    switch (entry) {
#ifdef CHK_HAVE_CD_READ_STREAM
    case 0x80081314u:*ret=pe_plat_cd_read_stream(a[0],a[1]);return 1;
#endif
#ifdef CHK_HAVE_CD_ISSUE_STEP
    case 0x80080DC4u:wait_polls=0;*ret=pe_plat_cd_issue_step((int32_t)a[0],a[1],a[2]);return 1;
#endif
#ifdef CHK_HAVE_CD_COMMAND_WAIT
    case 0x80080D5Cu:wait_polls=0;*ret=pe_plat_cd_command_wait((int32_t)a[0],a[1],a[2]);return 1;
#endif
#ifdef CHK_HAVE_CD_ISSUE_WRAPPER
    case 0x8007FCFCu: *ret=pe_plat_cd_issue_wrapper(a[0],a[1]);return 1;
#endif
#ifdef CHK_HAVE_CD_INIT_STEP
    case 0x8007F994u: *ret=pe_plat_cd_initialize_callbacks_retail(); return 1;
#endif
#ifdef CHK_HAVE_CD_STATUS_CALLBACK
    case 0x80080164u: pe_plat_cd_status_callback(a[0],a[1]); return 0;
#endif
#ifdef CHK_HAVE_CD_COMPLETE
    case 0x80080220u: pe_plat_cd_command_complete(a[0],a[1]); return 0;
#endif
#ifdef CHK_HAVE_CD_UNEXPECTED
    case 0x8008068Cu: pe_plat_cd_unexpected_status(a[0],a[1]); return 0;
#endif
#ifdef CHK_HAVE_CD_GEN2
    case 0x800808BCu: pe_plat_cd_abort_cmd(); return 0;
    case 0x80081D74u: *ret = (uint32_t)pe_plat_ds_start_ready_system((int)a[0], (int)a[1]); return 1;
    case 0x80081DF8u: pe_plat_ds_end_ready_system(); return 0;
    case 0x800819D8u: *ret = (uint32_t)pe_plat_ds_text_5c4((int)a[0], a[1]); return 1;
#endif
#ifdef CHK_HAVE_SPU_MODETYPE
    case 0x8008CF70u: *ret = (uint32_t)pe_plat_spu_set_reverb_mode_type((int)a[0]); return 1;
#endif
#ifdef CHK_HAVE_MATH_ERR
    case 0x800739C4u: *ret = (uint32_t)pe_plat_math_err((int)a[0], (int)a[1]); return 1;
#endif
#ifdef CHK_HAVE_SPU_FIDMA
    case 0x8007D614u: pe_plat_spu_fidma(); return 0;
#endif
#ifdef CHK_HAVE_CARD_CLEAR
    case 0x8007DD74u: *ret = (uint32_t)pe_plat_card_clear(a[0]); return 1;
#endif
#ifdef CHK_HAVE_QSORT
    case 0x800723A4u: pe_plat_qsort(a[0], a[1], a[2], a[3]); return 0;
#endif
#ifdef CHK_HAVE_CD_GEN
    case 0x800813E8u: pe_plat_ds_813e8(); return 0;
    case 0x800822BCu: pe_plat_ds_er_clear(); return 0;
    case 0x80082400u: pe_plat_ds_text_ec((unsigned char)a[0]); return 0;
    case 0x80080F64u: pe_plat_cd_set_mode2_callback((int)a[0]); return 0;
    case 0x8007FB04u: pe_plat_cd_7fb04(); return 0;
    case 0x8007BBB0u: pe_plat_cd_init_intr(); return 0;
    case 0x8007C2A0u: *ret = (uint32_t)pe_plat_cd_st_get_backloc((int)a[0]); return 1;
#endif
#ifdef CHK_HAVE_CD_GEN3
    case 0x8007F69Cu: pe_plat_ds_flush(); return 0;
#endif
#ifdef CHK_HAVE_CD_GEN4
    case 0x80081268u: pe_plat_ds_81268(); return 0;
#endif
#ifdef CHK_HAVE_CD_GEN5
    case 0x8007FB44u: *ret = (uint32_t)pe_plat_cd_try_issue_cmd(a[0], a[1]); return 1;
    case 0x8007FCBCu: *ret = (uint32_t)pe_plat_cd_7fcbc((int)a[0], (int)a[1]); return 1;
    case 0x80080CDCu: *ret = pe_plat_ds_last_pos(a[0]); return 1;
    case 0x8008227Cu: pe_plat_ds_text_508((unsigned char)a[0]); return 0;
    case 0x80082204u: pe_plat_ds_text_490(); return 0;
    case 0x8007F608u: *ret = (uint32_t)pe_plat_ds_ready((int)a[0]); return 1;
#endif
#ifdef CHK_HAVE_GPU_GEN
    case 0x80077548u: *ret = (uint32_t)pe_plat_gpu_2b04((int)a[0]); return 1;
    case 0x80077A00u: pe_plat_gpu_2fbc(); return 0;
    case 0x80076C10u: pe_plat_gpu_21cc((int)a[0], (int)a[1], (int)a[2]); return 0;
#endif
#ifdef CHK_HAVE_CD_GEN6
    case 0x80082444u: pe_plat_ds_text_130((unsigned char)a[0]); return 0;
#endif
#ifdef CHK_HAVE_CD_GEN7
    case 0x8007F960u: pe_plat_cd_text_13cc(a[0], a[1]); return 0;
#endif
#ifdef CHK_HAVE_CARD_GEN8
    case 0x800840DCu: *ret = (uint32_t)pe_plat_card_840dc(a[0]); return 1;
#endif
#ifdef CHK_HAVE_GPU_GEN9
    case 0x80077144u: *ret = (uint32_t)pe_plat_gpu_2700((int)a[0]); return 1;
#endif
#ifdef CHK_HAVE_DS_GEN10
    case 0x800809E0u: pe_plat_ds_status(); return 0;
#endif
#ifdef CHK_HAVE_CARD_GEN22
    case 0x800830DCu: *ret = (uint32_t)pe_plat_card_sio_exchange(a[0], (int)a[1]); return 1;
    case 0x800832B4u: *ret = (uint32_t)pe_plat_card_sio_write_byte(a[0], (int)a[1]); return 1;
#endif
#ifdef CHK_HAVE_DS_GEN28
    case 0x80081E70u: pe_plat_ds_ready_callback((unsigned char)a[0], a[1]); return 0;
#endif
#ifdef CHK_HAVE_CD_GEN29
    case 0x8007C044u: *ret = (uint32_t)pe_plat_cd_getsector2(a[0], a[1]); return 1;
#endif
#ifdef CHK_HAVE_DS_GEN30
    case 0x8007E8F4u: *ret = (uint32_t)pe_plat_ds_text_360(); return 1;
#endif
#ifdef CHK_HAVE_CD_RETRY
    case 0x800800F4u: pe_plat_cd_retry_command(); return 0;
#endif
#ifdef CHK_HAVE_CD_POLL
    case 0x800811E4u: *ret=(uint32_t)pe_plat_cd_read_poll(a[0]); return 1;
#endif
#ifdef CHK_HAVE_DS_GEN39
    case 0x8007E964u: pe_plat_ds_text_3d0(a[0], a[1]); return 0;
#endif
#ifdef CHK_HAVE_DS_GEN39
    case 0x8007E704u: pe_plat_ds_text_170(a[0], a[1]); return 0;
#endif
#ifdef CHK_HAVE_DS_GEN38
    case 0x80082314u: *ret = (uint32_t)pe_plat_ds_get_disk_type(); return 1;
#endif
#ifdef CHK_HAVE_CD_GEN33
    case 0x800824F0u: *ret = pe_plat_ds_data_callback(a[0]); return 1;
    case 0x8007F72Cu: *ret = (uint32_t)pe_plat_ds_system_status(); return 1;
    case 0x8007C13Cu: pe_plat_cd_intr_poll(); return 0;
#endif
#ifdef CHK_HAVE_CD_GEN32
    case 0x8007C304u: pe_plat_cd_set_stream(a[0], (int32_t)a[1], (int32_t)a[2], a[3], a[4]); return 0;
#endif
#ifdef CHK_HAVE_DS_GEN31
    case 0x8007EB88u: pe_plat_ds_text_5f4(a[0], a[1], a[2]); return 0;
#endif
#ifdef CHK_HAVE_PAD_GEN26
    case 0x80084C4Cu: pe_plat_pad_record_step(a[0]); return 0;
#endif
#ifdef CHK_HAVE_PAD_GEN34
    case 0x80082CF0u: *ret = (uint32_t)pe_plat_pad_start_com(); return 1;
#endif
#ifdef CHK_HAVE_PAD_GEN35
    case 0x80082E00u: rec35 = a[0]; *ret = (uint32_t)pe_plat_pad_sio_start(a[0]); return 1;
#endif
#ifdef CHK_HAVE_CARD_GEN25
    case 0x80082B70u: *ret = (uint32_t)pe_plat_card_timer_callback(); return 1;
#endif
#ifdef CHK_HAVE_PAD_GEN24
    case 0x80083014u: pe_plat_pad_transfer_step(a[0]); return 0;
#endif
#ifdef CHK_HAVE_PAD_GEN23
    case 0x80084168u: *ret = (uint32_t)pe_plat_pad_transfer(a[0]); return 1;
#endif
#ifdef CHK_HAVE_CARD_GEN21
    case 0x800834E8u: *ret = (uint32_t)pe_plat_card_sio_ack_wait(); return 1;
#endif
#ifdef CHK_HAVE_CARD_GEN20
    case 0x80083F04u: *ret = (uint32_t)pe_plat_card_obj_sync(a[0]); return 1;
    case 0x80083F44u: *ret = (uint32_t)pe_plat_card_obj_command(a[0]); return 1;
    case 0x8008401Cu: *ret = (uint32_t)pe_plat_card_obj_write(a[0]); return 1;
#endif
#ifdef CHK_HAVE_API_GEN19
    case 0x800727B4u: *ret = (uint32_t)pe_plat_api_firstfile(a[0], (int)a[1]); return 1;
    case 0x80072950u: *ret = (uint32_t)pe_plat_api_firstfile_hook(a[0], (int)a[1], (int)a[2]); return 1;
#endif
#ifdef CHK_HAVE_PAD_GEN18
    case 0x80084B78u: *ret = (uint32_t)pe_plat_pad_card_dispatch(a[0]); return 1;
#endif
#ifdef CHK_HAVE_CARD_GEN17
    case 0x80084EB0u: pe_plat_card_dir_fail_auto(a[0]); return 0;
#endif
#ifdef CHK_HAVE_DS_GEN16
    case 0x80081D18u: *ret = (uint32_t)pe_plat_ds_text_904((int)a[0], (int)a[1], (int)a[2]); return 1;
#endif
#ifdef CHK_HAVE_CARD_GEN15
    case 0x800719C4u: *ret = pe_plat_gpu_flag_field(a[0]); return 1;
    case 0x8007E4E0u: pe_plat_card_copy_patch(); return 0;
#endif
#ifdef CHK_HAVE_CARD_GEN14
    case 0x80082B08u: *ret = (uint32_t)pe_plat_card_slot_poll(); return 1;
    case 0x80083DF0u: *ret = (uint32_t)pe_plat_card_obj_reset(a[0]); return 1;
#endif
#ifdef CHK_HAVE_CARD_GEN13
    case 0x8007DDD4u: pe_plat_card_init((int)a[0]); return 0;
    case 0x8007DE40u: pe_plat_card_start(); return 0;
#endif
#ifdef CHK_HAVE_GPU_GEN12
    case 0x80074BB8u: *ret = (uint32_t)pe_plat_gpu_set_graph_debug((unsigned char)a[0]); return 1;
#endif
#ifdef CHK_HAVE_DS_GEN11
    case 0x80080F98u: pe_plat_ds_read_progress((int)a[0], (int)a[1]); return 0;
    case 0x8008214Cu: pe_plat_ds_text_3d8((unsigned char)a[0], (int)a[1]); return 0;
#endif
#ifdef PE_CHECK_SETREVERB
    case 0x80085A64u: *ret = (uint32_t)pe_plat_spu_set_reverb_enabled((int)a[0]); return 1;
#endif
    }
    return -1;
}
static int apply(const char *t, uint8_t *dst, uint32_t *set, int first_only)
{
    unsigned addr, size, val, i;
    if (sscanf(t, "%x:%u=%x", &addr, &size, &val) != 3 || size == 0 || size > 4) return 0;
    for (i = 0; i < size; i++) {
        uint32_t b = (addr + i) & (MEM - 1);
        if (set[b] == cur) { if (first_only) continue; }
        else if (!first_only && nxlog < 4096) xlog[nxlog++] = b;
        dst[b] = (uint8_t)(val >> (8 * i));
        set[b] = cur;
    }
    return 1;
}
static int find(char **tok, int nt, int from, const char *s)
{
    while (from < nt && strcmp(tok[from], s)) from++;
    return from;
}
int main(int c, char **v)
{
    FILE *f; static char line[262144]; unsigned n = 0, bad = 0;
    if (c != 2 || !(f = fopen(v[1], "r"))) return 2;
    while (fgets(line, sizeof line, f)) {
        static char *tok[16384]; static char copy[262144];
        int nt = 0, i, ri, li, si, ci, has, ok = 1, na; uint32_t entry, a[8] = {0}, wret, got = 0; unsigned k;
        strncpy(copy, line, sizeof copy - 1); copy[sizeof copy - 1] = 0;
        while (nt < 16384 && (tok[nt] = strtok(nt ? NULL : copy, " \n"))) nt++;
        if (nt < 8 || strcmp(tok[3], "A")) continue;
        ri = find(tok, nt, 4, "RET"); li = ri + 2; si = find(tok, nt, li, "S");
        if (si >= nt || li >= nt || strcmp(tok[li], "L")) continue;
        entry = (uint32_t)strtoul(tok[0], 0, 16);
        na = ri - 4;
        for (i = 0; i < na && i < 8; i++) if (strcmp(tok[4 + i], "-")) a[i] = (uint32_t)strtoul(tok[4 + i], 0, 16);
        wret = (uint32_t)strtoul(tok[ri + 1], 0, 16);
        cur = ++n; nwlog = nxlog = 0; bad_read = 0; nmlog = 0; nrq = rq_pos = 0; ngot = 0; frame_top = PE_GUEST_FRAME_HI; nstores = 0;
        for (k = (unsigned)li + 1; k < (unsigned)si; k++) {
            unsigned addr, size, val;
            if (!strcmp(tok[k], "-")) continue;
            if (sscanf(tok[k], "%x:%u=%x", &addr, &size, &val) == 3 &&
                ((addr >> 16) == 0x1F80u
#ifdef CHK_HAVE_DS_GEN38
                 || addr == 0x800B28F8u
#endif
                )) {
                if (nrq < 8192) { rq[nrq][0] = addr; rq[nrq][1] = size; rq[nrq][2] = val; nrq++; }
                continue;      /* register read: replayed in order, not memory */
            }
            apply(tok[k], ram, readable, 1);
        }
        ci = si + 1;
        while (ci < nt && strcmp(tok[ci], "CALL")) ci++;
        rets_val = 0; want_fset = got_fset = 0;
        for (k = (unsigned)si + 1; k + 1 < (unsigned)ci; k++)
            if (!strcmp(tok[k], "RETS")) {
                char *e = tok[k + 1];
                nrets = 0;
                while (nrets < 12) { rets_list[nrets++] = (uint32_t)strtoul(e, &e, 16); if (*e != ',') break; e++; }
                rets_val = rets_list[0];
            }
        for (k = (unsigned)si + 1; k < (unsigned)ci; k++) {
            if (!strcmp(tok[k], "RETS")) { k++; continue; }
            if (strcmp(tok[k], "-")) apply(tok[k], want, want_set, 0);
        }
        if (entry==0x800811E4u && nrets==5) {
            for (unsigned j=0;j<4;j++) {
                ram[0x9B6C4u+j]=(uint8_t)(0xdeadbeefu>>(8*j)); readable[0x9B6C4u+j]=cur;
                ram[0x9B6B4u+j]=(uint8_t)(0x12345678u>>(8*j)); readable[0x9B6B4u+j]=cur;
            }
        }
        if (entry==0x800800F4u && nrets==7) {
            for (unsigned a=0x9B5A4u;a<0x9B9A4u;a++) { ram[a]=0;readable[a]=cur; }
            for (unsigned j=0;j<4;j++) {
                ram[0x9B59Cu+j]=ram[0x9B560u+j]=0;
                readable[0x9B59Cu+j]=readable[0x9B560u+j]=cur;
            }
            ram[0x9B558u]=0;readable[0x9B558u]=cur;
        }
        if (entry==0x80080164u && nrets==6) {
            uint32_t i=rets_list[4];
            const uint32_t states[]={0,31,32,0x11f,0x80000000u};
            ram[0x9B56C]=(uint8_t)i;readable[0x9B56C]=cur;
            const uint32_t addresses[]={0x9B570,0x9B598,0x9B59C};
            const uint32_t values[]={states[i%5],0xdeadbeefu,0x12345678u};
            for(unsigned j=0;j<3;j++)for(unsigned k=0;k<4;k++){
                ram[addresses[j]+k]=(uint8_t)(values[j]>>(8*k));readable[addresses[j]+k]=cur;
            }
        }
        cur_entry = entry;
        runaway_reads = 0;
        if (setjmp(case_escape)) { case_escape_armed = 0; bad_read = 1; has = 0; got = 0; }
        else { case_escape_armed = 1; has = call(entry, a, &got); case_escape_armed = 0; }
        if (has < 0) { fprintf(stderr, "unknown entry %08X\n", entry); return 2; }
        if (has && got != wret) ok = 0;
        if (bad_read) ok = 0;
        if (nwlog != nxlog) ok = 0;
        if (rq_pos != nrq) ok = 0;     /* every register read the original made */
        {   /* callee calls: same order; arguments per callee */
            unsigned want_c = 0;
            for (k = (unsigned)ci + 1; k < (unsigned)nt; k++) {
                char w[320], tk[300], *at; unsigned fn = 0, x0 = 0, x1 = 0, x2 = 0, x3 = 0, x4 = 0, cnt = 0;
                if (!strcmp(tok[k], "-")) continue;
                strncpy(tk, tok[k], sizeof tk - 1); tk[sizeof tk - 1] = 0;
                if ((at = strchr(tk, '@')) != NULL) {
                    /* distinct bytes among the original's first n stores */
                    unsigned nst = (unsigned)strtoul(at + 1, 0, 10), q, b, seen = 0;
                    static uint32_t mark[MEM]; static uint32_t gen;
                    gen++;
                    for (q = (unsigned)si + 1; q < (unsigned)ci && nst; q++) {
                        unsigned ad, sz, vv;
                        if (!strcmp(tok[q], "RETS")) break;
                        if (sscanf(tok[q], "%x:%u=%x", &ad, &sz, &vv) != 3) continue;
                        for (b = 0; b < sz; b++) {
                            uint32_t m = (ad + b) & (MEM - 1);
                            if (mark[m] != gen) { mark[m] = gen; seen++; }
                        }
                        nst--;
                    }
                    cnt = seen;
                    *at = 0;
                }
                sscanf(tk, "%x,%x,%x,%x,%x,%x", &fn, &x0, &x1, &x2, &x3, &x4);
                if (fn == 0x80071A74u) {   /* printf: raw fields, prefix match below */
                    snprintf(w, sizeof w, "%s@%u", tk, cnt);
                    if (want_c >= ngot || (want_c < MAX_CALLS && !printf_token_match(got_calls[want_c], w))) ok = 0;
                    want_c++;
                    continue;
                }
                if (entry==0x80080D5Cu || entry==0x80080DC4u) {
                    if(fn==0x8007EE84u)snprintf(w,sizeof w,"%08X,%08X,%08X,%08X,%08X@%u",fn,x0,x1,x2,x3,cnt);
                    else snprintf(w,sizeof w,"%08X,%08X,%08X@%u",fn,x0,x1,cnt);
                }
                else if (entry==0x8007FCFCu) {
                    if(fn==0x8007B9ECu)snprintf(w,sizeof w,"%08X@%u",fn,cnt);
                    else if(fn==0x80080950u)snprintf(w,sizeof w,"%08X,%08X,%08X@%u",fn,x0,x1,cnt);
                    else snprintf(w,sizeof w,"%08X,%08X,%08X,%08X,%08X@%u",fn,x0,x1,x2,x3,cnt);
                }
                else if (entry==0x8007F994u) {
                    if(fn==0x80073D58u)snprintf(w,sizeof w,"%08X,%08X,%08X@%u",fn,x0,x1,cnt);
                    else if(fn==0x800812F4u)snprintf(w,sizeof w,"%08X,%08X@%u",fn,x0,cnt);
                    else snprintf(w,sizeof w,"%08X@%u",fn,cnt);
                }
                else if (entry==0x80080164u || entry==0x80080220u || entry==0x8008068Cu)
                    snprintf(w,sizeof w,"%08X,%08X,%08X@%u",fn,x0,x1,cnt);
                else if (fn == 0x8008D140u) {
                    const char *blob = strrchr(tk, ',');
                    snprintf(w, sizeof w, "8008D140,%s@%u", blob ? blob + 1 : "", cnt);
                } else if (fn == 0x8007DAE0u) snprintf(w, sizeof w, "8007DAE0,%08X,%08X,%08X@%u", x0, x1, x2, cnt);
                else if (fn == 0x8007C544u) snprintf(w, sizeof w, "%08X,%08X,%08X,%08X@%u", fn, x0, x1, x2, cnt);
                else if (fn == 0x80073A34u) snprintf(w, sizeof w, "80073A34,%08X,%08X@%u", x0, x1, cnt);
                else if (fn == 0x8007DCACu) snprintf(w, sizeof w, "8007DCAC@%u", cnt);
                else if (fn == 0x8007DDB4u) snprintf(w, sizeof w, "8007DDB4,%08X,%08X,%08X@%u", x0, x1, x2, cnt);
                else if (fn == 0x80077A28u) snprintf(w, sizeof w, "80077A28,%08X,%08X,%08X@%u", x0, x1 & 0xFFu, x2, cnt);
                else if (fn == CMP_ASC || fn == CMP_DESC) snprintf(w, sizeof w, "%08X,%08X,%08X@%u", fn, x0, x1, cnt);
                else if (fn==0x8007B558u && entry==0x800800F4u)
                    snprintf(w,sizeof w,"%08X,%08X,%08X,%08X,%08X@%u",fn,x0,x1,x2,x3,cnt);
                else if (fn == 0x8007C564u || fn == 0x8007B9ECu || fn == 0x80073C94u || fn == 0x800808BCu ||
                         fn == 0x80081DF8u || fn == 0x80080930u || fn == 0x8007F69Cu || fn == 0x800822BCu ||
                         fn == 0x800822ACu || fn == 0x80081268u || fn == 0x8007F778u || fn == 0x80082204u ||
                         fn == 0x80072714u || fn == 0x80072724u || fn == 0x8007DEB0u || fn == 0x8007E4E0u ||
                         fn == 0x8007E3DCu || fn == 0x8007E470u || fn == 0x8007E324u)
                    snprintf(w, sizeof w, "%08X@%u", fn, cnt);
                else if (fn == 0x8007FCBCu || fn == 0x8007FCFCu || fn == 0x80080998u || fn == 0x80073CF4u ||
                         fn == 0x80072A64u || fn == 0x800830DCu)
                    snprintf(w, sizeof w, "%08X,%08X,%08X@%u", fn, x0, x1, cnt);
                else if (fn == 0x8007FC28u || fn == 0x8007FC18u || fn == 0x8007FC44u) snprintf(w, sizeof w, "%08X@%u", fn, cnt);
                else if (fn == 0x80080C48u && entry == 0x80081E70u) { char p[24]; fmt_want_ptr(p, sizeof p, x0); snprintf(w, sizeof w, "80080C48,%s@%u", p, cnt); }
                else if (fn == 0x80080AE4u) { char p[24]; fmt_want_ptr(p, sizeof p, x0); snprintf(w, sizeof w, "80080AE4,%s,%08X@%u", p, x1, cnt); }
                else if (fn == 0x80080B04u) { char p[24]; fmt_want_ptr(p, sizeof p, x0); snprintf(w, sizeof w, "80080B04,%s,%08X@%u", p, x1, cnt); }
                else if (fn == 0x80083E50u) snprintf(w, sizeof w, "80083E50,%08X,%08X@%u", x0, x1 & 0xFFu, cnt);
                else if (fn == 0x80080B44u) { char p[24]; fmt_want_ptr(p, sizeof p, x1); snprintf(w, sizeof w, "80080B44,%08X,%s@%u", x0, p, cnt); }
                else if (fn == 0x80080E34u) { char p[24]; fmt_want_ptr(p, sizeof p, x0); snprintf(w, sizeof w, "80080E34,%s,%08X,%08X,%08X@%u", p, x1, x2, x3, cnt); }
                else if ((fn & 0x1FFFFFFFu) == 0x001309E0u) snprintf(w, sizeof w, "%08X,%08X,%08X@%u", fn, x0, x1, cnt);
                else if ((fn & 0x1FFFFFFFu) == 0x00130A00u && entry == 0x80081E70u) {   /* cdgen28: frame-relative a2 */
                    char p[24]; fmt_want_ptr(p, sizeof p, x2);
                    snprintf(w, sizeof w, "%08X,%08X,%08X,%s@%u", fn, x0, x1, p, cnt);
                }
                else if ((fn & 0x1FFFFFFFu) == 0x00130A00u || (fn & 0x1FFFFFFFu) == 0x00130B00u)
                    snprintf(w, sizeof w, "%08X,%08X,%08X,%08X@%u", fn, x0, x1, x2, cnt);
                else if ((fn & 0x1FFFFFFFu) == 0x00130A40u) snprintf(w, sizeof w, "%08X,%08X,%08X,%08X,%08X@%u", fn, x0, x1, x2, x3, cnt);
                else if ((fn & 0x1FFFFFFFu) == 0x00130A80u || (fn & 0x1FFFFFFFu) == 0x00130C40u ||
                         (fn & 0x1FFFFFFFu) == 0x00130C80u || fn == 0x80084FE4u ||
                         fn == 0x800834E8u || fn == 0x80083578u || fn == 0x8007AAB4u || fn == 0x8007F72Cu || fn == 0x8007E5C4u)   /* no argument: $a0 there is leftover */
                    snprintf(w, sizeof w, "%08X@%u", fn, cnt);
                else if (((fn & 0x1FFFFFFFu) == 0x00130AC0u || (fn & 0x1FFFFFFFu) == 0x00131240u ||
                          (fn & 0x1FFFFFFFu) == 0x00131280u) &&
                         entry != 0x8007E704u && entry != 0x8007E964u)   /* cdgen39 also dispatches 0x…131240 (three fields, below) */
                    snprintf(w, sizeof w, "%08X,%08X@%u", fn, x0, cnt);
                else if (fn == 0x80073C84u || fn == 0x8007E1E4u || fn == 0x8007E1F4u) snprintf(w, sizeof w, "%08X,%08X,%08X@%u", fn, x0, x1, cnt);
                else if (fn == 0x80071A04u) { char p[24]; fmt_want_ptr(p, sizeof p, x0); snprintf(w, sizeof w, "80071A04,%s,%08X,%08X@%u", p, x1, x2, cnt); }
                else if (fn == 0x8007F0C8u && entry == 0x80082314u) {   /* cdgen38: the record is in the caller's frame */
                    char p[24]; fmt_want_ptr(p, sizeof p, x1);
                    snprintf(w, sizeof w, "8007F0C8,%08X,%s,%08X,%08X,%08X@%u", x0, p, x2, x3, x4, cnt);
                }
                else if (fn == 0x8007F0C8u) snprintf(w, sizeof w, "8007F0C8,%08X,%08X,%08X,%08X,%08X@%u", x0, x1, x2, x3, x4, cnt);
                else if ((fn & 0x1FFFFFFFu) == 0x001309C0u || fn == 0x800832B4u) snprintf(w, sizeof w, "%08X,%08X,%08X@%u", fn, x0, x1, cnt);
                else if ((fn & 0x1FFFFFFFu) == 0x00131140u || (fn & 0x1FFFFFFFu) == 0x00131180u)   /* cdgen33 callbacks: status, response */
                    snprintf(w, sizeof w, "%08X,%08X,%08X@%u", fn, x0, x1, cnt);
                else if (fn == 0x8007EB88u) snprintf(w, sizeof w, "%08X,%08X,%08X,%08X@%u", fn, x0, x1, x2, cnt);
                else if ((fn & 0x1FFFFFFFu) == 0x00131200u || (fn & 0x1FFFFFFFu) == 0x00131240u ||
                         (fn & 0x1FFFFFFFu) == 0x00131300u || (fn & 0x1FFFFFFFu) == 0x00131340u ||
                         (fn & 0x1FFFFFFFu) == 0x00131380u || (fn & 0x1FFFFFFFu) == 0x001313C0u)   /* cdgen39 / cdgen39 callbacks: status, response */
                    snprintf(w, sizeof w, "%08X,%08X,%08X@%u", fn, x0, x1, cnt);
                else if (fn == 0x80130980u) snprintf(w, sizeof w, "80130980,%08X,%08X@%u", x0, x1, cnt);
                else if (fn == 0x80076C34u) snprintf(w, sizeof w, "%08X,%08X,%08X,%08X,%08X@%u", fn, x0, x1, x2, x3, cnt);
                else if (fn == 0x8007EE84u) snprintf(w, sizeof w, "%08X,%08X,%08X,%08X,%08X@%u", fn, x0, x1, x2, x3, cnt);
                else if (fn == 0x80073D58u || fn == 0x80073CC4u || fn == 0x8007A930u || fn == 0x80081D74u ||
                         fn == 0x80072A54u || fn == 0x8007FB44u)
                    snprintf(w, sizeof w, "%08X,%08X,%08X@%u", fn, x0, x1, cnt);
                else snprintf(w, sizeof w, "%08X,%08X@%u", fn, x0, cnt);
                if (want_c >= ngot || (want_c < MAX_CALLS && strcmp(got_calls[want_c], w))) ok = 0;
                want_c++;
            }
            if (want_c != ngot) ok = 0;
        }
        {   /* register stores: same sequence as the original's */
            unsigned want_m = 0, addr, size, val;
            for (k = (unsigned)si + 1; k < (unsigned)ci; k++) {
                if (sscanf(tok[k], "%x:%u=%x", &addr, &size, &val) != 3 || (addr >> 16) != 0x1F80u) continue;
                if (want_m >= nmlog || mlog[want_m][0] != addr || mlog[want_m][1] != size ||
                    mlog[want_m][2] != (val & (size == 4 ? 0xFFFFFFFFu : (1u << (8 * size)) - 1u))) ok = 0;
                want_m++;
            }
            if (want_m != nmlog) ok = 0;
        }
        for (k = 0; k < nwlog && ok; k++)
            if (want_set[wlog[k]] != cur || ram[wlog[k]] != want[wlog[k]]) ok = 0;
        if (!ok && bad < 3) {
            unsigned q;
            for (q = 0; q < ngot && q < MAX_CALLS; q++) {
                char *c = got_calls[q], *blob = strstr(c, "8008D140,");
                if (blob) fprintf(stderr, "  got 8008D140,<record>%s\n", strchr(c, '@') ? strchr(c, '@') : "");
                else fprintf(stderr, "  got %s\n", c);
            }
        }
        if (!ok && bad++ < 12)
            fprintf(stderr, "MISMATCH line %u (%s%s): %.200s\n", n, bad_read ? "read outside original loads; " : "",
                    "stores/return", line);
    }
    printf("checked %u cases, %u mismatches\n", n, bad);
    return bad != 0;
}
