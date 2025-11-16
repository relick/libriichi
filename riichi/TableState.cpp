#include "TableState.hpp"

#include "Rules.hpp"
#include "Table.hpp"

namespace Riichi::TableStates
{

//------------------------------------------------------------------------------
void BetweenTurnsBase::TransitionToTurn
(
	Option<TileDraw> const& i_tileDraw,
	TableEvent&& i_tableEvent
)	const
{
	Table& table = m_table.get();

	Round& round = table.m_rounds.back();

	// Any pending riichi bets should now be applied, as it's the start of the next turn
	for ( Seat player : round.Seats() )
	{
		if ( round.WaitingToPayRiichiBet( player ) )
		{
			// Pay the bet
			TablePayments const riichiBetPayments = table.m_rules->RiichiBetPayments( player );
			round.ApplyPayments( riichiBetPayments, table );
			round.RiichiBetPaid( player ); // adds the stick to the round pot
		}
	}

	Player const& turnPlayer = round.GetPlayer( round.CurrentTurn(), table );

	switch ( turnPlayer.Type() )
	{
	case PlayerType::User:
	{
		Hand const& playerHand = round.CurrentHand( round.CurrentTurn() );

		bool canTsumo = false;
		Vector<TileInstance> riichiDiscards;

		bool const isRiichi = round.CalledRiichi( round.CurrentTurn() );

		if ( i_tileDraw.has_value() )
		{
			bool const allowedToRiichi = playerHand.Melds().empty()
				&& !isRiichi
				&& table.m_rules->HasPermissionToRiichi( round.CurrentTurn(), table.GetPoints( round.GetPlayerID( round.CurrentTurn() ) ) );
			auto [ validWaits, validRiichiDiscards ] = table.m_rules->WaitsWithYaku(
				round,
				round.CurrentTurn(),
				playerHand,
				i_tileDraw.value(),
				allowedToRiichi
			);

			canTsumo = validWaits.contains( i_tileDraw.value().m_tile.Tile() );
			riichiDiscards = std::move( validRiichiDiscards );
		}

		Vector<HandKanOption> kanOptions = round.CurrentHand( round.CurrentTurn() ).KanOptions( i_tileDraw ? Option<TileInstance>( i_tileDraw->m_tile ) : Option<TileInstance>() );
		table.Transition(
			TableStates::Turn_User{ table, round.CurrentTurn(), canTsumo, std::move( riichiDiscards ), isRiichi, std::move( kanOptions ) },
			std::move( i_tableEvent )
		);
		break;
	}
	case PlayerType::AI:
	{
		table.Transition(
			TableStates::Turn_AI{ table, round.CurrentTurn() },
			std::move( i_tableEvent )
		);
		break;
	}
	}
}

//------------------------------------------------------------------------------
void BetweenTurnsBase::HandleRon
(
	SeatSet const& i_winners,
	TileDraw const& i_tileDraw
)	const
{
	Table& table = m_table.get();

	Round& round = table.m_rounds.back();

	// TODO-AI: assess whether any AI should join in ron
	// TODO-RULES: allow/disallow multiple ron

	Seat const loser = round.CurrentTurn();

	TablePayments const potPayments =
		table.m_rules->HonbaPotPayments( round.HonbaSticks(), i_winners, loser )
		+ table.m_rules->RiichiBetPotPayments( round.RiichiSticks(), i_winners, loser );

	TablePayments totalPayments = potPayments;
	for ( Seat winner : i_winners )
	{
		Hand const hand = round.CurrentHand( winner );

		HandScore const handScore = table.m_rules->CalculateBasicPoints(
			round,
			winner,
			hand,
			i_tileDraw
		);

		TablePayments const ronPayments = table.m_rules->RonPayments( handScore, winner, loser );
		totalPayments += ronPayments;

		FinalScore const finalScore{ ronPayments.m_pointsPerSeat[ winner ], potPayments.m_pointsPerSeat[ winner ] };
		round.AddWinner( winner, handScore, finalScore );
	}

	// Modify the points
	round.ApplyEndOfRoundPayments( totalPayments, table );

	if ( table.m_rules->NoMoreRounds( table, round ) )
	{
		table.Transition(
			TableStates::GameOver{ table },
			TableEvents::Ron{ i_tileDraw.m_tile, i_winners, loser }
		);
	}
	else
	{
		table.Transition(
			TableStates::BetweenRounds{ table },
			TableEvents::Ron{ i_tileDraw.m_tile, i_winners, loser }
		);
	}
}

//------------------------------------------------------------------------------
void Setup::StartGame
(
)	const
{
	Table& table = m_table.get();

	if ( table.m_players.size() < table.m_rules->GetPlayerCount() )
	{
		riError( "Cannot start game with fewer players than required!" );
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
			table.m_playerIDs,
			*table.m_rules,
			table.m_shuffleRNG
		);
	}
	else
	{
		// Follow from last round
		Round const& previousRound = table.m_rounds.back();
		table.m_rounds.emplace_back(
			table,
			previousRound,
			*table.m_rules,
			table.m_shuffleRNG
		);
	}

