cmake_minimum_required (VERSION 3.8)

project ("libriichi_demo")

# General settings
set (CMAKE_CXX_STANDARD 23)
set (CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")

# Library
set(LIBRIICHI_SOURCE
 "riichi/Hand.cpp"
 "riichi/HandInterpreter.cpp"
 "riichi/Round.cpp"
 "riichi/Table.cpp"
 "riichi/TableState.cpp"
 "riichi/Tile.cpp"
 "riichi/HandInterpreter_Standard.cpp"
 "riichi/Yaku_Standard.cpp"
 "riichi/Rules_Standard.cpp"
)
add_library(libriichi STATIC ${LIBRIICHI_SOURCE})
target_compile_options(libriichi PUBLIC "$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

# Demo
add_executable (libriichi_demo "demo.cpp")
target_link_libraries (libriichi_demo PRIVATE libriichi)

# Hand tests
add_executable (libriichi_hand_tests "hand_tests.cpp")
target_link_libraries (libriichi_hand_tests PRIVATE libriichi)

# Tests
add_executable (libriichi_tests "tests.cpp")
target_link_libraries (libriichi_tests PRIVATE libriichi)
