# Shared part of the zig toolchain files (zig-linux-x86_64.cmake,
# zig-windows-x86_64.cmake): the one `zig` executable is the C compiler, the
# archiver and ranlib.  Requires CMake >= 3.19 (file(CHMOD)).
#
# PE_ZIG selects the zig executable (default: `zig` on PATH; tested with
# 0.13.0).  zig writes its caches to ZIG_GLOBAL_CACHE_DIR / ZIG_LOCAL_CACHE_DIR
# when they are set in the environment of the configure and of the build.
if(NOT PE_ZIG)
    find_program(PE_ZIG NAMES zig REQUIRED)
endif()
set(PE_ZIG "${PE_ZIG}" CACHE FILEPATH "zig executable used as C compiler, archiver and ranlib")
# try_compile projects re-read the toolchain file with their own cache.
list(APPEND CMAKE_TRY_COMPILE_PLATFORM_VARIABLES PE_ZIG)

# A two-element CMAKE_C_COMPILER is split by CMake into the compiler and
# CMAKE_C_COMPILER_ARG1, so every compile and link runs `zig cc ...`.
set(CMAKE_C_COMPILER "${PE_ZIG}" cc)

# CMake runs the archiver as a single program (`<CMAKE_AR> qc ...`, also in
# the MinGW executable rule), and zig has no argv[0] dispatch, so the build
# tree gets two one-line forwarders to `zig ar` / `zig ranlib` (llvm-ar,
# which also indexes COFF objects).
set(_pe_zig_tools "${CMAKE_BINARY_DIR}/pe-zig-tools")
file(MAKE_DIRECTORY "${_pe_zig_tools}")
foreach(_pe_tool ar ranlib)
    if(CMAKE_HOST_WIN32)
        set(_pe_tool_path "${_pe_zig_tools}/zig-${_pe_tool}.cmd")
        file(WRITE "${_pe_tool_path}" "@\"${PE_ZIG}\" ${_pe_tool} %*\r\n")
    else()
        set(_pe_tool_path "${_pe_zig_tools}/zig-${_pe_tool}")
        file(WRITE "${_pe_tool_path}" "#!/bin/sh\nexec '${PE_ZIG}' ${_pe_tool} \"$@\"\n")
        file(CHMOD "${_pe_tool_path}" PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
             GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
    endif()
    string(TOUPPER "${_pe_tool}" _pe_tool_up)
    set(CMAKE_${_pe_tool_up} "${_pe_tool_path}" CACHE FILEPATH "zig ${_pe_tool} forwarder" FORCE)
endforeach()
