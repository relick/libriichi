#pragma once

#include "Containers.hpp"
#include "Declare.hpp"

#include <compare>
#include <limits>

namespace Riichi
{

//------------------------------------------------------------------------------
// IDs with strongly separated types and sentinel/null values
//------------------------------------------------------------------------------
template<typename T_Tag, typename T_CoreType = uint32_t, T_CoreType t_NullValue = std::numeric_limits<T_CoreType>::max()>
class TypeSafeID
{
	T_CoreType m_value{ t_NullValue };

public:
	using CoreType = T_CoreType;

	constexpr TypeSafeID() = default;
	constexpr explicit TypeSafeID( CoreType i_value ) : m_value{ i_value } {}

	constexpr bool IsValid() const { return !IsNull(); }
	constexpr bool IsNull() const { return m_value == t_NullValue; }

	constexpr CoreType GetValue() const { return m_value; }

	constexpr bool operator==( TypeSafeID const& i_other ) const = default;
	constexpr bool operator<( TypeSafeID const& i_other ) const = default;
	constexpr std::strong_ordering operator<=>( TypeSafeID const& i_other ) const = default;
};

//------------------------------------------------------------------------------
// Generator for IDs
//------------------------------------------------------------------------------
template<typename T_IDType>
class TypeSafeIDGenerator
{
	T_IDType m_nextID{ 0 };
public:
	TypeSafeIDGenerator()
	{
		// In case 0 is the null value..!
		if ( m_nextID.IsNull() )
		{
			m_nextID = T_IDType{ m_nextID.GetValue() + 1 };
		}
	}

	T_IDType operator()()
	{
		T_IDType next = m_nextID;
		m_nextID = T_IDType{ m_nextID.GetValue() + 1 };
		return next;
	}
};

//------------------------------------------------------------------------------
// Wrapper of any array type to allow indexing by IDs
//------------------------------------------------------------------------------
template<typename T_ArrayType, typename T_IDType>
class TypeSafeIDArray
	: public T_ArrayType
{
public:
	using T_ArrayType::T_ArrayType;

	typename T_ArrayType::reference operator[]( T_IDType i_id )
	{
		return T_ArrayType::operator[]( i_id.GetValue() );
	}

	typename T_ArrayType::const_reference operator[]( T_IDType i_id ) const
	{
		return T_ArrayType::operator[]( i_id.GetValue() );
	}
};

//------------------------------------------------------------------------------
// Important ID Types
//------------------------------------------------------------------------------
using TableIdent = TypeSafeID<struct TableIdentTag>;

}

//------------------------------------------------------------------------------
template<typename T_Tag, typename T_CoreType, T_CoreType t_NullValue>
struct std::hash<Riichi::TypeSafeID<T_Tag, T_CoreType, t_NullValue>>
{
	std::size_t operator()( Riichi::TypeSafeID<T_Tag, T_CoreType, t_NullValue> const& i_id ) const noexcept
	{
		return std::hash<T_CoreType>{}( i_id.GetValue() );
	}
};