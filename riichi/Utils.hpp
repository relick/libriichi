#pragma once

#include "Base.hpp"
#include "Containers.hpp"

#include <cassert>
#include <concepts>
#include <iterator>
#include <limits>

namespace Riichi::Utils
{
//------------------------------------------------------------------------------
// A hodge-podge of useful functions and classes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
#if NDEBUG
#define riEnsure(...)
#define riError(MSG)
#else
#define riEnsure(TEST, ...) assert((__VA_ARGS__, TEST))
#define riError(MSG) riEnsure(false, MSG)
#endif

//------------------------------------------------------------------------------
struct NullType{};

//------------------------------------------------------------------------------
template<typename T_Enum, size_t t_EnumEnd, size_t t_EnumBegin = 0>
struct EnumRange
{
	struct EnumIter
	{
		size_t m_enumPos{ t_EnumEnd };

		void operator++() { ++m_enumPos; }
		T_Enum operator*() { return ( T_Enum )m_enumPos; }
		bool operator!=( EnumIter const& b ) const { return m_enumPos != b.m_enumPos; }
	};

	static EnumIter begin() { return EnumIter{ t_EnumBegin }; }
	static EnumIter end() { return EnumIter{ t_EnumEnd }; }
};

//------------------------------------------------------------------------------
template<typename T_Value, typename T_Enum, size_t t_EnumCount>
struct EnumIndexedArray
{
	Array<T_Value, t_EnumCount> data;
	T_Value& operator[]( T_Enum e ) { return data[ ( size_t )e ]; }
	T_Value const& operator[]( T_Enum e ) const { return data[ ( size_t )e ]; }
	auto begin() const { return data.begin(); }
	auto end() const { return data.end(); }
};

//------------------------------------------------------------------------------
template<typename T_Enum, size_t t_EnumCount>
class EnumSet
{
	Utils::EnumIndexedArray<bool, T_Enum, t_EnumCount> m_enumVals{};
	size_t m_size{ 0 }; // TODO-OPT: could also not have this member and instead do a count when Size() is called.

public:
	struct Iter
	{
		EnumSet const* m_set{ nullptr };
		size_t m_enumPos{ t_EnumCount };

		void operator++() { do { ++m_enumPos; } while ( m_enumPos < t_EnumCount && !m_set->Contains( ( T_Enum )m_enumPos ) ); }
		T_Enum operator*() { return ( T_Enum )m_enumPos; }
		bool operator!=( Iter const& b ) const { return m_enumPos != b.m_enumPos; }
	};

	Iter begin() const { return Iter{ this, [ this ] { size_t pos = 0; while ( pos < t_EnumCount && !Contains( ( T_Enum )pos ) ) { ++pos; } return pos; }() }; }
	Iter end() const { return Iter{ this, t_EnumCount }; }

	explicit EnumSet() = default;
	explicit EnumSet( std::initializer_list<T_Enum> i_vals )
	{
		for ( T_Enum val : i_vals )
		{
			m_enumVals[ val ] = true;
			++m_size;
		}
	}

	void Insert( T_Enum i_val ) { if ( !m_enumVals[ i_val ] ) { ++m_size; } m_enumVals[ i_val ] = true; }
	void Erase( T_Enum i_val ) { if ( m_enumVals[ i_val ] ) { --m_size; } m_enumVals[ i_val ] = false; }
	bool Contains( T_Enum i_val ) const { return m_enumVals[ i_val ]; }
	bool ContainsAllOf( EnumSet const& i_o ) const
	{
		for ( T_Enum val : EnumRange<T_Enum, t_EnumCount>{} )
		{
			if ( !Contains( val ) && i_o.Contains( val ) )
			{
				return false;
			}
		}
		return true;
	}
	size_t Size() const { return m_size; }
};

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
bool EraseOne( T_Container& i_c, T_Pred i_fnPred )
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