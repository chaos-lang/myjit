cmake_minimum_required(VERSION 3.19)
project(myjit_master C)

set(CMAKE_C_STANDARD 11)

include_directories(disasm)
include_directories(disasm/arm32)
include_directories(disasm/sparc)
include_directories(disasm/udis86)
include_directories(extras)
include_directories(myjit)
include_directories(test)

add_executable(myjit_master
        myjit/jitlib-core.c
        demo1.c)
