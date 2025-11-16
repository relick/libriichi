#pragma once

#include "Base.hpp"
#include "Containers.hpp"

#include "DebugUtils.hpp"
#include "EnumUtils.hpp"
#include "RangeUtils.hpp"

#include <cassert>
#include <concepts>
#include <iterator>
#include <limits>
#include <ranges>

namespace Riichi::Utils
{
//------------------------------------------------------------------------------
// A hodge-podge of useful functions and classes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
struct NullType {};

//------------------------------------------------------------------------------
template<typename T_Container, typename T_Value>
T_Container Append( T_Container i_c, T_Value i_v )
{
	T_Container r = std::move( i_c );
	std::back_inserter( r ) = std::move( i_v );
	return r;
}

//------------------------------------------------------------------------------
template<typename T_Container, typename T_Pred>
bool EraseOneIf( T_Container& i_c, T_Pred i_fnPred )
{
	for ( auto i = i_c.begin(); i != i_c.end(); ++i )
	{
		if ( i_fnPred( *i ) )
		{
			i_c.erase( i );
			return true;
		}
	}

	return false;
}

//------------------------------------------------------------------------------
template<std::integral T_Value, std::predicate<T_Value> T_Pred>
T_Value NextFree( T_Value i_val, T_Pred&& i_pred )
{
	while ( !i_pred( i_val ) )
	{
		++i_val;
	}
	return i_val;
}

}
