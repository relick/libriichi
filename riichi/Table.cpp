#include "Table.hpp"

#include "Rules.hpp"
#include "Seat.hpp"

#include <algorithm>

namespace Riichi
{

//------------------------------------------------------------------------------
Table::Table
(
	std::unique_ptr<Rules>&& i_rules,
	unsigned int i_shuffleSeed,
	unsigned int i_aiSeed
)
	: m_rules{ std::move( i_rules ) }
	, m_state{ TableState::Tag<TableStateType::Setup>{}, *this }
	, m_shuffleRNG{ i_shuffleSeed }
	, m_aiRNG{ i_aiSeed }
{}

//------------------------------------------------------------------------------
PlayerID Table::AddPlayer
(
	Player&& i_player
)
{
	if ( m_state.Type() == TableStateType::Setup )
	{
		riEnsure( m_players.size() < m_rules->GetPlayerCount(), "Too many players added" );
		m_players.push_back( { std::move( i_player ), m_rules->InitialPoints() } );
		m_playerIDs.push_back( PlayerID{ m_players.size() - 1, m_ident } );

		return m_playerIDs.back();
	}

	riError( "Cannot change players past setup" );
	return {};
}

//------------------------------------------------------------------------------
Player const& Table::GetPlayer
(
	PlayerID i_playerID
)	const
{
	riEnsure( i_playerID.m_tableIdent == m_ident, "Player ID not for this table" );
	return m_players[ i_playerID.m_index ].first;
}

//------------------------------------------------------------------------------
Points Table::GetPoints
(
	PlayerID i_playerID
)	const
{
	riEnsure( i_playerID.m_tableIdent == m_ident, "Player ID not for this table" );
	return m_players[ i_playerID.m_index ].second;
}

//------------------------------------------------------------------------------
Points Table::ModifyPoints
(
	PlayerID i_playerID,
	Points i_amount
)
{
	riEnsure( i_playerID.m_tableIdent == m_ident, "Player ID not for this table" );
	m_players[ i_playerID.m_index ].second += i_amount;

	return m_players[ i_playerID.m_index ].second;
}

//------------------------------------------------------------------------------
std::ostream& Table::PrintStandings
(
	std::ostream& io_out
)	const
{
	io_out << "Standings:\n";
	for ( size_t i = 0; i < m_players.size(); ++i )
	{
		if ( i > 0 ) { io_out << '\n'; }
		io_out << "Player " << i << ":\t" << m_players[ i ].second;
	}

	return io_out;
}

//------------------------------------------------------------------------------
void Table::Transition
(
	TableState&& i_nextState,
	TableEvent&& i_nextEvent
)
{
	m_state = std::move( i_nextState );
	m_mostRecentEvent = std::move( i_nextEvent );
}

//------------------------------------------------------------------------------
bool Table::Playing
(
)	const
{
	return m_state.Type() != TableStateType::Setup && m_state.Type() != TableStateType::GameOver;
}

}