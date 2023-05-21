#include "TableState.hpp"

#include "Rules.hpp"
#include "Table.hpp"

namespace Riichi::TableStates
{

//-----------------------------------------------------------------------------
void Setup::StartGame
(
)	const
{
	Table& table = m_table.get();

	if ( table.m_players.size() < table.m_rules->GetPlayerCount() )
	{
		Error( "Cannot start game with fewer players than required!" );
		return;
	}

	table.m_state = TableStates::BetweenRounds{ table };
}

//-----------------------------------------------------------------------------
void BetweenRounds::StartRound
(
)	const
{
	Table& table = m_table.get();

	// Make new round data
	if ( table.m_rounds.empty() )
	{
		// First round starts on east, shuffle players
		table.m_rounds.emplace_back(
			Seat::East,
			table.m_players,
			*table.m_rules,
			table.m_shuffleRNG
		);
	}
	else
	{
		// Follow from last round
		RoundData const& lastRound = table.m_rounds.back();
		table.m_rounds.emplace_back(
			lastRound,
			*table.m_rules,
			table.m_shuffleRNG
		);
	}

	RoundData& round = table.m_rounds.back();
	//round.BreakWall( table.m_shuffleRNG );
	// Break wall
	// Deal hands
	// Set to correct turn type

	table.m_state = TableStates::GameOver{ table };
}

//-----------------------------------------------------------------------------
void Turn_AI::Discard
(
)	const
{}

//-----------------------------------------------------------------------------
void Turn_Player::Discard
(
	Tile const& i_tile
)	const
{}

//-----------------------------------------------------------------------------
void BetweenTurns::Pass
(
)	const
{}

//-----------------------------------------------------------------------------
void RobAKanChance::Pass
(
)	const
{}

}