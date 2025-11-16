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

# Library
add_library(libriichi STATIC)
target_sources(libriichi PUBLIC FILE_SET HEADERS FILES
	# Library Support
	riichi/Base.hpp
	riichi/Containers.hpp
	riichi/Random.hpp
	riichi/DebugUtils.hpp
	riichi/EnumUtils.hpp
	riichi/RangeUtils.hpp
	riichi/Utils.hpp

	# Riichi Mahjong Engine
	riichi/Declare.hpp
	riichi/Hand.hpp
	riichi/Hand.inl
	riichi/HandInterpreter.hpp
	riichi/NamedUnion.hpp
	riichi/Player.hpp
	riichi/PlayerCount.hpp
	riichi/Round.hpp
	riichi/Seat.hpp
	riichi/Table.hpp
	riichi/Table.inl
	riichi/TableEvent.hpp
	riichi/TableState.hpp
	riichi/Tile.hpp
	riichi/Yaku.hpp

	# Example Riichi implementation
	riichi/Declare_Standard.hpp
	riichi/HandInterpreter_Standard.hpp
	riichi/Rules_Standard.hpp
	riichi/Rules_Standard.inl
	riichi/Yaku_Standard.hpp
	riichi/Yaku_Standard.inl
)
target_sources(libriichi PRIVATE
	# Riichi Mahjong Engine
	riichi/Hand.cpp
	riichi/HandInterpreter.cpp
	riichi/Round.cpp
	riichi/Table.cpp
	riichi/TableState.cpp
	riichi/Tile.cpp
	
	# Example Riichi implementation
	riichi/HandInterpreter_Standard.cpp
	riichi/Yaku_Standard.cpp
	riichi/Rules_Standard.cpp
)
target_link_libraries(libriichi PRIVATE libriichi_common_flags)
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

set_property(TARGET libriichi libriichi_demo libriichi_hand_tests libriichi_tests PROPERTY CXX_STANDARD 23)
