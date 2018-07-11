cmake_minimum_required(VERSION 3.10)
project(agentd C)

set(CMAKE_C_STANDARD 11)

include_directories(include)
include_directories(lib/vcblockchain/lib/vpr/include/)
include_directories(lib/vcblockchain/lib/vccert/include/)
include_directories(lib/vcblockchain/lib/vccrypt/include/)
include_directories(lib/vcblockchain/lib/vcdb/include/)
link_directories(lib/vcblockchain/build/host/release)

file(GLOB_RECURSE agentd_src src/*.c)
file(GLOB_RECURSE agentd_headers include/*.h)

add_subdirectory(test)

add_executable(agentd
        ${agentd_src}
        ${agentd_headers}
        )
target_link_libraries(agentd vcblockchain)