	Round& round = table.m_rounds.back();

	TileDraw const firstDrawnTile = round.DealHands();

	TransitionToTurn(
		firstDrawnTile,
		TableEvent{ TableEvent::Tag<TableEventType::DealerDraw>(), firstDrawnTile, round.CurrentTurn() }
	);
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
Hand const& BaseTurn::GetCurrentHand
(
)	const
{
	Table& table = m_table.get();
	return table.GetRound().CurrentHand( m_seat );
}

//------------------------------------------------------------------------------
Option<TileInstance> BaseTurn::GetCurrentTileDraw
(
)	const
{
	Table& table = m_table.get();
	Option<TileDraw> const& drawnTile = table.GetRound().CurrentTileDraw( m_seat );
	if ( drawnTile.has_value() )
	{
		return drawnTile.value().m_tile;
	}
	return std::nullopt;
}

//------------------------------------------------------------------------------
void BaseTurn::TransitionToBetweenTurns
(
	TileInstance const& i_discardedTile,
	TableEvent&& i_tableEvent
)	const
{
	Table& table = m_table.get();

	Round& round = table.m_rounds.back();

	// TODO-RULES: call options (particularly chi) should be controllable by rules
	Seat const nextPlayer = NextPlayer( round.CurrentTurn(), table.m_players.size() );
	Pair<Seat, Vector<Pair<TileInstance, TileInstance>>> canChi{nextPlayer, round.CurrentHand( nextPlayer ).ChiOptions( i_discardedTile.Tile() )};
	if ( round.CalledRiichi( nextPlayer ) )
	{
		canChi.second.clear();
	}
	SeatSet canPon;
	SeatSet canKan;
	SeatSet canRon;

	TileDraw const discardedTileAsDraw{ i_discardedTile, TileDrawType::DiscardDraw };
	for ( size_t seatI = 0; seatI < table.m_players.size(); ++seatI )
	{
		Seat const seat = ( Seat )seatI;
		if ( seat == round.CurrentTurn() )
		{
			continue;
		}

		bool const isRiichi = round.CalledRiichi( seat );

		if ( !isRiichi && round.CurrentHand( seat ).CanPon( i_discardedTile.Tile() ) )
		{
			canPon.Insert( seat );
		}
		if ( !isRiichi && round.CurrentHand( seat ).CanCallKan( i_discardedTile.Tile() ) )
		{
			canKan.Insert( seat );
		}

		bool constexpr c_allowedToRiichi = false;
		auto const [ validWaits, canRiichi ] = table.m_rules->WaitsWithYaku(
			round,
			seat,
			round.CurrentHand( seat ),
			discardedTileAsDraw,
			c_allowedToRiichi
		);
		if ( !validWaits.empty() && !round.Furiten( seat, validWaits ) )
		{
			canRon.Insert( seat );
		}
	}

	table.Transition(
		TableStates::BetweenTurns{table, discardedTileAsDraw, std::move( canChi ), std::move( canPon ), std::move( canKan ), std::move( canRon )},
		std::move( i_tableEvent )
	);
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
	Round& round = table.m_rounds.back();
	TileInstance const discardedTile = round.Discard( std::nullopt );

	TransitionToBetweenTurns(
		discardedTile,
		TableEvent{ TableEvent::Tag<TableEventType::Discard>(), discardedTile, round.CurrentTurn() }
	);
}

//------------------------------------------------------------------------------
Turn_User::Turn_User
(
	Table& i_table,
	Seat i_seat,
	bool i_canTsumo,
	Vector<TileInstance> i_riichiDiscards,
	bool i_isRiichi,
	Vector<HandKanOption> i_kanOptions
)
	: BaseTurn{ i_table, i_seat }
	, m_canTsumo{ i_canTsumo }
	, m_riichiDiscards( std::move( i_riichiDiscards ) )
	, m_isRiichi{ i_isRiichi }
	, m_kanOptions{ std::move( i_kanOptions ) }
{}

//------------------------------------------------------------------------------
void Turn_User::Tsumo
(
)	const
{
	Table& table = m_table.get();

	riEnsure( m_canTsumo, "This user cannot tsumo" );

	Round& round = table.m_rounds.back();

	Seat const winner = round.CurrentTurn();

	TablePayments const potPayments =
		table.m_rules->HonbaPotPayments( round.HonbaSticks(), SeatSet{ winner }, std::nullopt )
		+ table.m_rules->RiichiBetPotPayments( round.RiichiSticks(), SeatSet{ winner }, std::nullopt );

	riEnsure( round.CurrentTileDraw( winner ).has_value(), "Cannot tsumo without drawn tile" );
	TileDraw const& tileDraw = round.CurrentTileDraw( winner ).value();
	Hand const hand = round.CurrentHand( winner );

	HandScore const handScore = table.m_rules->CalculateBasicPoints(
		round,
		winner,
		hand,
		tileDraw
	);

	TablePayments const tsumoPayments = table.m_rules->TsumoPayments( handScore, winner );

	FinalScore const finalScore{ tsumoPayments.m_pointsPerSeat[ winner ], potPayments.m_pointsPerSeat[ winner ] };
	TileInstance const winningTile = round.AddWinner( winner, handScore, finalScore );

	// Modify the points
	round.ApplyEndOfRoundPayments( potPayments + tsumoPayments, table );

	if ( table.m_rules->NoMoreRounds( table, round ) )
	{
		table.Transition(
			TableStates::GameOver( table ),
			TableEvents::Tsumo{ winningTile, winner }
		);
	}
	else
	{
		table.Transition(
			TableStates::BetweenRounds( table ),
			TableEvents::Tsumo{ winningTile, winner }
		);
	}
}

//------------------------------------------------------------------------------
void Turn_User::Discard
(
	Option<TileInstance> const& i_handTileToDiscard
)	const
{
	Table& table = m_table.get();

	Round& round = table.m_rounds.back();
	TileInstance const discardedTile = round.Discard( i_handTileToDiscard );

	TransitionToBetweenTurns(
		discardedTile,
		TableEvent{ TableEvent::Tag<TableEventType::Discard>(), discardedTile, round.CurrentTurn() }
	);
}

//------------------------------------------------------------------------------
void Turn_User::Riichi
(
	Option<TileInstance> const& i_handTileToDiscard
)	const
{
	Table& table = m_table.get();

	riEnsure( CanRiichi(), "This user cannot riichi" );

	Round& round = table.m_rounds.back();

	riEnsure(
		i_handTileToDiscard.has_value()
		? std::ranges::any_of( m_riichiDiscards, EqualsTileInstanceID{ i_handTileToDiscard.value() } )
		: std::ranges::any_of( m_riichiDiscards, EqualsTileInstanceID{ round.CurrentTileDraw( round.CurrentTurn() ).value().m_tile } )
		, "Invalid tile to riichi with"
	);

	// Make the discard
	// Note: paying the bet is deferred until the end of BetweenTurns, in case a ron occurs. The bet is not paid in that case.
	TileInstance const discardedTile = round.Riichi( i_handTileToDiscard );

	TransitionToBetweenTurns(
		discardedTile,
		TableEvent{ TableEvent::Tag<TableEventType::Riichi>(), discardedTile, round.CurrentTurn() }
	);
}

//------------------------------------------------------------------------------
void Turn_User::Kan
(
	HandKanOption const& i_kanOption
)	const
{
	Table& table = m_table.get();

	riEnsure( std::ranges::contains( m_kanOptions, i_kanOption ), "This user cannot kan with provided option" );

	Round& round = table.m_rounds.back();
	Seat const player = round.CurrentTurn();
	Option<TileDraw> const drawnTile = round.CurrentTileDraw( player );

	round.HandKan( i_kanOption );

	TileInstance const kanTile = i_kanOption.Tiles().front();
	TileDraw const kanTileTheft{ kanTile, i_kanOption.m_closed ? TileDrawType::ClosedKanTheft : TileDrawType::UpgradedKanTheft };

	SeatSet canRon;
	for ( size_t seatI = 0; seatI < table.m_players.size(); ++seatI )
	{
		Seat const seat = ( Seat )seatI;
		if ( seat == player )
		{
			continue;
		}
		bool constexpr c_allowedToRiichi = false;
		auto const [ validWaits, canRiichi ] = table.m_rules->WaitsWithYaku(
			round,
			seat,
			round.CurrentHand( seat ),
			kanTileTheft,
			c_allowedToRiichi
		);
		if ( !validWaits.empty() && !round.Furiten( seat, validWaits ) )
		{
			canRon.Insert( seat );
		}
	}

	table.Transition(
		TableStates::RonAKanChance{ table, kanTileTheft, std::move( canRon ) },
		i_kanOption.m_closed
		? TableEvent{ TableEvent::Tag<TableEventType::ClosedKan>(), kanTile.Tile().Kind() }
		: TableEvent{ TableEvent::Tag<TableEventType::UpgradedKan>(), kanTile }
	);
}

//------------------------------------------------------------------------------
BetweenTurns::BetweenTurns
(
	Table& i_table,
	TileDraw i_discardedTile,
	Pair<Seat, Vector<Pair<TileInstance, TileInstance>>> i_canChi,
	SeatSet i_canPon,
	SeatSet i_canKan,
	SeatSet i_canRon
)
	: BetweenTurnsBase{ i_table }
	, m_discardedTile{ i_discardedTile }
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

