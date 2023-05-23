#include "TableState.hpp"
#include "TableState.hpp"
#include "TableState.hpp"
#include "TableState.hpp"
#include "TableState.hpp"
#include "TableState.hpp"
#include "TableState.hpp"
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

	// TODO-DEBT: we call these here, they could technically be part of RoundData construction
	// They could also be exposed as their own states.
	// In other words, they're fine here for now, but maybe this decision needs revisiting later
	round.BreakWall( table.m_shuffleRNG );
	Tile const& firstDrawnTile = round.DealHands();

	Player const& turnPlayer = round.GetPlayer( round.CurrentTurn(), table );

	switch ( turnPlayer.Type() )
	{
	case PlayerType::User:
	{
		// TODO-MVP: calculate tsumo/riichi/kan
		table.Transition(
			TableStates::Turn_User{table, round.CurrentTurn(), false, false, false},
			TableEvent{ TableEvent::Tag<TableEventType::DealerDraw>(), firstDrawnTile, round.CurrentTurn() }
		);
		break;
	}
	case PlayerType::AI:
	{
		table.Transition(
			TableStates::Turn_AI{table, round.CurrentTurn()},
			TableEvent{ TableEvent::Tag<TableEventType::DealerDraw>(), firstDrawnTile, round.CurrentTurn() }
		);
		break;
	}
	}
}

//------------------------------------------------------------------------------
BaseTurn::BaseTurn
(
	Table& i_table,
	Seat i_seat
)
	: Base{ i_table }
	, m_seat{ i_seat }
{}

//------------------------------------------------------------------------------
Hand const& BaseTurn::GetHand
(
)	const
{
	Table& table = m_table.get();
	return table.GetRoundData().GetHand( m_seat );
}

//------------------------------------------------------------------------------
Turn_AI::Turn_AI
(
	Table& i_table,
	Seat i_seat
)
	: BaseTurn{ i_table, i_seat }
{}

//------------------------------------------------------------------------------
void Turn_AI::MakeDecision
(
)	const
{
	Table& table = m_table.get();

	// TODO-AI: Super good AI goes here
	// For now, we just discard the drawn tile
	RoundData& round = table.m_rounds.back();
	Tile const discardedTile = round.DiscardDrawn();

	// TODO-MVP: calculate calls
	table.Transition(
		TableStates::BetweenTurns{table, std::nullopt, SeatSet{}, SeatSet{}, SeatSet{}},
		TableEvent{ TableEvent::Tag<TableEventType::Discard>(), discardedTile, round.CurrentTurn() }
	);
}

//------------------------------------------------------------------------------
Turn_User::Turn_User
(
	Table& i_table,
	Seat i_seat,
	bool i_canTsumo,
	bool i_canRiichi,
	bool i_canKan
)
	: BaseTurn{ i_table, i_seat }
	, m_canTsumo{ i_canTsumo }
	, m_canRiichi{ i_canRiichi }
	, m_canKan{ i_canKan }
{}

//------------------------------------------------------------------------------
void Turn_User::Tsumo
(
)	const
{
	// TODO-MVP
	Table& table = m_table.get();
	table.Transition( TableStates::GameOver{table}, std::string( "nyi" ) );
}

//------------------------------------------------------------------------------
void Turn_User::Discard
(
	Tile const& i_tile
)	const
{
	// TODO-MVP
	Table& table = m_table.get();
	table.Transition( TableStates::GameOver{table}, std::string( "nyi" ) );
}

//------------------------------------------------------------------------------
void Turn_User::Riichi
(
	Tile const& i_tile
)	const
{
	// TODO-MVP
	Table& table = m_table.get();
	table.Transition( TableStates::GameOver{table}, std::string( "nyi" ) );
}

//------------------------------------------------------------------------------
void Turn_User::Kan
(
	Tile const& i_tile
)	const
{
	// TODO-MVP
	Table& table = m_table.get();
	table.Transition( TableStates::GameOver{table}, std::string( "nyi" ) );
}

//------------------------------------------------------------------------------
BetweenTurns::BetweenTurns
(
	Table& i_table,
	Option<Seat> i_canChi,
	SeatSet i_canPon,
	SeatSet i_canKan,
	SeatSet i_canRon
)
	: Base{ i_table }
	, m_canChi{ std::move( i_canChi ) }
	, m_canPon{ std::move( i_canPon ) }
	, m_canKan{ std::move( i_canKan ) }
	, m_canRon{ std::move( i_canRon ) }
{}

//------------------------------------------------------------------------------
void BetweenTurns::UserPass
(
)	const
{
	Table& table = m_table.get();

	RoundData& round = table.m_rounds.back();

	// TODO-AI: Process AI call/wins

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
		// TODO-MVP: calculate tsumo/riichi/kan
		table.Transition(
			TableStates::Turn_User{table, round.CurrentTurn(), false, false, false},
			TableEvent{ TableEvent::Tag<TableEventType::Draw>(), drawnTile, round.CurrentTurn() }
		);
		break;
	}
	case PlayerType::AI:
	{
		table.Transition(
			TableStates::Turn_AI{table, round.CurrentTurn()},
			TableEvent{ TableEvent::Tag<TableEventType::Draw>(), drawnTile, round.CurrentTurn() }
		);
		break;
	}
	}
}

//------------------------------------------------------------------------------
void BetweenTurns::UserChi
(
	Seat i_user
)	const
{
	// TODO-MVP
	Table& table = m_table.get();
	table.Transition( TableStates::GameOver{table}, std::string( "nyi" ) );
}

//------------------------------------------------------------------------------
void BetweenTurns::UserPon
(
	Seat i_user
)	const
{
	// TODO-MVP
	Table& table = m_table.get();
	table.Transition( TableStates::GameOver{table}, std::string( "nyi" ) );
}

//------------------------------------------------------------------------------
void BetweenTurns::UserKan
(
	Seat i_user
)	const
{
	// TODO-MVP
	Table& table = m_table.get();
	table.Transition( TableStates::GameOver{table}, std::string( "nyi" ) );
}

//------------------------------------------------------------------------------
void BetweenTurns::UserRon
(
	SeatSet const& i_users
)	const
{
	// TODO-MVP
	Table& table = m_table.get();
	table.Transition( TableStates::GameOver{table}, std::string( "nyi" ) );
}

//------------------------------------------------------------------------------
RonAKanChance::RonAKanChance
(
	Table& i_table,
	SeatSet i_canRon
)
	: Base{ i_table }
	, m_canRon{ std::move( i_canRon ) }
{}

//------------------------------------------------------------------------------
void RonAKanChance::Pass
(
)	const
{
	// TODO-MVP
	Table& table = m_table.get();
	table.Transition( TableStates::GameOver{table}, std::string( "nyi" ) );
}

//------------------------------------------------------------------------------
void RonAKanChance::Ron
(
	SeatSet const& i_players
)	const
{
	Ensure( m_canRon.ContainsAllOf( i_players ), "Players tried to ron a kan when not allowed." );

	// TODO-MVP
	Table& table = m_table.get();
	table.Transition( TableStates::GameOver{table}, std::string( "nyi" ) );
}

}