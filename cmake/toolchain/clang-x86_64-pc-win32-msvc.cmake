set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang)

set(triple x86_64-pc-win32-msvc)
include(toolchain/utils/common)
include(toolchain/utils/common-clang)
include(toolchain/utils/vcpkg)
