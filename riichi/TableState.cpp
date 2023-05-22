#include "TableState.hpp"

#include "Rules.hpp"
#include "Table.hpp"

namespace Riichi::TableStates
{

//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
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

	// TODO: we call these here, they could technically be part of RoundData construction
	// They could also be exposed as their own states.
	// In other words, they're fine here for now, but maybe this decision needs revisiting later
	round.BreakWall( table.m_shuffleRNG );
	round.DealHands();

	Player const& turnPlayer = round.GetPlayer( round.CurrentTurn(), table );

	switch ( turnPlayer.Type() )
	{
	case PlayerType::User:
	{
		table.m_state = TableStates::Turn_Player{ table };
		break;
	}
	case PlayerType::AI:
	{
		table.m_state = TableStates::Turn_AI{ table };
		break;
	}
	}
}

//------------------------------------------------------------------------------
void Turn_AI::Discard
(
)	const
{
	// TODO
	Table& table = m_table.get();
	table.m_state = TableStates::GameOver{ table };
}

//------------------------------------------------------------------------------
void Turn_Player::Discard
(
	Tile const& i_tile
)	const
{
	// TODO
	Table& table = m_table.get();
	table.m_state = TableStates::GameOver{ table };
}

//------------------------------------------------------------------------------
void BetweenTurns::Pass
(
)	const
{
	// TODO
	Table& table = m_table.get();
	table.m_state = TableStates::GameOver{ table };
}

//------------------------------------------------------------------------------
void RobAKanChance::Pass
(
)	const
{
	// TODO
	Table& table = m_table.get();
	table.m_state = TableStates::GameOver{ table };
}

}