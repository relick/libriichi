#pragma once

#include "Base.hpp"
#include "AI.hpp"

#include <memory>

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
	std::unique_ptr<AI::Agent> m_aiAgent;
public:
	PlayerType Type() const { return m_aiAgent ? PlayerType::AI : PlayerType::User; }
	AI::Agent& Agent() const { riEnsure( Type() == PlayerType::AI, "Must only call Player::Agent() on AI players" ); return *m_aiAgent; }

	// User has no params
	Player()
	{}

	// AI requires an agent to be provided
	Player( std::unique_ptr<AI::Agent>&& i_agent )
		: m_aiAgent{ std::move( i_agent ) }
	{}
};

//------------------------------------------------------------------------------
class PlayerID
{
	size_t m_index{ SIZE_MAX };
	TableIdent m_tableIdent;
	explicit PlayerID( size_t i_tableIndex, TableIdent i_tableIdent )
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