cmake_minimum_required (VERSION 3.8)

project ("libriichi_demo")

# General settings
set (CMAKE_CXX_STANDARD 23)
set (CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")

# Library
set(LIBRIICHI_SOURCE
 "riichi/Hand.cpp"
 "riichi/Table.cpp"
 "riichi/TableState.cpp"
 "riichi/RoundData.cpp"
 "riichi/Yaku_Standard.cpp"
 "riichi/Rules.cpp")
add_library(libriichi STATIC ${LIBRIICHI_SOURCE})
target_compile_options(libriichi PUBLIC "$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

# Demo
add_executable (libriichi_demo "demo.cpp")
target_link_libraries (libriichi_demo PRIVATE libriichi)
target_compile_options(libriichi_demo PRIVATE "$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
