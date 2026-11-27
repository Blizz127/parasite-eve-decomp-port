cmake_minimum_required(VERSION 3.16)
project(parasite_eve_port C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# ---------------------------------------------------------------------------
# Phase 6D-S — Native Boot-to-Black Scaffold with host-safe guest memory
# ---------------------------------------------------------------------------

# Sanitizer option
option(PE_PORT_SANITIZERS "Build with AddressSanitizer and UBSan" OFF)

# Source lists
set(PLATFORM_SRCS
    platform/pe_plat_gte.c
    platform/pe_plat_gte_regs.c
    platform/pe_plat_gte_stack.c
    platform/pe_plat_gte_trig.c
    platform/pe_plat_gte_trig_host.c
    platform/pe_plat_gte_cmd.c
    platform/pe_plat_gte_cmd2.c
    platform/pe_plat_gte_div.c
    platform/pe_plat_gte_vn.c
    platform/pe_plat_gte_init.c
    platform/pe_plat_ds_leaf.c
    platform/pe_plat_cd_data_ready.c
    platform/pe_plat_cd_poll.c
    platform/pe_plat_cd_retry.c
    platform/pe_plat_cd_initialize_callbacks.c
    platform/pe_plat_cd_issue_wrapper.c
    platform/pe_plat_cd_command_wait_host.c
    platform/pe_plat_cd_issue_step_host.c
    platform/pe_plat_cd_read_stream_host.c
    platform/pe_plat_cd_public_init_host.c
    platform/pe_plat_cd_reset_host.c
    platform/pe_plat_cd_response_query_host.c
    platform/pe_plat_cd_command_sequence_host.c
    platform/pe_plat_cd_queue_command_host.c
    platform/pe_plat_cd_search_file_host.c
    platform/pe_plat_cd_ack_host.c
    platform/pe_plat_cd_status_poll_host.c
    platform/pe_plat_cd_command_issue_host.c
    platform/pe_plat_cd_sector_dma_host.c
    platform/pe_plat_cd_stream_handler_host.c
    platform/pe_plat_cd_dma_issue_host.c
    platform/pe_plat_cd_status_callback.c
    platform/pe_plat_cd_command_complete.c
    platform/pe_plat_cd_unexpected_status.c
    platform/pe_plat_cd_leaf.c
    platform/pe_plat_cd_ring_host.c
    platform/pe_plat_spu_reverb.c
    platform/pe_plat_spu_modetype.c
    platform/pe_plat_math_err.c
    platform/pe_plat_spu_fidma.c
    platform/pe_plat_card_clear.c
    platform/pe_plat_qsort.c
    platform/pe_plat_card_leaf.c
    platform/pe_plat_gte_lv.c
    platform/pe_plat_pad_rc2.c
    platform/pe_vlc_decode.c
    platform/pe_plat_vlc_frame.c
    platform/pe_plat_mdec_io.c
    platform/pe_plat_mdec_io_host.c
    platform/pe_plat_card_sio.c
    platform/pe_plat_cd_gen.c
    platform/pe_plat_cd_gen2.c
    platform/pe_plat_cd_gen3.c
    platform/pe_plat_cd_gen4.c
    platform/pe_plat_cd_gen5.c
    platform/pe_plat_gpu_gen.c
    platform/pe_plat_cd_gen6.c
    platform/pe_plat_cd_gen7.c
    platform/pe_plat_card_gen8.c
    platform/pe_plat_gpu_gen9.c
    platform/pe_plat_ds_gen10.c
    platform/pe_plat_ds_gen11.c
    platform/pe_plat_gpu_gen12.c
    platform/pe_plat_card_gen13.c
    platform/pe_plat_card_gen14.c
    platform/pe_plat_gpu_gen15.c
    platform/pe_plat_gpu_tim.c
    platform/pe_plat_card_gen15.c
    platform/pe_plat_ds_gen16.c
    platform/pe_plat_card_gen17.c
    platform/pe_plat_pad_gen18.c
    platform/pe_plat_api_gen19.c
    platform/pe_plat_card_gen20.c
    platform/pe_plat_card_gen21.c
    platform/pe_plat_card_gen22.c
    platform/pe_plat_pad_gen23.c
    platform/pe_plat_pad_gen24.c
    platform/pe_plat_card_gen25.c
    platform/pe_plat_pad_gen26.c
    platform/pe_plat_pad_gen34.c
    platform/pe_plat_ds_gen28.c
    platform/pe_plat_cd_gen29.c
    platform/pe_plat_cd_gen33.c
    platform/pe_plat_ds_gen38.c
    platform/pe_plat_ds_gen39.c
    platform/pe_plat_ds_gen30.c
    platform/pe_plat_ds_gen31.c
    platform/pe_plat_cd_gen32.c
    platform/pe_plat_mdec_reset.c
    platform/pe_plat_mdec_core.c
    platform/pe_plat_cd_ready.c
    platform/pe_plat_81110.c
    platform/pe_plat_sio_tick.c
    platform/pe_plat_gte_cmd_host.c
    platform/pe_plat_gpu_words.c
    platform/pe_plat_gpu_drawenv.c
    platform/pe_plat2_gpuenv.c
    platform/pe_plat_gpu_primitive.c
    platform/pe_plat_gpu_address.c
    platform/pe_plat_state.c
    platform/host_framebuffer.c
    platform/pe_dispenv.c
    platform/pe_timer1.c
    platform/pe_rcnt2.c
    platform/pe_sio0.c
    platform/pe_bios_string.c
    platform/pe_bios_rand.c
    platform/pe_plat_bios_event.c
    platform/pe_plat_gpu_status.c
    platform/pe_plat_gpu_area.c
    platform/pe_plat_gpu_timeout.c
    platform/pe_plat_gpu_dma_queue.c
    platform/pe_plat_gpu_drawsync.c
    platform/pe_plat_gpu_resetgraph.c
    platform/pe_plat_gpu_dma_start.c
    platform/pe_plat_gpu_putdispenv.c
    platform/pe_plat_gpu_host.c
    platform/pe_plat_gpu_swap.c
    platform/pe_plat_gpu_swap_host.c
    platform/pe_plat_irq_callback.c
    platform/pe_plat_pad.c
    platform/pe_plat_pad_info.c
    platform/pe_plat_pad_set.c
    platform/pe_plat_pad_gen35.c
    platform/pe_plat_pad_leaf.c
    platform/pe_plat_pad_state.c
    platform/pe_plat_pad_actinfo.c
    platform/pe_mmio.c
    platform/pe_guestcode.c
    platform/pe_vsync.c
    platform/host_vram.c
    platform/pe_guest_ram.c
    platform/pe_gpu.c
    platform/pe_irq.c
    platform/pe_irq_delivery.c
    platform/pe_gte.c
    platform/pe_gte_commands.c
    platform/pe_mdec.c
    platform/pe_libpress.c
    platform/pe_str_feed.c
    platform/pe_libetc.c
    platform/pe_libgpu.c
    platform/pe_plat_gpu_clear_worker_host.c
    platform/pe_plat_gpu_upload_worker_host.c
    platform/pe_plat_gpu_readback_worker_host.c
    platform/pe_plat_gpu_queue_pump_host.c
    platform/pe_plat_resetgraph_light_host.c
    platform/pe_libsnd.c
    platform/pe_spu_dma.c
    platform/pe_spu.c
    platform/pe_audio_driver.c
    platform/pe_libcard.c
    platform/pe_memcard.c
    platform/pe_bios_card.c
    platform/pe_plat_bios_card.c
    platform/pe_cdreg.c
    platform/pe_libcd.c
    platform/pe_plat_cd.c
    platform/pe_plat_cd_sync.c
    platform/pe_plat_libc.c
    platform/pe_plat_hostfb_forward.c
    platform/pe_drive_tray.c
    platform/pe_disc.c
    platform/pe_disc_check.c
    platform/pe_disc_cache.c
    platform/pe_save.c
    platform/pe_stream.c
    platform/pe_stream_commands.c
    platform/func_80071A24_port.c
    platform/func_800719E4_port.c
    platform/pe_guest_image.c
    platform/plat_mods.c
    platform/plat_cheats.c
    platform/host_pad_combo.c
    platform/plat_enhance.c
    platform/plat_timing.c
    platform/plat_audio.c
)

# Dev-only runtime source: absent from the published tree (see
# cmake/pe_public_inputs.cmake); the list keeps its order when it is present.
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/platform/plat_enhance.c")
    message(STATUS "pe: platform/plat_enhance.c absent: building without the host enhancement settings")
    list(REMOVE_ITEM PLATFORM_SRCS platform/plat_enhance.c)
endif()

set(BOOTSTRAP_SRCS
    bootstrap/game_port.c
    bootstrap/stub_registry.c
    bootstrap/pe_bootstrap.c
    bootstrap/func_8006E9A0_port.c
    bootstrap/func_8006E834_port.c
    bootstrap/func_8006ECEC_port.c
    bootstrap/func_800698D4_port.c
    bootstrap/func_8001220C_port.c
)

set(GAME_SRCS
    game/boot/func_8003E610_port.c
    game/boot/func_8003E680_port.c
    game/boot/func_80071A84_port.c
    game/boot/func_80041108_port.c
    game/boot/func_800405A4_port.c
    game/boot/func_80015AF0_port.c
    game/boot/func_80019DB8_port.c
    game/boot/func_8003E91C_port.c
    game/boot/func_8003E754_port.c
    game/boot/func_8003E944_port.c
    game/boot/func_8003E974_port.c
    game/boot/func_8003EAC8_port.c
    game/boot/func_800371A4_port.c
    game/boot/func_80029388_port.c
    game/boot/func_8005BCA8_port.c
    game/boot/func_80068D28_port.c
    game/boot/func_80068E24_port.c
    game/boot/func_80066B60_port.c
    game/boot/func_8006F39C_port.c
    game/boot/func_8006F6D4_port.c
    game/boot/func_800D413C_port.c
    game/boot/func_800D751C_port.c
    game/boot/func_800C2414_port.c
    game/boot/func_800C3B04_port.c
    game/boot/func_800C9C20_port.c
    game/boot/func_800C5538_port.c
    game/boot/func_800C5A40_port.c
    game/boot/func_80069B08_port.c
    game/boot/func_800D0E88_port.c
    game/boot/func_800D1384_port.c
    game/boot/func_801EDC44_liberate_native.c
    game/boot/func_801EEBC8_liberate_native.c
    game/boot/func_801F0068_effect_native.c
    game/boot/func_800D2104_port.c
    game/boot/func_800D3114_port.c
    game/boot/func_800D2B58_port.c
    game/boot/func_800CF844_port.c
    game/boot/func_800D3BC8_port.c
    game/boot/func_800CEE20_port.c
    game/boot/func_8018F330_port.c
    game/boot/m0013i_effect_port.c
    game/boot/m0023i_effect_port.c
    game/boot/mirror_effect_port.c
    game/boot/m34_boss_effect_port.c
    game/boot/m28_movement_port.c
    game/boot/field_item_pickup_port.c
    game/boot/actor_contact_port.c
    game/boot/func_800CEB8C_port.c
    game/boot/func_800C71E4_port.c
    game/boot/func_80062D2C_port.c
    game/boot/func_8004DD64_port.c
    game/boot/func_8004DF74_port.c
    game/boot/func_8005C498_port.c
    game/boot/func_800438EC_port.c
    game/boot/func_80043B0C_port.c
    game/boot/func_800512AC_port.c
    game/boot/func_8005DCEC_port.c
    game/boot/func_80055760_port.c
    game/boot/func_80044924_port.c
    game/boot/func_8004F910_port.c
    game/boot/func_800452C0_port.c
    game/boot/func_8005112C_port.c
    game/boot/func_80026824_port.c
    game/boot/func_80044B0C_port.c
    game/boot/func_80057834_port.c
    game/boot/func_8005E12C_port.c
    game/boot/func_8005EED4_port.c
    game/boot/func_80061C34_port.c
    game/boot/func_800638D8_port.c
    game/boot/func_80063E0C_port.c
    game/boot/func_8002FAF8_port.c
    game/boot/func_80014228_port.c
    game/boot/func_80069594_port.c
    game/boot/func_80037548_port.c
    game/boot/func_800375E0_port.c
    game/boot/func_80037870_port.c
    game/boot/func_8003EB04_port.c
    game/boot/func_8003F3C4_port.c
    game/boot/func_800124F8_port.c
    game/boot/func_800125E0_port.c
    game/boot/func_80035558_port.c
    game/boot/func_80017018_port.c
    game/boot/func_800181CC_port.c
    game/boot/func_80015DAC_port.c
    game/boot/func_80191854_port.c
    game/boot/func_8018F55C_port.c
    game/boot/func_80195994_port.c
    game/boot/func_80196498_port.c
    game/boot/func_80192030_port.c
    game/boot/func_8018F344_port.c
    game/boot/func_8018F05C_port.c
    game/boot/func_80190D3C_port.c
    game/boot/func_80192800_port.c
    game/boot/transition_ordering_table_port.c
    game/boot/pe_controller_sdk_port.c
    game/boot/func_80197BA0_port.c
    game/boot/func_8019A318_port.c
    game/boot/func_801995BC_port.c
    game/boot/func_8019B1D0_port.c
    game/boot/func_80079384_port.c
    game/boot/func_8018F92C_port.c
    game/boot/func_80193478_port.c
    game/boot/func_801942FC_port.c
    game/boot/func_801904B0_port.c
    game/boot/movie_overlay_port.c
    game/boot/cd_stream_port.c
    game/boot/func_80191DE8_port.c
    game/boot/func_80190998_port.c
    game/boot/func_80012850_port.c
    game/boot/func_80016910_port.c
    game/boot/func_8001A374_port.c
    game/boot/func_8001735C_port.c
    game/boot/func_80015240_port.c
    game/boot/func_80014BA0_port.c
    game/boot/func_8001A15C_port.c
    game/boot/func_800130B4_port.c
    game/boot/func_800131E8_port.c
    game/boot/func_80013C34_port.c
    game/boot/func_800143B0_port.c
    game/boot/func_8006B4F8_port.c
    game/boot/func_800371B0_port.c
    game/boot/func_8001A890_port.c
    game/boot/func_8001AE40_port.c
    game/boot/func_80034F10_port.c
    game/boot/func_8006536C_port.c
    game/boot/func_800653B8_port.c
    game/boot/func_80065954_port.c
    game/boot/func_800659F8_port.c
    game/boot/func_80013988_port.c
    game/boot/func_80013E84_port.c
    game/boot/func_800659C8_port.c
    game/boot/func_80066800_port.c
    game/boot/func_80065AD4_port.c
    game/boot/func_800671C8_port.c
    game/boot/func_80017764_port.c
    game/boot/func_80065400_port.c
    game/boot/func_80012700_port.c
    game/boot/func_80017BB4_port.c
    game/boot/func_800177AC_port.c
    game/boot/func_80019154_port.c
    game/boot/func_8002F7D8_port.c
    game/boot/func_8002FA10_port.c
    game/boot/func_8002FAA4_port.c
    game/boot/func_8002FF78_port.c
    game/boot/func_8002FE78_port.c
    game/boot/func_8003010C_port.c
    game/boot/func_80030220_port.c
    game/boot/battle_reward_port.c
    game/boot/func_800299CC_port.c
    game/boot/func_80033A40_port.c
    game/boot/func_8001D340_port.c
    game/boot/func_8001F9C4_port.c
    game/boot/func_80023E14_port.c
    game/boot/func_8006DE80_port.c
    game/boot/func_80053E6C_port.c
    game/boot/func_8002AA98_port.c
    game/boot/func_80027D14_port.c
    game/boot/func_8002DC58_port.c
    game/boot/func_8002CF24_port.c
    game/boot/func_800293F4_port.c
    game/boot/func_8006CDA4_port.c
    game/boot/func_8006D2B8_port.c
    game/boot/func_80029810_port.c
    game/boot/func_8006C4C4_port.c
    game/boot/func_8003D050_port.c
    game/boot/func_8003C638_port.c
    game/boot/func_8003C5D8_port.c
    game/boot/func_8006698C_port.c
    game/boot/func_8003DFD8_port.c
    game/boot/func_800794C4_port.c
    game/boot/func_8003A088_port.c
    game/boot/func_8003BCE0_port.c
    game/boot/func_80038D1C_port.c
    game/boot/func_8006E6A8_port.c
    game/boot/func_8006E7E8_port.c
    game/boot/func_8006F044_port.c
    game/boot/func_8006E498_port.c
    game/boot/func_8006A9E4_port.c
    game/boot/func_8005B890_port.c
    game/boot/func_8005BC98_port.c
    game/boot/func_8004F808_port.c
    game/boot/func_80042B38_port.c
    game/boot/func_80051084_port.c
    game/boot/func_8005332C_port.c
    game/boot/func_80053968_port.c
    game/boot/func_80053B48_port.c
    game/boot/func_80051CC4_port.c
    game/boot/func_80051980_port.c
    game/boot/field_message_port.c
    game/boot/func_8002156C_port.c
    game/boot/func_80020F18_port.c
    game/boot/func_80026FF8_port.c
    game/boot/func_80025EE8_port.c
    game/boot/func_80021F38_port.c
    game/boot/func_80022210_port.c
    game/boot/func_800D4C24_port.c
    game/boot/func_80022394_port.c
    game/boot/func_80024250_port.c
    game/boot/func_80046574_port.c
    game/boot/func_8004542C_port.c
    game/boot/func_80046DFC_port.c
    game/boot/func_8005B500_port.c
    game/boot/func_80046ABC_port.c
    game/boot/func_8004AD9C_port.c
    game/boot/func_8004FF30_port.c
    game/boot/func_8004AE1C_port.c
    game/boot/func_8004AF3C_port.c
    game/boot/func_8004AFA4_port.c
    game/boot/func_8004B03C_port.c
    game/boot/func_8004B0A4_port.c
    game/boot/func_8004B13C_port.c
    game/boot/func_8004B584_port.c
    game/boot/func_8004FF58_port.c
    game/boot/func_8004FF80_port.c
    game/boot/func_8004B534_port.c
    game/boot/func_8004B55C_port.c
    game/boot/func_80050C70_port.c
    game/boot/func_80050CB4_port.c
    game/boot/func_8004B214_port.c
    game/boot/func_8004B394_port.c
    game/boot/func_8004B650_port.c
    game/boot/func_80050438_port.c
    game/boot/func_8005D994_port.c
    game/boot/func_8005247C_port.c
    game/boot/func_8004B5DC_port.c
    game/boot/func_8005FCAC_port.c
    game/boot/func_8005ED18_port.c
    game/boot/func_800515F8_port.c
    game/boot/func_800236E8_port.c
    game/boot/func_80028574_port.c
    game/boot/func_800314E4_port.c
    game/boot/func_8007041C_port.c
    game/boot/func_800C22F8_port.c
    game/boot/func_800CE084_port.c
    game/boot/func_8005218C_port.c
    game/boot/func_8005B91C_port.c
    game/boot/func_800527C8_port.c
    game/boot/func_800528F0_port.c
    game/boot/func_8005E588_port.c
    game/boot/func_8005E57C_port.c
    game/boot/func_8005C1EC_port.c
    game/boot/func_80042538_port.c
    game/boot/func_80062568_port.c
    game/boot/func_80064964_port.c
    game/boot/func_8005DE88_port.c
    game/boot/func_80052C6C_port.c
    game/boot/func_80053D2C_port.c
    game/boot/func_80042C78_port.c
    game/boot/func_80042CC4_port.c
    game/boot/func_8006A2E8_port.c
    game/boot/func_80052790_port.c
    game/boot/func_800649D0_port.c
    game/boot/func_8005E850_port.c
    game/boot/func_8005E884_port.c
    game/boot/func_800614AC_port.c
    game/boot/func_8005BCBC_port.c
    game/boot/func_8005D6F4_port.c
    game/boot/func_8005DC4C_port.c
    game/boot/func_80052594_port.c
    game/boot/func_8005DB8C_port.c
    game/boot/func_8005DBAC_port.c
    game/boot/func_800438C0_port.c
    game/boot/func_8005CCA4_port.c
    game/boot/func_8005E968_port.c
    game/boot/func_8005F844_port.c
    game/boot/func_80070D10_port.c
    game/boot/func_80070D6C_port.c
    game/boot/func_80070DD0_port.c
    game/boot/func_8006A5BC_port.c
    game/boot/func_8006A64C_port.c
    game/boot/func_8006A674_port.c
    game/boot/func_8006A8D4_port.c
    game/boot/func_80086728_port.c
    game/boot/func_80087090_port.c
    game/boot/func_800851A8_port.c
    game/boot/func_80085EB4_port.c
    game/boot/func_800850F4_port.c
    game/boot/func_8006AD40_port.c
    game/boot/func_8006E1C0_port.c
    game/boot/func_8007506C_port.c
    game/boot/func_8007512C_port.c
    game/boot/func_80042798_port.c
    game/boot/func_800752AC_port.c
    game/boot/func_80075358_port.c
    game/boot/func_800753B4_port.c
    game/boot/func_80070E54_port.c
    game/boot/func_80076664_port.c
    game/boot/func_800768A0_port.c
    game/boot/func_800746A0_port.c
    game/boot/func_80076C34_port.c
    game/boot/func_80076EE4_port.c
    game/boot/func_80077B64_port.c
    game/boot/func_80077BA4_port.c
    game/boot/func_80077BC4_port.c
    game/boot/func_80077BE4_port.c
    game/boot/func_80077C44_port.c
    game/boot/func_80077C64_port.c
    game/boot/func_80077A64_port.c
    game/boot/func_80077AA4_port.c
    game/boot/func_80077B04_port.c
    game/boot/func_80077B34_port.c
    game/boot/func_80077C04_port.c
    game/boot/func_80077CB4_port.c
    game/boot/func_8007A214_port.c
    game/boot/func_8005DADC_port.c
    game/boot/func_800370DC_port.c
    game/boot/func_80037140_port.c
    game/boot/func_80030894_port.c
    game/boot/func_8010BE3C_port.c
    game/boot/func_80190660_port.c
    game/boot/func_8018F2F4_port.c
    game/boot/func_8018F468_port.c
    game/boot/title_menu_callbacks_port.c
    game/boot/func_8018FBC0_port.c
    game/boot/func_80190064_port.c
    game/boot/func_801909B4_port.c
    game/boot/func_80192FE8_port.c
    game/boot/func_8019319C_port.c
    game/boot/func_8019234C_port.c
    game/boot/func_801918F8_port.c
    game/boot/func_80191FB8_port.c
    game/boot/func_801924F8_port.c
    game/boot/func_80191DC8_port.c
    game/boot/func_80192934_port.c
    game/boot/func_80081314_port.c
    game/boot/func_80192CE8_port.c
    game/boot/func_800870F0_port.c
    game/boot/func_80191B64_port.c
    game/boot/func_8010BD4C_port.c
)

# Decomp-derived port TUs (pc_port/game/decomp/*.c).  They are generated from
# the verified matching C leaves in src/ by tools/analysis/gen_decomp_ports.py;
# the glob keeps the manifest honest as matching advances.  A generated TU that
# would collide with an existing pc_port definition is never written (see the
# generator's conflict report), so every name here has exactly one provider.
file(GLOB DECOMP_SRCS CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/game/decomp/*_port.c
)
# These menu callbacks must compute guest addresses with 32-bit wrap.
list(REMOVE_ITEM DECOMP_SRCS
    "${CMAKE_SOURCE_DIR}/game/decomp/func_800428D4_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80050F10_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80050AD8_port.c")
# Field-menu step unwinds on stopped callbacks; keep generated source intact.
list(REMOVE_ITEM DECOMP_SRCS
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8005C498_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8005C594_port.c")
# Game item-use adapters unwind on stopped recovery callbacks.
list(REMOVE_ITEM DECOMP_SRCS
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80057834_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80022210_port.c")
# Game effect-cleanup adapters retain stop propagation across callbacks.
list(REMOVE_ITEM DECOMP_SRCS
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8006FE14_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80070064_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_800702DC_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_800703F4_port.c")
# 50BE8's matching leaf has an implicit live a0. The native reward callback
# supplies that argument explicitly; keep the generated matching source intact.
list(REMOVE_ITEM DECOMP_SRCS "${CMAKE_SOURCE_DIR}/game/decomp/func_80050BE8_port.c")
# These independently provided entries may have stale generated outputs.
list(REMOVE_ITEM DECOMP_SRCS
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8007FA2C_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8007A4D0_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8007A60C_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8007A740_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8007A3CC_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8007A8EC_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80080404_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8008080C_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8007C544_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8007CE80_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8007D15C_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8007DCAC_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8007D054_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8007D074_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8007D1D4_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8007D454_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80085984_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80085F74_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_800862F4_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80085C44_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80085A64_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80085EB4_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80085B34_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80085BB4_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80085A04_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80089EB8_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8007DD14_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80089F28_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80085D84_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80085DC4_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80085F14_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80085E54_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80085DF4_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8007BDDC_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8007A910_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80080B24_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80089F08_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8007DC04_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8007DAE0_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80075C04_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8007FBC0_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8007FBCC_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8007FBD8_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8007FBE4_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8007C130_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80080930_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_800812F4_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8007DB24_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8007DC84_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8007DBC8_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8007DC5C_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80076B58_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80076BE0_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8007633C_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80076B20_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80076B44_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80073DF8_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80085F44_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80085814_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_800858E8_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80085918_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_800858B0_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8008594C_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8007F7A8_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8007C444_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80078C94_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8007A400_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8007A434_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80071944_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80071964_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80071994_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80080950_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80080998_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80081254_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_800824B4_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_800824C8_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_800824DC_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8007A4A8_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8007A4BC_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8007A488_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8007FC64_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8007FC88_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8007A88C_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8007A8AC_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8007A8CC_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80080AC4_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80080AE4_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80080B04_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_800816F4_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_800749D8_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80077CB4_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80077A28_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80074330_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_800744A4_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8007474C_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80074478_port.c"
    # Dead SDK leaves (gen_decomp_ports.py DEAD_SDK_UNLINKED): not linked.
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8007DEC0_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8007DF50_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8007DFE0_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8007E010_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8007E048_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8007E0C0_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8007E0F8_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8007E160_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80074CC8_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8007DE78_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_800829C4_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80075C44_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80073BBC_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80075C6C_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8007D6D0_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8007EE64_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80080D34_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80082514_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80082554_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_800835A4_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80082DBC_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80082574_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_800835B0_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80077B04_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80077B34_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80077B84_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80077BE4_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80077C24_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80077C84_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80076150_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_800762A0_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80077B64_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80077BA4_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80077BC4_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80077C04_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80077C44_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80077C64_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80077A64_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80077AA4_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80074CB8_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8007DEA4_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8007FC34_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8007FC44_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8007FC08_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8007F7C8_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8007FC18_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80074A14_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80081E5C_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80073DE8_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80074A28_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8007DEB0_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8007F788_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_800822AC_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8007F778_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8007FC28_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8007FC54_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8007FCAC_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80080940_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80080B44_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80080C48_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80080CC8_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_8007A3EC_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_800724F4_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80072734_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80072744_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80072754_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80072764_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80072774_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80072784_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80072794_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_800727A4_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80072A54_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80071A04_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80071A14_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80072314_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80071A34_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80071A44_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80071A24_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80036DC8_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_80072334_port.c"
    "${CMAKE_SOURCE_DIR}/game/decomp/func_800725DC_port.c")
# Independent CD providers replace these generated SDK TUs, including stale outputs.
list(FILTER DECOMP_SRCS EXCLUDE REGEX "/game/decomp/func_8007A(930|A34|324|334|344|354)_port\\.c$")
# Hand adapters of matched leaves the generator cannot derive mechanically
# (pc_port/game/decomp_hand/*_port.c): written from src/func_X.c, each with
# a pc_port test.  Globbed so parallel porting lanes never edit this list.
file(GLOB DECOMP_HAND_SRCS CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/game/decomp_hand/*_port.c
)
list(SORT DECOMP_HAND_SRCS)
list(APPEND DECOMP_SRCS ${DECOMP_HAND_SRCS})
# Overlay-derived TUs (pc_port/game/decomp_ovl/<ovl>/*_port.c), generated by
# tools/analysis/gen_overlay_ports.py; each defines the namespaced symbol
# `<ovl>__func_X`, so overlays sharing an arena link side by side.
file(GLOB DECOMP_OVL_SRCS CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/game/decomp_ovl/*/*_port.c
)
list(SORT DECOMP_OVL_SRCS)
list(APPEND DECOMP_SRCS ${DECOMP_OVL_SRCS})
list(SORT DECOMP_SRCS)
# Plain `char` is UNSIGNED in the retail era toolchain (gcc-2.7.2-psx: a
# matched leaf gets `lbu` for `char` and needs `signed char` for `lb`).  The
# decomp-derived TUs copy that C verbatim, so they must be compiled with the
# same char signedness or e.g. src/func_80052BCC.c's `c &= 0xFF; if (c == t)`
# never sees the 0xFF terminator on an x86 host (portverify, retail oracle
# INV8 case 80).
# The era compiler also had no type-based aliasing (-fstrict-aliasing came in
# GCC 2.95), and retail's addu/subu wrap.  Verbatim decomp C reads guest RAM
# through typed host pointers (`*(short *)(p + 8)` beside `*(int *)p`) and
# relies on wrapping arithmetic, so at -O3 GCC must not assume either away:
# 88 decomp TUs compile differently with these two options, 16 because of
# -fno-strict-aliasing and 73 because of -fwrapv (docs/port/NATIVE_COMPLETION.md,
# "Host compile semantics").
set_source_files_properties(${DECOMP_SRCS} PROPERTIES COMPILE_OPTIONS
    "$<IF:$<C_COMPILER_ID:MSVC>,/J,-funsigned-char>;$<$<NOT:$<C_COMPILER_ID:MSVC>>:-fno-strict-aliasing>;$<$<NOT:$<C_COMPILER_ID:MSVC>>:-fwrapv>")
list(LENGTH DECOMP_SRCS PE_DECOMP_TU_COUNT)
message(STATUS "pe: ${PE_DECOMP_TU_COUNT} decomp-derived port TUs")

set(FIELD_RUNTIME_SRCS
    src/pe_globals.c
    platform/pe_callback.c
    platform/pe_guest_decomp.c
    ${PLATFORM_SRCS}
    ${BOOTSTRAP_SRCS}
    ${GAME_SRCS}
    ${DECOMP_SRCS}
)

# The static runtime is the reusable product boundary.  It contains the
# translated retail graph and its host-safe providers, but not the CLI entry
# point or the X11 presentation adapter.  Creating this target does not claim
# that the currently translated graph is a complete Day 1 field runtime.
add_library(pe_field_runtime STATIC ${FIELD_RUNTIME_SRCS})
target_include_directories(pe_field_runtime PUBLIC
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/platform
    ${CMAKE_SOURCE_DIR}/bootstrap
    ${CMAKE_SOURCE_DIR}/src
)
target_compile_definitions(pe_field_runtime PUBLIC
    PE_PORT_HEADLESS=1
    PE_PORT_FB_WIDTH=320
    PE_PORT_FB_HEIGHT=240
)

# The command-line host is one consumer of the static runtime.  Its window
# backend remains outside the library so non-X11 consumers do not inherit it.
# Window backend is per-OS: X11/dlopen on POSIX, Win32 GDI on Windows.
# Both implement the same platform/host_window.h interface.
#
# host_pad_remap.c (controls.conf: pad.* / key.* remapping) and host_key_merge.c
# (several keys held on one button) are pure C used by the backends' keyboard
# handling, so they live with the window backend and not in the library.
if(WIN32)
    set(HOST_WINDOW_SRC platform/host_window_win32.c platform/host_pad_remap.c platform/host_key_merge.c)
else()
    set(HOST_WINDOW_SRC platform/host_window.c platform/host_pad.c platform/host_pad_sdl.c platform/host_dev_menu.c platform/host_pad_remap.c platform/host_key_merge.c)
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/platform/host_dev_menu.c")
        message(STATUS "pe: platform/host_dev_menu.c absent: building without the dev menu")
        list(REMOVE_ITEM HOST_WINDOW_SRC platform/host_dev_menu.c)
    endif()
endif()

# Guest-code dispatch registry (generated by gen_overlay_ports.py): linked
# only into the full runtime executables, see pc_port/platform/pe_guestcode.h.
set(PE_GUESTCODE_REGISTRY ${CMAKE_SOURCE_DIR}/game/decomp_ovl/_dispatch/zz_dispatch.c)
add_executable(parasite-eve-port
    src/port_main.c
    ${HOST_WINDOW_SRC}
    platform/host_audio.c
    ${PE_GUESTCODE_REGISTRY}
)
target_link_libraries(parasite-eve-port PRIVATE pe_field_runtime)
# Optional headers only: SDL2 is dlopened when PE_DEV_MENU=1, preserving
# the default host's dependency footprint. Never replace SDL with js API.
if(UNIX AND NOT APPLE)
    find_package(SDL2 2.0.14 QUIET)
    if(SDL2_FOUND)
        target_include_directories(parasite-eve-port PRIVATE ${SDL2_INCLUDE_DIRS})
        target_compile_definitions(parasite-eve-port PRIVATE PE_HAVE_SDL2=1)
    else()
        message(STATUS "SDL2 headers unavailable: opt-in dev controller reports unsupported")
    endif()
endif()

target_include_directories(parasite-eve-port PRIVATE ${CMAKE_SOURCE_DIR}/tests)
if(NOT MSVC)
    # Display filters use the C math library (MSVC supplies it in the CRT).
    target_link_libraries(parasite-eve-port PRIVATE m)
endif()
if(NOT WIN32)
    # POSIX X11 backend resolves dlopen/dlsym from libdl.
    target_link_libraries(parasite-eve-port PRIVATE dl)
    # host_audio.c writer thread (glibc provides it; no new external library).
    find_package(Threads REQUIRED)
    target_link_libraries(parasite-eve-port PRIVATE Threads::Threads)
endif()

target_compile_definitions(parasite-eve-port PRIVATE
    PE_PORT_HEADLESS=1
    PE_PORT_FB_WIDTH=320
    PE_PORT_FB_HEIGHT=240
)

# GCC/Clang-style sanitizer flags; MSVC uses /fsanitize=address instead,
# so these only apply to non-MSVC toolchains.
if(PE_PORT_SANITIZERS AND NOT MSVC)
    target_compile_options(pe_field_runtime PRIVATE
        -fsanitize=address,undefined
        -fno-omit-frame-pointer
    )
    target_compile_options(parasite-eve-port PRIVATE
        -fsanitize=address,undefined
        -fno-omit-frame-pointer
    )
    # Static runtimes: the system shared libasan can mismatch the compiler
    target_link_options(parasite-eve-port PRIVATE
        -fsanitize=address,undefined
        -static-libasan -static-libubsan
    )
endif()
include(cmake/sdl.cmake)  # PE_HOST_BACKEND: x11 (default) | sdl2; APPLE always sdl2

# Tests
# Some test inputs (retail-derived vectors, dev-only tests and headers) are not
# in every tree.  pe_test_inputs_present() gates each affected target on its
# #include closure, so a tree without them skips exactly those targets, with
# one message (pe_report_skipped_tests() below); a full tree builds them all.
include(cmake/pe_public_inputs.cmake)
option(BUILD_TESTS "Build native port tests" ON)
if(BUILD_TESTS)
    if(WIN32)
        add_executable(pe-win32-input-tests tests/test_win32_input.c platform/host_window_win32.c platform/host_pad_remap.c platform/host_key_merge.c)
        target_include_directories(pe-win32-input-tests PRIVATE platform)
        target_link_libraries(pe-win32-input-tests PRIVATE user32 gdi32)
        add_test(NAME win32-input COMMAND pe-win32-input-tests)
        # A user's controls.conf must not change the default-key assertions.
        set_tests_properties(win32-input PROPERTIES TIMEOUT 30
                             ENVIRONMENT "PE_CONTROLS_CONFIG=pe-no-such-controls.conf")
    endif()
    enable_testing()
    if(UNIX AND NOT APPLE)
        # The dev menu reads the merged controller word (no SDL of its own):
        # the test feeds HostDevMenu_Step synthetic words and clock values.
        pe_test_inputs_present(_pe_ok pe-dev-menu-controller-tests tests/test_dev_menu_controller.c platform/host_dev_menu.c)
        if(_pe_ok)
            add_executable(pe-dev-menu-controller-tests tests/test_dev_menu_controller.c platform/host_dev_menu.c)
            target_include_directories(pe-dev-menu-controller-tests PRIVATE platform include)
            target_link_libraries(pe-dev-menu-controller-tests PRIVATE pe_field_runtime)
            if(PE_PORT_SANITIZERS)
                target_compile_options(pe-dev-menu-controller-tests PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
                target_link_options(pe-dev-menu-controller-tests PRIVATE -fsanitize=address,undefined -static-libasan -static-libubsan)
            endif()
            add_test(NAME dev-menu-controller COMMAND pe-dev-menu-controller-tests)
            add_test(NAME dev-menu-hard-disable COMMAND pe-dev-menu-controller-tests --disabled)
            set_tests_properties(dev-menu-controller PROPERTIES TIMEOUT 30)
        endif()
    endif()


    # Letterbox/pillarbox scale math is platform-independent (shared by the
    # X11 and Win32 backends, and by --fullscreen); no X11/Win32/runtime
    # dependency needed, so it always builds regardless of WIN32.
    # MDEC host model: status word and register-path == block-API bookkeeping (stubbed neighbours).
    add_executable(pe-mdec-status-word-tests tests/test_mdec_status_word.c platform/pe_mdec.c)
    add_executable(pe-mdec-register-path-tests tests/test_mdec_register_path.c platform/pe_mdec.c)
    target_include_directories(pe-mdec-status-word-tests PRIVATE platform include bootstrap)
    target_include_directories(pe-mdec-register-path-tests PRIVATE platform include bootstrap)
    add_test(NAME mdec-status-word COMMAND pe-mdec-status-word-tests)
    add_test(NAME mdec-register-path COMMAND pe-mdec-register-path-tests)
    set_tests_properties(mdec-status-word mdec-register-path PROPERTIES TIMEOUT 30)
    add_executable(pe-window-scale-tests tests/test_host_window_scale.c)
    add_test(NAME window-scale-math COMMAND pe-window-scale-tests)
    set_tests_properties(window-scale-math PROPERTIES TIMEOUT 30)
    # DISPLAY SCALER filters (sharp bilinear, FSR 1): identity of the retail
    # NEAREST mode and the filtered modes' invariants; also the before/after
    # capture tool (`pe-upscale-tests --demo in.ppm prefix W H`).
    add_executable(pe-upscale-tests tests/test_host_upscale.c)
    add_executable(pe-upscale-no-threads-tests tests/test_host_upscale.c)
    target_compile_definitions(pe-upscale-no-threads-tests PRIVATE PE_UPSCALE_NO_THREADS=1)
    if(NOT MSVC)
        target_link_libraries(pe-upscale-tests PRIVATE m)
        target_link_libraries(pe-upscale-no-threads-tests PRIVATE m)
    endif()
    if(NOT WIN32)
        find_package(Threads REQUIRED)
        target_link_libraries(pe-upscale-tests PRIVATE Threads::Threads)
    endif()
    add_test(NAME upscale-filters COMMAND pe-upscale-tests)
    add_test(NAME upscale-filters-single-thread COMMAND pe-upscale-tests)
    add_test(NAME upscale-no-threads COMMAND pe-upscale-no-threads-tests)
    set_tests_properties(upscale-filters-single-thread PROPERTIES ENVIRONMENT "PE_SCALER_THREADS=1")
    set_tests_properties(upscale-filters upscale-filters-single-thread PROPERTIES TIMEOUT 120)
    if(UNIX AND NOT APPLE)
        pe_test_inputs_present(_pe_ok pe-window-scaler-failure-tests tests/test_window_scaler_failures.c platform/host_key_merge.c)
        if(_pe_ok)
            add_executable(pe-window-scaler-failure-tests tests/test_window_scaler_failures.c platform/host_key_merge.c)
            target_link_libraries(pe-window-scaler-failure-tests PRIVATE m Threads::Threads dl)
            add_test(NAME window-scaler-failures COMMAND pe-window-scaler-failure-tests)
        endif()
        pe_test_inputs_present(_pe_ok pe-scaler-windows-caps-tests tests/test_scaler_windows_caps.c)
        if(_pe_ok)
            add_executable(pe-scaler-windows-caps-tests tests/test_scaler_windows_caps.c)
            add_test(NAME scaler-windows-policy COMMAND pe-scaler-windows-caps-tests)
        endif()
        foreach(scaler_target pe-window-scaler-failure-tests pe-scaler-windows-caps-tests)
            if(TARGET ${scaler_target})
                target_include_directories(${scaler_target} PRIVATE include platform bootstrap)
                target_compile_options(${scaler_target} PRIVATE -ffunction-sections -fdata-sections)
                target_link_options(${scaler_target} PRIVATE -Wl,--gc-sections)
            endif()
        endforeach()
        add_executable(pe-gpu-shadow-tests tests/test_gpu_shadow.c platform/pe_gpu.c)
        add_executable(pe-framebuffer-shadow-tests tests/test_framebuffer_shadow.c
            platform/pe_gpu.c platform/host_framebuffer.c platform/host_vram.c)
        add_executable(pe-gpu-filter-tests tests/test_gpu_filter.c platform/pe_gpu.c)
        add_executable(pe-gpu-dither-tests tests/test_gpu_dither.c platform/pe_gpu.c)
        add_executable(pe-gpu-subpixel-tests tests/test_gpu_subpixel.c platform/pe_gpu.c)
        add_executable(pe-framebuffer-ssaa-tests tests/test_framebuffer_ssaa.c
            platform/pe_gpu.c platform/host_framebuffer.c platform/host_vram.c)
        foreach(shadow_target pe-gpu-shadow-tests pe-framebuffer-shadow-tests pe-gpu-filter-tests pe-gpu-dither-tests pe-gpu-subpixel-tests pe-framebuffer-ssaa-tests)
            target_include_directories(${shadow_target} PRIVATE include platform bootstrap)
            target_compile_options(${shadow_target} PRIVATE -ffunction-sections -fdata-sections)
            target_link_options(${shadow_target} PRIVATE -Wl,--gc-sections)
        endforeach()
        add_test(NAME gpu-shadow-parity COMMAND pe-gpu-shadow-tests)
        add_test(NAME framebuffer-shadow-parity COMMAND pe-framebuffer-shadow-tests)
        add_test(NAME gpu-filter-parity COMMAND pe-gpu-filter-tests)
        add_test(NAME gpu-dither-parity COMMAND pe-gpu-dither-tests)
        add_test(NAME gpu-subpixel-parity COMMAND pe-gpu-subpixel-tests)
        add_test(NAME framebuffer-ssaa-parity COMMAND pe-framebuffer-ssaa-tests)
    endif()

    # tests/pe_akao_interp_oracle.c: TEST-ONLY retail-code oracle (R3000A
    # interpreter).  Never add it to pe_field_runtime or parasite-eve-port:
    # the port binary is native only (no MIPS execution, 2026-10-07).
    pe_test_inputs_present(_pe_ok pe-native-tests tests/test_native.c tests/pe_akao_interp_oracle.c)
    if(_pe_ok)
        add_executable(pe-native-tests tests/test_native.c tests/pe_akao_interp_oracle.c ${PE_GUESTCODE_REGISTRY})
    endif()
    # Differential check of hand adapters vs the matched C compiled for the
    # PSX (pc_port/tools/build_leaf_oracle.py writes build/leaf-oracle/,
    # git-ignored).  Test-only; exit 77 = image missing (skip).
    pe_test_inputs_present(_pe_ok pe-leaf-oracle-tests tests/test_leaf_oracle.c tests/pe_akao_interp_oracle.c)
    if(_pe_ok)
        add_executable(pe-leaf-oracle-tests tests/test_leaf_oracle.c tests/pe_akao_interp_oracle.c ${PE_GUESTCODE_REGISTRY})
        target_link_libraries(pe-leaf-oracle-tests PRIVATE pe_field_runtime)
        add_test(NAME leaf-oracle COMMAND pe-leaf-oracle-tests
                 WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/..)
        set_tests_properties(leaf-oracle PROPERTIES SKIP_RETURN_CODE 77)
    endif()
    # Whole-tick AKAO differential (func_8008DB7C graph); image from
    # build_leaf_oracle.py --out build/leaf-oracle-akao --range 0x80085000
    # 0x80092000 func_8008DB7C.  Test-only; exit 77 = image missing.
    pe_test_inputs_present(_pe_ok pe-akao-tick-oracle-tests tests/test_akao_tick_oracle.c tests/pe_akao_interp_oracle.c)
    if(_pe_ok)
        add_executable(pe-akao-tick-oracle-tests tests/test_akao_tick_oracle.c tests/pe_akao_interp_oracle.c ${PE_GUESTCODE_REGISTRY})
        target_link_libraries(pe-akao-tick-oracle-tests PRIVATE pe_field_runtime)
        add_test(NAME akao-tick-oracle COMMAND pe-akao-tick-oracle-tests
                 WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/..)
        set_tests_properties(akao-tick-oracle PROPERTIES SKIP_RETURN_CODE 77 TIMEOUT 600)
    endif()
    # M0086I park callbacks through the generated registry and actual native
    # callees, compared with retained retail-oracle fixtures. No interpreter
    # is linked; the owner's disc/cache supplies the original room identity.
    pe_test_inputs_present(_pe_ok pe-park-m0086-full-tests tests/test_park_m0086_full.c)
    if(_pe_ok)
        add_executable(pe-park-m0086-full-tests
            tests/test_park_m0086_full.c ${PE_GUESTCODE_REGISTRY})
        target_link_libraries(pe-park-m0086-full-tests PRIVATE pe_field_runtime)
        add_test(NAME park-m0086-full COMMAND pe-park-m0086-full-tests
                 WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/..)
        set_tests_properties(park-m0086-full PROPERTIES SKIP_RETURN_CODE 77 TIMEOUT 120)
    endif()

    # M0089I code-0x77 effect through the generated registry and actual native
    # callees, with retained retail RAM/packet/GTE/RNG fixtures and controls.
    pe_test_inputs_present(_pe_ok pe-park-m0089-full-tests tests/test_park_m0089_full.c)
    if(_pe_ok)
        add_executable(pe-park-m0089-full-tests
            tests/test_park_m0089_full.c ${PE_GUESTCODE_REGISTRY})
        target_link_libraries(pe-park-m0089-full-tests PRIVATE pe_field_runtime)
        add_test(NAME park-m0089-full COMMAND pe-park-m0089-full-tests
                 WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/..)
        set_tests_properties(park-m0089-full PROPERTIES SKIP_RETURN_CODE 77 TIMEOUT 120)
    endif()

    # Standalone GTE math: no game runtime or interpreter in this executable.
    add_executable(pe-gte-native-command-tests tests/test_gte_native_commands.c
        platform/pe_gte.c platform/pe_gte_commands.c)
    target_include_directories(pe-gte-native-command-tests PRIVATE
        ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/platform ${CMAKE_SOURCE_DIR}/bootstrap)
    add_test(NAME gte-native-fixed COMMAND pe-gte-native-command-tests)
    set_tests_properties(gte-native-fixed PROPERTIES TIMEOUT 30)
    set(PE_GTE_HW_LOG "" CACHE FILEPATH "Existing MIT ps1-tests real-console GTE log")
    if(PE_GTE_HW_LOG)
        add_test(NAME gte-native-hardware COMMAND pe-gte-native-command-tests
            --hardware-log ${PE_GTE_HW_LOG})
        set_tests_properties(gte-native-hardware PROPERTIES TIMEOUT 30)
    endif()

    if(TARGET pe-native-tests)
        target_link_libraries(pe-native-tests PRIVATE pe_field_runtime)
    endif()

    # Small external-consumer contract: this source is intentionally not part
    # of the runtime target and reaches both host substrate and translated
    # retail code only through the archive's public include/link interface.
    add_executable(pe-field-runtime-link-test
        tests/test_field_runtime_link.c
    )
    target_link_libraries(pe-field-runtime-link-test PRIVATE pe_field_runtime)

    if(UNIX AND NOT APPLE)
        pe_test_inputs_present(_pe_ok pe-disc-loader-tests tests/test_disc_loader_loop.c)
        if(_pe_ok)
            add_executable(pe-disc-loader-tests tests/test_disc_loader_loop.c)
            target_link_libraries(pe-disc-loader-tests PRIVATE pe_field_runtime)
            foreach(provider 80087198 80087414 8006E6D4 8006E7E8 800871AC 80087090 800875FC 80087428 800870E0)
                target_link_options(pe-disc-loader-tests PRIVATE "-Wl,--wrap=func_${provider}")
            endforeach()
            if(PE_PORT_SANITIZERS)
                target_compile_options(pe-disc-loader-tests PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
                target_link_options(pe-disc-loader-tests PRIVATE -fsanitize=address,undefined -static-libasan -static-libubsan)
            endif()
            add_test(NAME disc-loader-control-flow COMMAND pe-disc-loader-tests)
            set_tests_properties(disc-loader-control-flow PROPERTIES TIMEOUT 30)
        endif()
    endif()

    if(UNIX AND NOT APPLE)
        # Linux joystick mapping + FIFO-backed device hot-plug (no hardware).
        add_executable(pe-host-pad-tests tests/test_host_pad.c platform/host_pad.c platform/host_pad_sdl.c)
        target_include_directories(pe-host-pad-tests PRIVATE platform)
        target_link_libraries(pe-host-pad-tests PRIVATE dl)
        add_test(NAME host-pad-input COMMAND pe-host-pad-tests)
        set_tests_properties(host-pad-input PROPERTIES TIMEOUT 30 ENVIRONMENT "PE_PAD_SDL=off")
        # SDL2 GameController / raw-joystick mapping (pure) + runtime with a
        # stub libSDL2 (tests/stub_sdl2.c) for hot-plug and fallback order.
        add_library(pe-stub-sdl2 SHARED tests/stub_sdl2.c)
        add_executable(pe-host-pad-sdl-tests tests/test_host_pad_sdl.c platform/host_pad.c platform/host_pad_sdl.c)
        target_include_directories(pe-host-pad-sdl-tests PRIVATE platform)
        target_link_libraries(pe-host-pad-sdl-tests PRIVATE dl)
        add_dependencies(pe-host-pad-sdl-tests pe-stub-sdl2)
        add_test(NAME host-pad-sdl COMMAND pe-host-pad-sdl-tests $<TARGET_FILE:pe-stub-sdl2>)
        set_tests_properties(host-pad-sdl PROPERTIES TIMEOUT 30 ENVIRONMENT "PE_JOYSTICK=off")
    # Syscall-mocked hot-plug replay (ENODEV mid-burst, fd reuse, renumber,
    # layout change): the Legion sequence of run-20260929T151702Z.log.
    # The test #includes host_pad.c, so it is not linked separately.
    add_executable(pe-host-pad-hotplug-tests tests/test_host_pad_hotplug.c platform/host_pad_sdl.c)
    target_include_directories(pe-host-pad-hotplug-tests PRIVATE platform)
    target_link_libraries(pe-host-pad-hotplug-tests PRIVATE dl)
    add_test(NAME host-pad-hotplug COMMAND pe-host-pad-hotplug-tests)
    set_tests_properties(host-pad-hotplug PROPERTIES TIMEOUT 30 ENVIRONMENT "PE_PAD_SDL=off")
    # Controls remapping (controls.conf): parser, identity over all 65536 pad
    # words, swap, key table == the old X11 chain, reserved keys, Win32 VK table
    # == the old switch.  Pure; linked with the combo to pin the composition.
    add_executable(pe-host-pad-remap-tests tests/test_host_pad_remap.c platform/host_pad_remap.c platform/host_pad_combo.c)
    target_include_directories(pe-host-pad-remap-tests PRIVATE platform)
    add_test(NAME host-pad-remap COMMAND pe-host-pad-remap-tests)
    set_tests_properties(host-pad-remap PROPERTIES TIMEOUT 30)
    # Several keys on one button (host_key_merge.c): the pure merger and its
    # backend glue.  Pure; no window.
    add_executable(pe-host-key-merge-tests tests/test_host_key_merge.c platform/host_key_merge.c)
    target_include_directories(pe-host-key-merge-tests PRIVATE platform)
    add_test(NAME host-key-merge COMMAND pe-host-key-merge-tests)
    set_tests_properties(host-key-merge PROPERTIES TIMEOUT 30)
    # POSIX card persistence: synthetic images and injected pre-rename I/O
    # failures. This fixture never resolves a user's configured card path.
    add_executable(pe-memcard-sync-tests tests/test_memcard_sync.c)
    if(PE_PORT_SANITIZERS)
        target_compile_options(pe-memcard-sync-tests PRIVATE -fsanitize=address,undefined
                               -fno-sanitize-recover=all -fno-omit-frame-pointer)
        target_link_options(pe-memcard-sync-tests PRIVATE -fsanitize=address,undefined)
    endif()
    add_test(NAME memcard-sync COMMAND pe-memcard-sync-tests ${CMAKE_CURRENT_BINARY_DIR})
    set_tests_properties(memcard-sync PROPERTIES TIMEOUT 30)
    # Screenshot names must be claimed without overwriting a concurrent or
    # unreadable capture. Synthetic files only, in private temporary folders.
    add_executable(pe-host-shot-tests tests/test_host_shot_exclusive.c)
    target_include_directories(pe-host-shot-tests PRIVATE platform)
    if(PE_PORT_SANITIZERS)
        target_compile_options(pe-host-shot-tests PRIVATE -fsanitize=address,undefined
                               -fno-sanitize-recover=all -fno-omit-frame-pointer)
        target_link_options(pe-host-shot-tests PRIVATE -fsanitize=address,undefined)
    endif()
    add_test(NAME host-shot COMMAND pe-host-shot-tests)
    set_tests_properties(host-shot PROPERTIES TIMEOUT 30)
    # Windows audio contracts using a fake Win32/WinMM API on the POSIX host.
    # Each fixture includes host_audio_winmm.c; no game runtime or audio
    # device is linked, and the fake writer thread never runs.
    add_executable(pe-winmm-lifecycle-tests tests/winmm_fixture/test_winmm_lifecycle.c)
    add_executable(pe-winmm-timeout-tests tests/winmm_fixture/test_winmm_timeout_contract.c)
    foreach(_pe_winmm_target pe-winmm-lifecycle-tests pe-winmm-timeout-tests)
        target_include_directories(${_pe_winmm_target} PRIVATE tests/winmm_fixture platform include bootstrap)
        if(PE_PORT_SANITIZERS)
            target_compile_options(${_pe_winmm_target} PRIVATE -fsanitize=address,undefined
                                   -fno-sanitize-recover=all -fno-omit-frame-pointer)
            target_link_options(${_pe_winmm_target} PRIVATE -fsanitize=address,undefined)
        endif()
    endforeach()
    add_test(NAME winmm-lifecycle COMMAND pe-winmm-lifecycle-tests)
    add_test(NAME winmm-wait-timeout COMMAND pe-winmm-timeout-tests 0)
    add_test(NAME winmm-wait-failed COMMAND pe-winmm-timeout-tests 1)
    add_test(NAME winmm-stuck-reopen COMMAND pe-winmm-timeout-tests 2)
    set_tests_properties(winmm-lifecycle winmm-wait-timeout winmm-wait-failed winmm-stuck-reopen
                         PROPERTIES TIMEOUT 30)
    # Retaining resources after an unconfirmed join is the safety contract.
    # Keep leak detection enabled for the normal lifecycle fixture.
    set_tests_properties(winmm-wait-timeout winmm-wait-failed winmm-stuck-reopen
                         PROPERTIES ENVIRONMENT "ASAN_OPTIONS=detect_leaks=0")
    # Host formatter against black-box retail vectors of the EXE formatter
    # (tools/bios_sprintf_oracle.py regenerates/checks the vector file from the
    # user's own retail EXE).  Standalone: its own guest-memory doubles.
    pe_test_data_present(_pe_ok pe-bios-sprintf-retail-tests tests/bios_sprintf_retail_vectors.tsv)
    if(_pe_ok)
        add_executable(pe-bios-sprintf-retail-tests tests/test_bios_sprintf_retail_vectors.c platform/pe_bios_string.c)
        target_include_directories(pe-bios-sprintf-retail-tests PRIVATE include platform bootstrap)
        target_compile_options(pe-bios-sprintf-retail-tests PRIVATE -ffunction-sections -fdata-sections)
        target_link_options(pe-bios-sprintf-retail-tests PRIVATE -Wl,--gc-sections)
        add_test(NAME bios-sprintf-retail
                 COMMAND pe-bios-sprintf-retail-tests ${CMAKE_CURRENT_SOURCE_DIR}/tests/bios_sprintf_retail_vectors.tsv)
        set_tests_properties(bios-sprintf-retail PROPERTIES TIMEOUT 30)
    endif()
    endif()

    if(UNIX AND NOT APPLE)
        pe_test_inputs_present(_pe_ok pe-ambient-reload-tests tests/test_ambient_reload.c)
        if(_pe_ok)
            add_executable(pe-ambient-reload-tests tests/test_ambient_reload.c)
            target_link_libraries(pe-ambient-reload-tests PRIVATE pe_field_runtime)
            target_link_options(pe-ambient-reload-tests PRIVATE
                "-Wl,--wrap=func_8006CDA4" "-Wl,--wrap=func_80086FF8"
                "-Wl,--wrap=func_8006D2B8" "-Wl,--wrap=func_80086464"
                "-Wl,--wrap=func_80086C5C" "-Wl,--wrap=func_8006DB48")
            if(PE_PORT_SANITIZERS)
                target_compile_options(pe-ambient-reload-tests PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
                target_link_options(pe-ambient-reload-tests PRIVATE -fsanitize=address,undefined -static-libasan -static-libubsan)
            endif()
            add_test(NAME ambient-reload-control-flow COMMAND pe-ambient-reload-tests)
            set_tests_properties(ambient-reload-control-flow PROPERTIES TIMEOUT 30)
        endif()

        pe_test_inputs_present(_pe_ok pe-music-bank-tests tests/test_music_bank.c)
        if(_pe_ok)
            add_executable(pe-music-bank-tests tests/test_music_bank.c)
            target_link_libraries(pe-music-bank-tests PRIVATE pe_field_runtime)
            target_link_options(pe-music-bank-tests PRIVATE
                "-Wl,--wrap=func_8006CDA4" "-Wl,--wrap=func_80086FF8")
            if(PE_PORT_SANITIZERS)
                target_compile_options(pe-music-bank-tests PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
                target_link_options(pe-music-bank-tests PRIVATE -fsanitize=address,undefined -static-libasan -static-libubsan)
            endif()
            add_test(NAME music-bank-control-flow COMMAND pe-music-bank-tests)
            set_tests_properties(music-bank-control-flow PROPERTIES TIMEOUT 30)
        endif()

        pe_test_inputs_present(_pe_ok pe-transition-exit-tests tests/test_transition_exit.c)
        if(_pe_ok)
            add_executable(pe-transition-exit-tests tests/test_transition_exit.c)
            target_link_libraries(pe-transition-exit-tests PRIVATE pe_field_runtime)
            foreach(provider HostFB_SetDispMask HostFB_VSync func_80074A44 func_80074F44 func_80074DC0 func_800755F0 func_80086C5C func_8008CBA8 func_80086FF8 func_80087024)
                target_link_options(pe-transition-exit-tests PRIVATE "-Wl,--wrap=${provider}")
            endforeach()
            if(PE_PORT_SANITIZERS)
                target_compile_options(pe-transition-exit-tests PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
                target_link_options(pe-transition-exit-tests PRIVATE -fsanitize=address,undefined -static-libasan -static-libubsan)
            endif()
            add_test(NAME transition-exit-control-flow COMMAND pe-transition-exit-tests)
            set_tests_properties(transition-exit-control-flow PROPERTIES TIMEOUT 30)
        endif()

        pe_test_inputs_present(_pe_ok pe-transition-constructor-tests tests/test_transition_constructor.c)
        if(_pe_ok)
            add_executable(pe-transition-constructor-tests tests/test_transition_constructor.c)
            target_link_libraries(pe-transition-constructor-tests PRIVATE pe_field_runtime)
            foreach(provider func_80191854 func_800371B0 func_80078E34 func_80078E64 func_80078FC4 func_80077E64 func_80078FE4)
                target_link_options(pe-transition-constructor-tests PRIVATE "-Wl,--wrap=${provider}")
            endforeach()
            if(PE_PORT_SANITIZERS)
                target_compile_options(pe-transition-constructor-tests PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
                target_link_options(pe-transition-constructor-tests PRIVATE -fsanitize=address,undefined -static-libasan -static-libubsan)
            endif()
            add_test(NAME transition-constructor-control-flow COMMAND pe-transition-constructor-tests)
            set_tests_properties(transition-constructor-control-flow PROPERTIES SKIP_RETURN_CODE 77)
            set_tests_properties(transition-constructor-control-flow PROPERTIES TIMEOUT 30)
        endif()

        pe_test_inputs_present(_pe_ok pe-transition-init-tests tests/test_transition_init.c)
        if(_pe_ok)
            add_executable(pe-transition-init-tests tests/test_transition_init.c)
            target_link_libraries(pe-transition-init-tests PRIVATE pe_field_runtime)
            foreach(provider func_8019BD78 func_8005BCB0 func_800371A4 func_80191C94)
                target_link_options(pe-transition-init-tests PRIVATE "-Wl,--wrap=${provider}")
            endforeach()
            if(PE_PORT_SANITIZERS)
                target_compile_options(pe-transition-init-tests PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
                target_link_options(pe-transition-init-tests PRIVATE -fsanitize=address,undefined -static-libasan -static-libubsan)
            endif()
            add_test(NAME transition-init-control-flow COMMAND pe-transition-init-tests)
            set_tests_properties(transition-init-control-flow PROPERTIES TIMEOUT 30)
        endif()

        add_executable(pe-transition-outer-tests tests/test_transition_outer.c)
        target_link_libraries(pe-transition-outer-tests PRIVATE pe_field_runtime)
        foreach(provider func_80196498 func_80191DE8 HostFB_VSync HostFB_SetDispMask pe_plat_bios_srand pe_plat_bios_rand func_8003EB04 func_8006A25C func_801942FC func_8018F05C func_8018F92C func_80194108 func_80192740 func_80192800 func_80193478 func_80191E30 func_80191EFC func_80037870 func_80074DC0 func_80193AB0 func_80074A44 func_80075424 func_800755F0 func_800753B4 func_800752AC func_8019BF8C func_80192030 PE_TransitionCompactOT)
            target_link_options(pe-transition-outer-tests PRIVATE "-Wl,--wrap=${provider}")
        endforeach()
        if(PE_PORT_SANITIZERS)
            target_compile_options(pe-transition-outer-tests PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
            target_link_options(pe-transition-outer-tests PRIVATE -fsanitize=address,undefined -static-libasan -static-libubsan)
        endif()
        add_test(NAME transition-outer-control-flow COMMAND pe-transition-outer-tests)
        set_tests_properties(transition-outer-control-flow PROPERTIES TIMEOUT 30)

        # End-to-end boot -> Day-2 route harness (control-flow/state traversal).
        # Split into its own binary: it drives the whole boot spine for 52k
        # frames, which is far too slow to sit in the default unit suite.
        add_executable(pe-route-boot-day2-tests tests/test_route_boot_day2.c ${PE_GUESTCODE_REGISTRY})
        target_link_libraries(pe-route-boot-day2-tests PRIVATE pe_field_runtime)
        add_test(NAME route-boot-day2-control-flow COMMAND pe-route-boot-day2-tests)
        set_tests_properties(route-boot-day2-control-flow PROPERTIES
            TIMEOUT 600
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/..
        )

        pe_test_inputs_present(_pe_ok pe-transition-loader-tests tests/test_transition_loader.c)
        if(_pe_ok)
            add_executable(pe-transition-loader-tests tests/test_transition_loader.c)
            target_link_libraries(pe-transition-loader-tests PRIVATE pe_field_runtime)
            foreach(provider HostFB_VSync HostFB_SetDispMask func_8006E6A8 func_8006E7E8 func_8006CDA4 func_8006E6D4 pe_plat_cd_read_poll_host pe_plat_gpu_load_tim func_80074DC0 pe_plat_bios_enter_critical pe_plat_bios_flush_cache pe_plat_bios_exit_critical)
                target_link_options(pe-transition-loader-tests PRIVATE "-Wl,--wrap=${provider}")
            endforeach()
            if(PE_PORT_SANITIZERS)
                target_compile_options(pe-transition-loader-tests PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
                target_link_options(pe-transition-loader-tests PRIVATE -fsanitize=address,undefined -static-libasan -static-libubsan)
            endif()
            add_test(NAME transition-loader-control-flow COMMAND pe-transition-loader-tests)
            set_tests_properties(transition-loader-control-flow PROPERTIES TIMEOUT 30)
        endif()
    endif()

    if(PE_PORT_SANITIZERS AND NOT MSVC)
        if(TARGET pe-native-tests)
            target_compile_options(pe-native-tests PRIVATE
                -fsanitize=address,undefined
                -fno-omit-frame-pointer
            )
            target_link_options(pe-native-tests PRIVATE
                -fsanitize=address,undefined
                -static-libasan -static-libubsan
            )
        endif()
        target_compile_options(pe-field-runtime-link-test PRIVATE
            -fsanitize=address,undefined
            -fno-omit-frame-pointer
        )
        target_link_options(pe-field-runtime-link-test PRIVATE
            -fsanitize=address,undefined
            -static-libasan -static-libubsan
        )
    endif()

    if(TARGET pe-native-tests)
        add_test(NAME native-tests COMMAND pe-native-tests)
        set_tests_properties(native-tests PROPERTIES
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/..
        )
    endif()
    add_test(NAME field-runtime-link COMMAND pe-field-runtime-link-test)
    # Retail-data guard (owner decision 2026-09-28: nothing from the game is
    # committed; tests read it from the user's disc).  The guard scans the
    # tracked tree; with the git-ignored disc cache it compares against the
    # real EXE/PE.IMG, otherwise against committed one-way signatures.
    find_package(Python3 COMPONENTS Interpreter QUIET)
    if(Python3_Interpreter_FOUND)
        get_filename_component(PE_REPO_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)
        # The repository tooling is not part of every tree; register each
        # check only where its script exists.
        if(EXISTS "${PE_REPO_ROOT}/tools/analysis/test_retail_data_guard.py")
            add_test(NAME retail-data-guard-selftest
                     COMMAND ${Python3_EXECUTABLE} ${PE_REPO_ROOT}/tools/analysis/test_retail_data_guard.py)
        endif()
        if(EXISTS "${PE_REPO_ROOT}/tools/extract/test_disc_cache.py")
            add_test(NAME disc-cache-selftest
                     COMMAND ${Python3_EXECUTABLE} ${PE_REPO_ROOT}/tools/extract/test_disc_cache.py)
        endif()
        if(EXISTS "${PE_REPO_ROOT}/tools/analysis/retail_data_guard.py")
            add_test(NAME retail-data-guard
                     COMMAND ${Python3_EXECUTABLE} ${PE_REPO_ROOT}/tools/analysis/retail_data_guard.py --quiet-baseline
                     WORKING_DIRECTORY ${PE_REPO_ROOT})
            set_tests_properties(retail-data-guard PROPERTIES TIMEOUT 1200)
        endif()
    endif()
    set_tests_properties(field-runtime-link PROPERTIES
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/..
    )
endif()
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../tools/analysis/psyq_port_guard.py")
    include(cmake/psyq_guard.cmake)
endif()
# cmake/enh.cmake registers three window-test targets; it is one file, so they
# are registered together or not at all.  Any of the three that was buildable
# on its own but is dropped with the others is named in the skip message too.
set(_pe_enh_ok TRUE)
set(_pe_enh_dropped "")
if(NOT DEFINED BUILD_TESTS OR BUILD_TESTS)
    if(UNIX AND NOT APPLE)
        foreach(_pe_enh_pair
                "pe-enh-window-tests|tests/test_enh_window.c"
                "pe-window-key-merge-tests|tests/test_window_key_merge.c"
                "pe-window-sdl-key-merge-tests|tests/test_window_sdl_key_merge.c")
            string(REPLACE "|" ";" _pe_enh_pair "${_pe_enh_pair}")
            list(GET _pe_enh_pair 0 _pe_enh_target)
            list(GET _pe_enh_pair 1 _pe_enh_src)
            pe_test_inputs_present(_pe_ok ${_pe_enh_target} ${_pe_enh_src} platform/host_key_merge.c)
            if(_pe_ok)
                list(APPEND _pe_enh_dropped ${_pe_enh_target})
            else()
                set(_pe_enh_ok FALSE)
            endif()
        endforeach()
    endif()
endif()
if(NOT _pe_enh_ok)
    pe_note_skipped_tests(${_pe_enh_dropped})
endif()
if(_pe_enh_ok)
    include(cmake/enh.cmake)
endif()
pe_report_skipped_tests()
include(cmake/windows.cmake)