	Round& round = table.m_rounds.back();

	// TODO-AI: Process AI call/wins

	if ( round.WallTilesRemaining() == 0u )
	{
		// Ryuukyoku/draw has occurred
		// TODO-RULES: nagashi mangan

		SeatSet inTenpai;
		for ( size_t seatI = 0; seatI < table.m_players.size(); ++seatI )
		{
			Seat const seat = ( Seat )seatI;
			if ( !HandAssessment( round.CurrentHand( seat ), *table.m_rules ).Waits().empty() )
			{
				round.AddFinishedInTenpai( seat );
				inTenpai.Insert( seat );
			}
		}

		TablePayments const exhaustiveDrawPayments = table.m_rules->ExhaustiveDrawPayments( inTenpai );
		round.ApplyEndOfRoundPayments( exhaustiveDrawPayments, table );

		if ( table.m_rules->NoMoreRounds( table, round ) )
		{
			table.Transition(
				TableStates::GameOver{ table },
				TableEvents::WallDepleted{ std::move( inTenpai ) }
			);
		}
		else
		{
			table.Transition(
				TableStates::BetweenRounds{ table },
				TableEvents::WallDepleted{ std::move( inTenpai ) }
			);
		}
		return;
	}

	// Round continues, advance to next player and draw from wall
	TileDraw const drawnTile = round.PassCalls( m_canRon );

