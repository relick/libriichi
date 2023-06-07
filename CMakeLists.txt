cmake_minimum_required (VERSION 3.20)

project ("libriichi_demo")

add_library (libriichi_common_flags INTERFACE)

target_compile_options (libriichi_common_flags INTERFACE
	$<$<CXX_COMPILER_ID:Clang,GNU>:
		-Wno-old-style-cast
		-Wno-unused-parameter
	>

	$<$<CXX_COMPILER_ID:MSVC>:
		/Zc:preprocessor
		/utf-8
		
		/wd5105 # /Zc:preprocessor unfortunately makes winbase.h complain about undefined behaviour. Since this is MS's, we'll just have to put up with it.
	>
)

# General settings
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
target_link_libraries(libriichi PRIVATE libriichi_common_flags)
target_include_directories(libriichi PUBLIC ".")
target_include_directories(libriichi PUBLIC "range-v3/include")

# Demo
add_executable (libriichi_demo "demo.cpp")
target_link_libraries (libriichi_demo PRIVATE libriichi libriichi_common_flags)

# Hand tests
add_executable (libriichi_hand_tests "hand_tests.cpp")
target_link_libraries (libriichi_hand_tests PRIVATE libriichi libriichi_common_flags)

# Tests
add_executable (libriichi_tests "tests.cpp")
target_link_libraries (libriichi_tests PRIVATE libriichi libriichi_common_flags)

add_test(NAME tests COMMAND libriichi_tests)

set_property(TARGET libriichi libriichi_demo libriichi_hand_tests libriichi_tests PROPERTY CXX_STANDARD 20)
