# Opt-in host-side enhancements (docs/port/ENHANCEMENTS.md, GRAPHICS_OPTIONS.md).
#
# include(cmake/enh.cmake) at the end of CMakeLists.txt.  The enhancement code adds no
# compiled source of its own: the presenter modules (platform/host_filter_*.h) are
# header-only, like host_window_scale.h and host_upscale.h, so the X11, Win32 and SDL
# backends include them directly and nothing needs linking.  This file registers the
# one ctest that runs the real presenter, HostWindow_Blit, against a fake X sink.
# The pure pieces and the registry are tested in the native suite (tests/test_enh_*.h).
if(NOT DEFINED BUILD_TESTS OR BUILD_TESTS)
    if(UNIX AND NOT APPLE)
        # test_enh_window.c includes platform/host_window.c itself (like
        # test_window_scaler_failures.c); gc-sections drops the X11/pad/audio code the
        # test never reaches, so it links against nothing but libc.
        add_executable(pe-enh-window-tests tests/test_enh_window.c)
        target_link_libraries(pe-enh-window-tests PRIVATE m Threads::Threads dl)
        target_include_directories(pe-enh-window-tests PRIVATE include platform bootstrap)
        target_compile_options(pe-enh-window-tests PRIVATE -ffunction-sections -fdata-sections)
        target_link_options(pe-enh-window-tests PRIVATE -Wl,--gc-sections)
        add_test(NAME enh-window COMMAND pe-enh-window-tests)
        set_tests_properties(enh-window PROPERTIES TIMEOUT 60)
    endif()
endif()
