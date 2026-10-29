# CMake toolchain: cross-compile the native port for macOS x86_64 with zig.
#   PE_ZIG=/path/to/zig cmake -S pc_port -B build/pcbuild-macos-x86_64 \
#     -DCMAKE_TOOLCHAIN_FILE=$PWD/pc_port/cmake/zig-x86_64-macos.cmake \
#     -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=OFF
#   PE_ZIG=/path/to/zig cmake --build build/pcbuild-macos-x86_64 --target parasite-eve-port
# APPLE builds always use the SDL2 host backend (pc_port/cmake/sdl.cmake);
# SDL2 is dlopened at run time.  See docs/port/MACOS.md.
set(PE_ZIG_MACOS_ARCH x86_64)
include("${CMAKE_CURRENT_LIST_DIR}/zig/macos-toolchain.cmake")
