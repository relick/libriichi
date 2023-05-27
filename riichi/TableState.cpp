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
	TileDraw const firstDrawnTile = round.DealHands();

	// TODO-DEBT: we should pull this sort of player assessment -> Turn state stuff to its own function
	Player const& turnPlayer = round.GetPlayer( round.CurrentTurn(), table );

	switch ( turnPlayer.Type() )
	{
	case PlayerType::User:
	{
		Hand const& playerHand = round.GetHand( round.CurrentTurn() );
		HandAssessment const assessment( playerHand, *table.m_rules );
		bool const canRiichi = !assessment.Waits().empty() && playerHand.Melds().empty();
		bool const canTsumo = assessment.Waits().contains( firstDrawnTile.m_tile );

		Vector<Hand::DrawKanResult> kanOptions = round.GetHand( round.CurrentTurn() ).DrawKanOptions( nullptr );
		table.Transition(
			TableStates::Turn_User{table, round.CurrentTurn(), canRiichi, canTsumo, std::move( kanOptions )},
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
Option<Tile> BaseTurn::GetDrawnTile
(
)	const
{
	Table& table = m_table.get();
	Option<TileDraw> const& drawnTile = table.GetRoundData().DrawnTile( m_seat );
	if ( drawnTile.has_value() )
	{
		return drawnTile.value().m_tile;
	}
	return std::nullopt;
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
	Tile const discardedTile = round.Discard( std::nullopt );

	// TODO-DEBT: can this be pulled into a function instead of repeating it?

	// TODO-RULES: call options (particularly chi) should be controllable by rules
	Seat const nextPlayer = NextPlayer( round.CurrentTurn(), table.m_players.size() );
	Pair<Seat, Vector<Pair<Tile, Tile>>> canChi{nextPlayer, round.GetHand( nextPlayer ).ChiOptions( discardedTile )};
	SeatSet canPon;
	SeatSet canKan;
	SeatSet canRon;

	for ( size_t seatI = 0; seatI < table.m_players.size(); ++seatI )
	{
		Seat const seat = ( Seat )seatI;
		if ( round.GetHand( seat ).CanPon( discardedTile ) )
		{
			canPon.Insert( seat );
		}
		if ( round.GetHand( seat ).CanCallKan( discardedTile ) )
		{
			canKan.Insert( seat );
		}

		Set<Tile> const waits = table.m_rules->WaitsWithYaku( round, seat, round.GetHand( seat ), { discardedTile, TileDrawType::DiscardDraw } );
		if ( !waits.empty() && !round.Furiten( seat, waits ) )
		{
			canRon.Insert( seat );
		}
	}

	table.Transition(
		TableStates::BetweenTurns{table, std::move( canChi ), std::move( canPon ), std::move( canKan ), std::move( canRon )},
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
	Vector<Hand::DrawKanResult> i_kanOptions
)
	: BaseTurn{ i_table, i_seat }
	, m_canTsumo{ i_canTsumo }
	, m_canRiichi{ i_canRiichi }
	, m_kanOptions{ std::move( i_kanOptions ) }
{}

//------------------------------------------------------------------------------
void Turn_User::Tsumo
(
)	const
{
	// TODO-MVP
	Table& table = m_table.get();
	table.Transition( TableStates::GameOver{table}, std::string( "nyi" ) );

	Ensure( m_canTsumo, "This user cannot tsumo" );

	RoundData& round = table.m_rounds.back();

	Ensure( round.DrawnTile( round.CurrentTurn() ).has_value(), "Cannot tsumo without drawn tile" );
	TileDraw const& tileDraw = round.DrawnTile( round.CurrentTurn() ).value();
	Hand const hand = round.GetHand( round.CurrentTurn() );

	//table.m_rules->CalculateBasicPoints();
}

//------------------------------------------------------------------------------
void Turn_User::Discard
(
	Option<Tile> const& i_handTileToDiscard
)	const
{
	Table& table = m_table.get();

	RoundData& round = table.m_rounds.back();
	Tile const discardedTile = round.Discard( i_handTileToDiscard );

	// TODO-DEBT: can this be pulled into a function instead of repeating it?

	// TODO-RULES: call options (particularly chi) should be controllable by rules
	Seat const nextPlayer = NextPlayer( round.CurrentTurn(), table.m_players.size() );
	Pair<Seat, Vector<Pair<Tile, Tile>>> canChi{nextPlayer, round.GetHand( nextPlayer ).ChiOptions( discardedTile )};
	SeatSet canPon;
	SeatSet canKan;
	SeatSet canRon;

	for ( size_t seatI = 0; seatI < table.m_players.size(); ++seatI )
	{
		Seat const seat = ( Seat )seatI;
		if ( round.GetHand( seat ).CanPon( discardedTile ) )
		{
			canPon.Insert( seat );
		}
		if ( round.GetHand( seat ).CanCallKan( discardedTile ) )
		{
			canKan.Insert( seat );
		}

		Set<Tile> const waits = table.m_rules->WaitsWithYaku( round, seat, round.GetHand( seat ), { discardedTile, TileDrawType::DiscardDraw } );
		if ( !waits.empty() && !round.Furiten( seat, waits ) )
		{
			canRon.Insert( seat );
		}
	}

	table.Transition(
		TableStates::BetweenTurns{table, std::move( canChi ), std::move( canPon ), std::move( canKan ), std::move( canRon )},
		TableEvent{ TableEvent::Tag<TableEventType::Discard>(), discardedTile, round.CurrentTurn() }
	);
}

//------------------------------------------------------------------------------
void Turn_User::Riichi
(
	Option<Tile> const& i_handTileToDiscard
)	const
{
	Table& table = m_table.get();

	Ensure( m_canRiichi, "This user cannot riichi" );

	RoundData& round = table.m_rounds.back();
	Tile const discardedTile = round.Riichi( i_handTileToDiscard );

	// TODO-DEBT: can this be pulled into a function instead of repeating it?

	// TODO-RULES: call options (particularly chi) should be controllable by rules
	Seat const nextPlayer = NextPlayer( round.CurrentTurn(), table.m_players.size() );
	Pair<Seat, Vector<Pair<Tile, Tile>>> canChi{nextPlayer, round.GetHand( nextPlayer ).ChiOptions( discardedTile )};
	SeatSet canPon;
	SeatSet canKan;
	SeatSet canRon;

	for ( size_t seatI = 0; seatI < table.m_players.size(); ++seatI )
	{
		Seat const seat = ( Seat )seatI;
		if ( round.GetHand( seat ).CanPon( discardedTile ) )
		{
			canPon.Insert( seat );
		}
		if ( round.GetHand( seat ).CanCallKan( discardedTile ) )
		{
			canKan.Insert( seat );
		}

		Set<Tile> const waits = table.m_rules->WaitsWithYaku( round, seat, round.GetHand( seat ), { discardedTile, TileDrawType::DiscardDraw } );
		if ( !waits.empty() && !round.Furiten( seat, waits ) )
		{
			canRon.Insert( seat );
		}
	}

	table.Transition(
		TableStates::BetweenTurns{table, std::move( canChi ), std::move( canPon ), std::move( canKan ), std::move( canRon )},
		TableEvent{ TableEvent::Tag<TableEventType::Riichi>(), discardedTile, round.CurrentTurn() }
	);
}

//------------------------------------------------------------------------------
void Turn_User::Kan
(
	Tile const& i_tile
)	const
{
	Table& table = m_table.get();

	Ensure( std::ranges::any_of( m_kanOptions, [ & ]( Hand::DrawKanResult const& i_option ) -> bool
		{
			return i_option.kanTile == i_tile;
		}), "This user cannot kan with provided tile");

	RoundData& round = table.m_rounds.back();
	Hand::KanResult const kanResult = round.HandKan( i_tile );

	SeatSet canRon;
	if ( kanResult.m_upgradedFromPon )
	{
		bool anyCanRon = false;
		for ( size_t seatI = 0; seatI < table.m_players.size(); ++seatI )
		{
			Seat const seat = ( Seat )seatI;
			Set<Tile> const waits = table.m_rules->WaitsWithYaku( round, seat, round.GetHand( seat ), { i_tile, TileDrawType::KanTheft } );
			if ( !waits.empty() && !round.Furiten( seat, waits ) )
			{
				canRon.Insert( seat );
				anyCanRon = true;
			}
		}
	}

	table.Transition(
		TableStates::RonAKanChance{table, std::move( canRon )},
		TableEvent{ TableEvent::Tag<TableEventType::HandKan>(), i_tile, !kanResult.m_open }
	);
}

//------------------------------------------------------------------------------
BetweenTurns::BetweenTurns
(
	Table& i_table,
	Pair<Seat, Vector<Pair<Tile, Tile>>> i_canChi,
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

	TileDraw const drawnTile = round.PassCalls( m_canRon );

	Player const& turnPlayer = round.GetPlayer( round.CurrentTurn(), table );

	switch ( turnPlayer.Type() )
	{
	case PlayerType::User:
	{
		Hand const& playerHand = round.GetHand( round.CurrentTurn() );
		HandAssessment const assessment( playerHand, *table.m_rules );
		bool const canRiichi = !assessment.Waits().empty() && playerHand.Melds().empty();
		bool const canTsumo = assessment.Waits().contains( drawnTile.m_tile );

		Vector<Hand::DrawKanResult> kanOptions = round.GetHand( round.CurrentTurn() ).DrawKanOptions( nullptr );
		table.Transition(
			TableStates::Turn_User{table, round.CurrentTurn(), canRiichi, canTsumo, std::move( kanOptions )},
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
	Seat i_user,
	Pair<Tile, Tile> const& i_option
)	const
{
	Table& table = m_table.get();

	RoundData& round = table.m_rounds.back();
	Pair<Seat, Tile> const calledTile = round.Chi( i_user, i_option );

	Player const& turnPlayer = round.GetPlayer( round.CurrentTurn(), table );

	switch ( turnPlayer.Type() )
	{
	case PlayerType::User:
	{
		bool constexpr c_canTsumo = false;
		bool constexpr c_canRiichi = false;
		Vector<Hand::DrawKanResult> kanOptions = round.GetHand( round.CurrentTurn() ).DrawKanOptions( nullptr );

		table.Transition(
			TableStates::Turn_User{table, round.CurrentTurn(), c_canTsumo, c_canRiichi, std::move( kanOptions )},
			TableEvent{ TableEvent::Tag<TableEventType::Call>(), TableEvents::CallType::Chi, calledTile.second, calledTile.first }
		);
		break;
	}
	case PlayerType::AI:
	{
		table.Transition(
			TableStates::Turn_AI{table, round.CurrentTurn()},
			TableEvent{ TableEvent::Tag<TableEventType::Call>(), TableEvents::CallType::Chi, calledTile.second, calledTile.first }
		);
		break;
	}
	}
}

//------------------------------------------------------------------------------
void BetweenTurns::UserPon
(
	Seat i_user
)	const
{
	Table& table = m_table.get();

	RoundData& round = table.m_rounds.back();
	Pair<Seat, Tile> const calledTile = round.Pon( i_user );

	Player const& turnPlayer = round.GetPlayer( round.CurrentTurn(), table );

	switch ( turnPlayer.Type() )
	{
	case PlayerType::User:
	{
		bool constexpr c_canTsumo = false;
		bool constexpr c_canRiichi = false;
		Vector<Hand::DrawKanResult> kanOptions = round.GetHand( round.CurrentTurn() ).DrawKanOptions( nullptr );

		table.Transition(
			TableStates::Turn_User{table, round.CurrentTurn(), c_canTsumo, c_canRiichi, std::move( kanOptions )},
			TableEvent{ TableEvent::Tag<TableEventType::Call>(), TableEvents::CallType::Pon, calledTile.second, calledTile.first }
		);
		break;
	}
	case PlayerType::AI:
	{
		table.Transition(
			TableStates::Turn_AI{table, round.CurrentTurn()},
			TableEvent{ TableEvent::Tag<TableEventType::Call>(), TableEvents::CallType::Pon, calledTile.second, calledTile.first }
		);
		break;
	}
	}
}

//------------------------------------------------------------------------------
void BetweenTurns::UserKan
(
	Seat i_user
)	const
{
	Table& table = m_table.get();

	RoundData& round = table.m_rounds.back();
	Pair<Seat, Tile> const calledTile = round.DiscardKan( i_user );

	Player const& turnPlayer = round.GetPlayer( round.CurrentTurn(), table );

	switch ( turnPlayer.Type() )
	{
	case PlayerType::User:
	{
		bool constexpr c_canTsumo = false;
		bool constexpr c_canRiichi = false;
		Vector<Hand::DrawKanResult> kanOptions = round.GetHand( round.CurrentTurn() ).DrawKanOptions( nullptr );

		table.Transition(
			TableStates::Turn_User{table, round.CurrentTurn(), c_canTsumo, c_canRiichi, std::move( kanOptions )},
			TableEvent{ TableEvent::Tag<TableEventType::Call>(), TableEvents::CallType::Kan, calledTile.second, calledTile.first }
		);
		break;
	}
	case PlayerType::AI:
	{
		table.Transition(
			TableStates::Turn_AI{table, round.CurrentTurn()},
			TableEvent{ TableEvent::Tag<TableEventType::Call>(), TableEvents::CallType::Kan, calledTile.second, calledTile.first }
		);
		break;
	}
	}
}

//------------------------------------------------------------------------------
void BetweenTurns::UserRon
(
	SeatSet const& i_users
)	const
{
	Ensure( m_canRon.ContainsAllOf( i_users ), "Players tried to ron when not allowed." );

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
	Table& table = m_table.get();

	RoundData& round = table.m_rounds.back();
	TileDraw const deadWallDraw = round.HandKanRonPass();

	// TODO-DEBT: we should pull this sort of player assessment -> Turn state stuff to its own function
	Player const& turnPlayer = round.GetPlayer( round.CurrentTurn(), table );

	switch ( turnPlayer.Type() )
	{
	case PlayerType::User:
	{
		Hand const& playerHand = round.GetHand( round.CurrentTurn() );
		HandAssessment const assessment( playerHand, *table.m_rules );
		bool const canRiichi = !assessment.Waits().empty() && playerHand.Melds().empty();
		bool const canTsumo = assessment.Waits().contains( deadWallDraw.m_tile );

		Vector<Hand::DrawKanResult> kanOptions = round.GetHand( round.CurrentTurn() ).DrawKanOptions( nullptr );
		table.Transition(
			TableStates::Turn_User{table, round.CurrentTurn(), canRiichi, canTsumo, std::move( kanOptions )},
			TableEvent{ TableEvent::Tag<TableEventType::Draw>(), deadWallDraw, round.CurrentTurn() }
		);
		break;
	}
	case PlayerType::AI:
	{
		table.Transition(
			TableStates::Turn_AI{table, round.CurrentTurn()},
			TableEvent{ TableEvent::Tag<TableEventType::Draw>(), deadWallDraw, round.CurrentTurn() }
		);
		break;
	}
	}
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