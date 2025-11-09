#pragma once

#include "Base.hpp"

//------------------------------------------------------------------------------
constexpr size_t operator""_Players( unsigned long long int n )
{
	return static_cast< size_t >( n > 4 ? 4 : n );
}
