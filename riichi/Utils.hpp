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

template<auto t_EnumMin, auto t_EnumMax>
concept ValidEnumMinMax = requires {
	requires std::is_enum_v<decltype( t_EnumMin )>;
	requires std::same_as<decltype( t_EnumMin ), decltype( t_EnumMax )>;
	requires t_EnumMin <= t_EnumMax;
};

//------------------------------------------------------------------------------
template<auto t_EnumMin, auto t_EnumMax>
	requires ValidEnumMinMax<t_EnumMin, t_EnumMax>
struct EnumRange
{
	using T_Enum = decltype( t_EnumMin );

	static constexpr T_Enum Min() { return t_EnumMin; }
	static constexpr T_Enum Max() { return t_EnumMax; }
	static constexpr size_t Count() { return static_cast< size_t >( Max() ) + 1 - static_cast< size_t >( Min() ); }

	static constexpr size_t BeginValue() { return static_cast< size_t >( Min() ); }
	static constexpr size_t EndValue() { return BeginValue() + Count(); }

	static T_Enum IndexToValue( size_t i_index )
	{
		riEnsure( i_index < Count(), "Index too large for enum range" );
		return static_cast< T_Enum >( i_index + BeginValue() );
	}

	static size_t ValueToIndex( T_Enum i_value )
	{
		riEnsure( i_value >= Min(), "Enum value too small for enum range" );
		riEnsure( i_value <= Max(), "Enum value too large for enum range" );
		return static_cast< size_t >( i_value ) - BeginValue();
	}

	static constexpr bool InRange( T_Enum i_value ) { return i_value >= Min() && i_value <= Max(); }
	static constexpr T_Enum NextWrapped( T_Enum i_value ) { return IndexToValue( ( ValueToIndex( i_value ) + 1 ) % Count() ); }
	static constexpr T_Enum PrevWrapped( T_Enum i_value ) { return IndexToValue( ( ValueToIndex( i_value ) + Count() - 1 ) % Count()); }

	struct EnumIter
	{
		size_t m_enumPos{ EndValue() };

		void operator++() { ++m_enumPos; }
		T_Enum operator*() { return ( T_Enum )m_enumPos; }
		bool operator!=( EnumIter const& b ) const { return m_enumPos != b.m_enumPos; }
	};

	static constexpr EnumIter begin() { return EnumIter{ BeginValue() }; }
	static constexpr EnumIter end() { return EnumIter{ EndValue() }; }
};

template<typename T_EnumRange>
concept ValidEnumRange = requires {
	T_EnumRange::Min();
	T_EnumRange::Max();
	{ T_EnumRange::Count() } -> std::convertible_to<size_t>;
	{ T_EnumRange::BeginValue() } -> std::convertible_to<size_t>;
	{ T_EnumRange::EndValue() } -> std::convertible_to<size_t>;
	requires ValidEnumMinMax<T_EnumRange::Min(), T_EnumRange::Max()>;
};

//------------------------------------------------------------------------------
template<typename T_Value, typename T_EnumRange>
	requires ValidEnumRange<T_EnumRange>
struct EnumArray
{
	using T_Enum = T_EnumRange::T_Enum;

	static constexpr size_t Size() { return T_EnumRange::Count(); }

	Array<T_Value, Size()> m_data;
	T_Value& operator[]( T_Enum i_value ) { return m_data[ T_EnumRange::ValueToIndex( i_value ) ]; }
	T_Value const& operator[]( T_Enum i_value ) const { return m_data[ T_EnumRange::ValueToIndex( i_value ) ]; }
	auto begin() const { return m_data.begin(); }
	auto end() const { return m_data.end(); }
};

//------------------------------------------------------------------------------
template<typename T_EnumRange>
	requires ValidEnumRange<T_EnumRange>
class EnumSet
{
public:
	using T_Enum = T_EnumRange::T_Enum;

private:
	Utils::EnumArray<bool, T_EnumRange> m_enumVals{};
	size_t m_size{ 0 }; // TODO-OPT: could also not have this member and instead do a count when Size() is called.

public:
	struct Iter
	{
		EnumSet const* m_set{ nullptr };
		size_t m_enumPos{ T_EnumRange::EndValue() };

		void operator++() { do { ++m_enumPos; } while ( m_enumPos < T_EnumRange::EndValue() && !m_set->Contains( ( T_Enum )m_enumPos ) ); }
		T_Enum operator*() { return ( T_Enum )m_enumPos; }
		bool operator!=( Iter const& b ) const { return m_enumPos != b.m_enumPos; }
	};

	Iter begin() const { return Iter{ this, [ this ] { size_t pos = T_EnumRange::BeginValue(); while ( pos < T_EnumRange::EndValue() && !Contains( ( T_Enum )pos ) ) { ++pos; } return pos; }() }; }
	Iter end() const { return Iter{ this, T_EnumRange::EndValue() }; }

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
		for ( T_Enum val : T_EnumRange{} )
		{
			if ( !Contains( val ) && i_o.Contains( val ) )
			{
				return false;
			}
		}
		return true;
	}
	size_t Size() const { return m_size; }

	friend EnumSet operator~( EnumSet const& a )
	{
		EnumSet negated = a;
		for ( T_Enum val : T_EnumRange{} )
		{
			if ( negated.Contains( val ) )
			{
				negated.Erase( val );
			}
			else
			{
				negated.Insert( val );
			}
		}
		return negated;
	}
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
template<typename T_Container, typename T_Value>
bool EraseOne( T_Container& i_c, T_Value const& i_val )
{
	for ( auto i = i_c.begin(); i != i_c.end(); ++i )
	{
		if ( *i == i_val )
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