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
#define Ensure(...)
#define Error(MSG)
#else
#define Ensure(TEST, ...) assert((__VA_ARGS__, TEST))
#define Error(MSG) Ensure(false, MSG)
#endif

//------------------------------------------------------------------------------
template<
	std::integral T_Integral,
	T_Integral t_Min = std::numeric_limits<T_Integral>::min(),
	T_Integral t_Default = T_Integral(),
	T_Integral t_Max = std::numeric_limits<T_Integral>::max()
>
class RestrictedIntegral
{
public:
	using CoreType = T_Integral;
	using RestrictedType = RestrictedIntegral<CoreType, t_Min, t_Default, t_Max>;
	
	struct BlessedValue { T_Integral m_val; };

	static constexpr BlessedValue Min{ t_Min };
	static constexpr BlessedValue Max{ t_Max };

	struct RestrictedTypeRange
	{
		struct Iter
		{
			T_Integral m_current;

			void operator++() { ++m_current; }
			RestrictedType operator*() { return RestrictedType{ m_current }; }
			bool operator!=( Iter const& b ) const { return m_current != b.m_current; }
		};

		T_Integral m_lower;
		T_Integral m_upper;

		RestrictedTypeRange( T_Integral i_lower, T_Integral i_upper, bool i_includeUpperBound )
			: m_lower{ i_lower }
			, m_upper{ T_Integral( i_upper + ( i_includeUpperBound ? 1 : 0 ) ) }
		{}

		Iter begin() const { return Iter{ m_lower }; }
		Iter end() const { return Iter{ m_upper }; }
	};

public:
	CoreType m_val{ t_Default };

	explicit constexpr RestrictedIntegral( CoreType i_val ) : m_val{ i_val } {}
	constexpr RestrictedIntegral( BlessedValue i_val ) : m_val{ i_val.m_val } {}

public:
	template<CoreType t_Value>
	static constexpr RestrictedType Set()
	{
		static_assert( t_Value >= t_Min, "Restricted integer provided with too small a value" );
		static_assert( t_Value <= t_Max, "Restricted integer provided with too large a value" );
		return RestrictedType( t_Value );
	}

	static constexpr RestrictedTypeRange ExclusiveRange( RestrictedType i_l, RestrictedType i_u )
	{
		Ensure( i_l.m_val <= i_u.m_val, "Lower bound must be <= upper bound in range" );
		return RestrictedTypeRange( i_l.m_val, i_u.m_val, false );
	}

	static constexpr RestrictedTypeRange InclusiveRange( RestrictedType i_l, RestrictedType i_u )
	{
		Ensure( i_l.m_val <= i_u.m_val, "Lower bound must be <= upper bound in range" );
		return RestrictedTypeRange( i_l.m_val, i_u.m_val, true );
	}

	constexpr RestrictedIntegral() {}
	constexpr RestrictedIntegral( RestrictedType const& i_o ) : m_val{ i_o.m_val } {}

	constexpr operator CoreType() const { return m_val; }
	constexpr CoreType Get() const { return m_val; }

	constexpr RestrictedType& operator=( RestrictedType const& i_o ) { m_val = i_o.m_val; return *this; }

	constexpr RestrictedType& operator++()
	{
		Ensure( m_val < t_Max, "Can not increment value past max" );
		++m_val;
		return *this;
	}

	constexpr RestrictedType& operator--()
	{
		Ensure( m_val > t_Min, "Can not decrement value past min" );
		--m_val;
		return *this;
	}

	friend constexpr RestrictedType operator+( RestrictedType const& i_a, RestrictedType const& i_b )
	{
		if ( i_b.m_val < 0 )
		{
			Ensure( i_a.m_val >= t_Min - i_b.m_val, "Can not add value past min" );
		}
		else
		{
			Ensure( i_a.m_val <= t_Max - i_b.m_val, "Can not add value past max" );
		}
		return RestrictedType{ ( T_Integral )( i_a.m_val + i_b.m_val ) };
	}

	friend constexpr RestrictedType operator-( RestrictedType const& i_a, RestrictedType const& i_b )
	{
		if ( i_b.m_val >= 0 )
		{
			Ensure( i_a.m_val >= t_Min + i_b.m_val, "Can not sub value past min" );
		}
		else
		{
			Ensure( i_a.m_val <= t_Max + i_b.m_val, "Can not sub value past max" );
		}
		return RestrictedType{ ( T_Integral )( i_a.m_val - i_b.m_val ) };
	}

	friend constexpr bool operator==( RestrictedType const&, RestrictedType const& ) = default;
};

//------------------------------------------------------------------------------
struct NullType{};

//------------------------------------------------------------------------------
template<typename T_Enum, size_t t_EnumCount>
struct EnumRange
{
	struct EnumIter
	{
		size_t m_enumPos{ t_EnumCount };

		void operator++() { ++m_enumPos; }
		T_Enum operator*() { return ( T_Enum )m_enumPos; }
		bool operator!=( EnumIter const& b ) const { return m_enumPos != b.m_enumPos; }
	};

	static EnumIter begin() { return EnumIter{ 0 }; }
	static EnumIter end() { return EnumIter{ t_EnumCount }; }
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

public:
	struct Iter
	{
		EnumSet const* m_set{ nullptr };
		size_t m_enumPos{ t_EnumCount };

		void operator++() { do { ++m_enumPos; } while ( m_enumPos < t_EnumCount && !m_set->Contains( ( T_Enum )m_enumPos ) ); }
		T_Enum operator*() { return ( T_Enum )m_enumPos; }
		bool operator!=( Iter const& b ) const { return m_enumPos != b.m_enumPos; }
	};

	Iter begin() const { return Iter{ this, 0 }; }
	Iter end() const { return Iter{ this, t_EnumCount }; }

	explicit EnumSet() = default;
	explicit EnumSet( std::initializer_list<T_Enum> i_vals )
	{
		for ( T_Enum val : i_vals )
		{
			m_enumVals[ val ] = true;
		}
	}

	void Insert( T_Enum i_val ) { m_enumVals[ i_val ] = true; }
	void Erase( T_Enum i_val ) { m_enumVals[ i_val ] = false; }
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
void EraseOne( T_Container& i_c, T_Pred i_fnPred )
{
	for ( auto i = i_c.begin(); i != i_c.end(); ++i )
	{
		if ( i_fnPred( *i ) )
		{
			i_c.erase( i );
			return;
		}
	}
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