	TransitionToTurn(
		drawnTile,
		TableEvent{ TableEvent::Tag<TableEventType::Draw>(), drawnTile, round.CurrentTurn() }
	);
}

//------------------------------------------------------------------------------
void BetweenTurns::UserChi
(
	Seat i_user,
	Pair<TileInstance, TileInstance> const& i_option
)	const
{
	Table& table = m_table.get();

	Round& round = table.m_rounds.back();
	Meld::CalledTile const calledTile = round.Chi( i_user, i_option );

	TransitionToTurn(
		std::nullopt,
		TableEvent{ TableEvent::Tag<TableEventType::Call>(), TableEvents::CallType::Chi, calledTile }
	);
}

//------------------------------------------------------------------------------
void BetweenTurns::UserPon
(
	Seat i_user
)	const
{
	Table& table = m_table.get();

	Round& round = table.m_rounds.back();
	Meld::CalledTile const calledTile = round.Pon( i_user );

	TransitionToTurn(
		std::nullopt,
		TableEvent{ TableEvent::Tag<TableEventType::Call>(), TableEvents::CallType::Pon, calledTile }
	);
}

//------------------------------------------------------------------------------
void BetweenTurns::UserKan
(
	Seat i_user
)	const
{
	Table& table = m_table.get();

	Round& round = table.m_rounds.back();
	auto const [ deadWallDraw, calledTile ] = round.DiscardKan( i_user );

	TransitionToTurn(
		deadWallDraw,
		TableEvent{ TableEvent::Tag<TableEventType::Call>(), TableEvents::CallType::Kan, calledTile }
	);
}

//------------------------------------------------------------------------------
void BetweenTurns::UserRon
(
	SeatSet const& i_users
)	const
{
	riEnsure( m_canRon.ContainsAllOf( i_users ), "Players tried to ron when not allowed." );

	HandleRon( i_users, m_discardedTile );
}

//------------------------------------------------------------------------------
RonAKanChance::RonAKanChance
(
	Table& i_table,
	TileDraw i_kanTile,
	SeatSet i_canRon
)
	: BetweenTurnsBase{ i_table }
	, m_kanTile{ i_kanTile }
	, m_canRon{ std::move( i_canRon ) }
{}

//------------------------------------------------------------------------------
void RonAKanChance::Pass
(
)	const
{
	Table& table = m_table.get();

	Round& round = table.m_rounds.back();
	TileDraw const deadWallDraw = round.HandKanRonPass();

	TransitionToTurn(
		deadWallDraw,
		TableEvent{ TableEvent::Tag<TableEventType::Draw>(), deadWallDraw, round.CurrentTurn() }
	);
}

//------------------------------------------------------------------------------
void RonAKanChance::Ron
(
	SeatSet const& i_players
)	const
{
	riEnsure( m_canRon.ContainsAllOf( i_players ), "Players tried to ron a kan when not allowed." );

	HandleRon( i_players, m_kanTile );
}

}