#include "Table.hpp"

#include "Rules.hpp"
#include "Seat.hpp"

#include <algorithm>

namespace Riichi
{

//-----------------------------------------------------------------------------
Standings::Standings
(
	Rules const& i_rules
)
	: m_points( i_rules.GetPlayerCount(), i_rules.InitialPoints() )
{}

//-----------------------------------------------------------------------------
std::ostream& operator<<( std::ostream& io_out, Standings const& i_standings )
{
	io_out << "Standings:\n";
	for ( size_t i = 0; i < i_standings.m_points.size(); ++i )
	{
		if ( i > 0 ) { io_out << '\n'; }
		io_out << ToString( ( Seat )i ) << ":\t" << i_standings.m_points[ i ];
	}

	return io_out;
}

//-----------------------------------------------------------------------------
Table::Table
(
	std::unique_ptr<Rules>&& i_rules,
	unsigned int i_shuffleSeed,
	unsigned int i_aiSeed
)
	: m_rules{ std::move( i_rules ) }
	, m_standings{ *m_rules }
	, m_state{ TableState::Tag<TableStateType::Setup>{}, *this }
	, m_shuffleRNG{ i_shuffleSeed }
	, m_aiRNG{ i_aiSeed }
{}

//-----------------------------------------------------------------------------
void Table::AddPlayer
(
	Player&& i_player
)
{
	if ( m_state.Type() == TableStateType::Setup )
	{
		Ensure( m_players.size() < m_rules->GetPlayerCount(), "Too many players added" );
		m_players.emplace_back( std::move( i_player ) );
	}
	else
	{
		Error( "Cannot change players past setup" );
	}
}

//-----------------------------------------------------------------------------
bool Table::Playing
(
)	const
{
	return m_state.Type() != TableStateType::Setup && m_state.Type() != TableStateType::GameOver;
}

}