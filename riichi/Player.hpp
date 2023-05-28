#pragma once

#include "Base.hpp"

#include <any>

namespace Riichi
{

//------------------------------------------------------------------------------
enum class PlayerType : EnumValueType
{
	User,
	AI,
};

//------------------------------------------------------------------------------
class Player
{
	PlayerType m_type;
public:
	PlayerType Type() const { return m_type; }

	Player( PlayerType i_type )
		: m_type{ i_type }
	{}
};

//------------------------------------------------------------------------------
class PlayerID
{
	size_t m_index{ SIZE_MAX };
	uint32_t m_tableIdent{ UINT32_MAX };
	explicit PlayerID( size_t i_tableIndex, uint32_t i_tableIdent )
		: m_index{ i_tableIndex }
		, m_tableIdent{ i_tableIdent }
	{}

public:
	friend class Table; // Only tables can set up PlayerIDs

	PlayerID() = default;
	PlayerID( PlayerID const& ) = default;
	PlayerID( PlayerID&& ) = default;
	PlayerID& operator=( PlayerID const& ) = default;
	PlayerID& operator=( PlayerID&& ) = default;
	
	friend bool operator==( PlayerID const&, PlayerID const& ) = default;
};

}