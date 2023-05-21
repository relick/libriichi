#pragma once

#include "Base.hpp"

#include <cassert>
#include <concepts>
#include <iterator>
#include <limits>
#include <variant>

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
	static constexpr auto Min = t_Min;
	static constexpr auto Max = t_Max;

public:
	CoreType m_val{ t_Default };

	constexpr RestrictedIntegral( CoreType i_val ) : m_val{ i_val } {}

public:
	template<CoreType t_Value>
	static constexpr RestrictedType Set()
	{
		static_assert( t_Value >= t_Min, "Restricted integer provided with too small a value" );
		static_assert( t_Value <= t_Max, "Restricted integer provided with too large a value" );
		return RestrictedType( t_Value );
	}
	constexpr RestrictedIntegral() {}
	constexpr RestrictedIntegral( RestrictedType const& i_o ) : m_val{ i_o.m_val } {}

	constexpr operator CoreType() const { return m_val; }
	constexpr CoreType Get() const { return m_val; }

	constexpr RestrictedType& operator=( RestrictedType const& i_o ) { m_val = i_o.m_val; return *this; }
};

//------------------------------------------------------------------------------
template<typename T, typename S>
concept DifferentTypes = !std::same_as<std::remove_cvref_t<T>, std::remove_cvref_t<S>>;

//------------------------------------------------------------------------------
template<typename T_Tags, T_Tags t_Tag>
class NamedVariantTagHelper
{};

//------------------------------------------------------------------------------
template<typename T_Tags, typename... T_Types>
class NamedVariant
{
public: 
	using Data = std::variant<T_Types...>;
	template<T_Tags t_Tag>
	using Tag = NamedVariantTagHelper<T_Tags, t_Tag>;
	template<T_Tags t_Tag>
	using TypeAt = std::variant_alternative_t<( size_t )t_Tag, Data>;

private:
	Data m_data;

public:
	NamedVariant() = default;

	NamedVariant( NamedVariant&& ) = default;
	NamedVariant( NamedVariant const& ) = default;

	template<typename T_Type>
	NamedVariant( T_Type&& i_type )
		requires DifferentTypes<T_Type, NamedVariant<T_Tags, T_Types...>>
		: m_data( std::move( i_type ) )
	{}

	template<T_Tags t_Tag, typename... T_Args>
	NamedVariant( Tag<t_Tag>, T_Args&&... i_args )
		: m_data( std::in_place_index<( size_t )t_Tag>, std::forward<T_Args>( i_args )... )
	{}

	NamedVariant& operator=( NamedVariant&& ) = default;
	NamedVariant& operator=( NamedVariant const& ) = default;

	template<typename T_Type>
	NamedVariant& operator=( T_Type&& i_type )
		requires DifferentTypes<T_Type, NamedVariant<T_Tags, T_Types...>>
	{
		m_data = std::move( i_type );
		return *this;
	}

	T_Tags Type() const { return ( T_Tags )m_data.index(); }

	template<T_Tags t_Tag>
	TypeAt<t_Tag>& Get() { return std::get<( size_t )t_Tag>( m_data ); }

	template<T_Tags t_Tag>
	TypeAt<t_Tag> const& Get() const { return std::get<( size_t )t_Tag>( m_data ); }

	friend bool operator==( NamedVariant const&, NamedVariant const& ) = default;
	friend auto operator<=>( NamedVariant const&, NamedVariant const& ) = default;
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
	std::array<T_Value, t_EnumCount> data;
	T_Value& operator[]( T_Enum e ) { return data[ ( size_t )e ]; }
	T_Value const& operator[]( T_Enum e ) const { return data[ ( size_t )e ]; }
	auto begin() const { return data.begin(); }
	auto end() const { return data.end(); }
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