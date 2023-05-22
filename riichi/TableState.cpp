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

	table.Transition( TableStates::BetweenRounds{ table }, TableEvent::Tag<TableEventType::None>() );
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
	Tile const& firstDrawnTile = round.DealHands();

	Player const& turnPlayer = round.GetPlayer( round.CurrentTurn(), table );

	switch ( turnPlayer.Type() )
	{
	case PlayerType::User:
	{
		table.Transition(
			TableStates::Turn_Player{table, round.CurrentTurn()},
			TableEvent{ TableEvent::Tag<TableEventType::DealerDraw>(), firstDrawnTile }
		);
		break;
	}
	case PlayerType::AI:
	{
		table.Transition(
			TableStates::Turn_AI{table, round.CurrentTurn()},
			TableEvent{ TableEvent::Tag<TableEventType::DealerDraw>(), firstDrawnTile }
		);
		break;
	}
	}
}

//------------------------------------------------------------------------------
Turn_AI::Turn_AI
(
	Table& i_table,
	Seat i_seat
)
	: Base{ i_table }
	, m_seat{ i_seat }
{}

//------------------------------------------------------------------------------
void Turn_AI::Discard
(
)	const
{
	Table& table = m_table.get();

	// TODO: Super good AI goes here
	// For now, we just discard the drawn tile
	RoundData& round = table.m_rounds.back();
	Tile const discardedTile = round.DiscardDrawn();

	table.Transition( TableStates::BetweenTurns{table}, TableEvents::Discard{discardedTile} );
}

//------------------------------------------------------------------------------
Turn_Player::Turn_Player
(
	Table& i_table,
	Seat i_seat
)
	: Base{ i_table }
	, m_seat{ i_seat }
{}

//------------------------------------------------------------------------------
void Turn_Player::Discard
(
	Tile const& i_tile
)	const
{
	// TODO
	Table& table = m_table.get();
	table.Transition( TableStates::GameOver{table}, std::string( "nyi" ) );
}

//------------------------------------------------------------------------------
void BetweenTurns::Pass
(
)	const
{
	Table& table = m_table.get();

	RoundData& round = table.m_rounds.back();

	if ( round.WallTilesRemaining() == 0u )
	{
		table.Transition( TableStates::GameOver{table}, TableEvents::WallDepleted{} );
		return;
	}

	Tile const& drawnTile = round.PassCalls();

	Player const& turnPlayer = round.GetPlayer( round.CurrentTurn(), table );

	switch ( turnPlayer.Type() )
	{
	case PlayerType::User:
	{
		table.Transition(
			TableStates::Turn_Player{table, round.CurrentTurn()},
			TableEvent{ TableEvent::Tag<TableEventType::Draw>(), drawnTile }
		);
		break;
	}
	case PlayerType::AI:
	{
		table.Transition(
			TableStates::Turn_AI{table, round.CurrentTurn()},
			TableEvent{ TableEvent::Tag<TableEventType::Draw>(), drawnTile }
		);
		break;
	}
	}
}

//------------------------------------------------------------------------------
void RobAKanChance::Pass
(
)	const
{
	// TODO
	Table& table = m_table.get();
	table.Transition( TableStates::GameOver{table}, std::string( "nyi" ) );
}

}