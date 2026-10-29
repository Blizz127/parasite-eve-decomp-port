# CMake toolchain: Linux x86_64 build of pc_port with zig cc (clang) instead
# of the system gcc.  It separates compiler effects from OS effects (the same
# zig/clang as the Windows cross build, docs/port/WINDOWS.md); the release
# Linux binary stays the gcc build.
#
#   export ZIG_GLOBAL_CACHE_DIR=... ZIG_LOCAL_CACHE_DIR=...   # zig's caches
#   cmake -S pc_port -B build/pcbuild-zig-linux \
#         -DCMAKE_TOOLCHAIN_FILE=pc_port/cmake/zig-linux-x86_64.cmake \
#         -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=OFF
#   cmake --build build/pcbuild-zig-linux --target parasite-eve-port
#
# The target is the native host (zig's glibc for the running system), as with
# the earlier build/zigcc/cc wrapper.  zig's linker rejects -Wl,--wrap, so the
# eight --wrap control-flow test targets cannot link with this toolchain.
include("${CMAKE_CURRENT_LIST_DIR}/zig-common.cmake")
