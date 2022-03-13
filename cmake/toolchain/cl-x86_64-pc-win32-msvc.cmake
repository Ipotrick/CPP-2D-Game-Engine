include(toolchain/utils/vcvarsall-setup)

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)
set(CMAKE_C_COMPILER cl)
set(CMAKE_CXX_COMPILER cl)

include(toolchain/utils/common)
include(toolchain/utils/common-cl)
include(toolchain/utils/vcpkg)
