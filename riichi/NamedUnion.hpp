#pragma once

#include "Containers.hpp"

#include <concepts>
#include <type_traits>

namespace Riichi
{

//------------------------------------------------------------------------------
template<typename T, typename S>
concept DifferentTypes = !std::same_as<std::decay_t<T>, std::decay_t<S>>;

//------------------------------------------------------------------------------
template<typename T_Tags, T_Tags t_Tag>
class NamedUnionTagHelper
{};

//------------------------------------------------------------------------------
template<typename T_Tags, typename... T_Types>
class NamedUnion
{
public:
	using Data = Union<T_Types...>;
	template<T_Tags t_Tag>
	using Tag = NamedUnionTagHelper<T_Tags, t_Tag>;
	template<T_Tags t_Tag>
	using TypeAt = std::variant_alternative_t<( size_t )t_Tag, Data>;

private:
	Data m_data;

public:
	NamedUnion() = default;

	NamedUnion( NamedUnion&& ) = default;
	NamedUnion( NamedUnion const& ) = default;

	template<typename T_Type>
	NamedUnion( T_Type&& i_type )
		requires DifferentTypes<T_Type, NamedUnion<T_Tags, T_Types...>>
	: m_data( std::move( i_type ) )
	{}

	template<T_Tags t_Tag, typename... T_Args>
	NamedUnion( Tag<t_Tag>, T_Args&&... i_args )
		: m_data( std::in_place_index<( size_t )t_Tag>, std::forward<T_Args>( i_args )... )
	{}

	NamedUnion& operator=( NamedUnion&& ) = default;
	NamedUnion& operator=( NamedUnion const& ) = default;

	template<typename T_Type>
	NamedUnion& operator=( T_Type&& i_type )
		requires DifferentTypes<T_Type, NamedUnion<T_Tags, T_Types...>>
	{
		m_data = std::move( i_type );
		return *this;
	}

	T_Tags Type() const { return ( T_Tags )m_data.index(); }

	template<T_Tags t_Tag>
	TypeAt<t_Tag>& Get() { return std::get<( size_t )t_Tag>( m_data ); }

	template<T_Tags t_Tag>
	TypeAt<t_Tag> const& Get() const { return std::get<( size_t )t_Tag>( m_data ); }

	friend bool operator==( NamedUnion const&, NamedUnion const& ) = default;
	friend auto operator<=>( NamedUnion const&, NamedUnion const& ) = default;
};

}