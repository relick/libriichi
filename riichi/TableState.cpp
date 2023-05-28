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

	Player const& turnPlayer = round.GetPlayer( round.CurrentTurn(), table );

	switch ( turnPlayer.Type() )
	{
	case PlayerType::User:
	{
		Hand const& playerHand = round.GetHand( round.CurrentTurn() );

		bool canTsumo = false;
		Vector<Tile> riichiDiscards;

		bool const isRiichi = round.CalledRiichi( round.CurrentTurn() );

		if ( i_tileDraw.has_value() )
		{
			bool const allowedToRiichi = playerHand.Melds().empty()
				&& !isRiichi
				&& table.GetPoints( round.GetPlayerID( round.CurrentTurn() ) ) >= table.m_rules->RiichiBet();
			auto [ validWaits, validRiichiDiscards ] = table.m_rules->WaitsWithYaku(
				round,
				round.CurrentTurn(),
				playerHand,
				i_tileDraw.value(),
				allowedToRiichi
			);

			canTsumo = validWaits.contains( i_tileDraw.value().m_tile );
			riichiDiscards = std::move( validRiichiDiscards );
		}

		Vector<Hand::DrawKanResult> kanOptions = round.GetHand( round.CurrentTurn() ).DrawKanOptions( i_tileDraw.has_value() ? Option<Tile>( i_tileDraw.value().m_tile ) : Option<Tile>( std::nullopt ) );
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

	bool constexpr c_isTsumo = false;
	Pair<Points, Points> const pot = table.m_rules->PotPoints( round.HonbaSticks(), round.RiichiSticks(), c_isTsumo, i_winners.Size() );

	for ( Seat seat : i_winners )
	{
		Hand const hand = round.GetHand( seat );

		HandScore const score = table.m_rules->CalculateBasicPoints(
			round,
			seat,
			hand,
			i_tileDraw
		);

		round.AddWinner( seat, score );

		bool const isDealer = round.IsDealer( seat );
		Points const winnings = table.m_rules->PointsFromPlayerRon( score.first, isDealer );

		for ( size_t seatI = 0; seatI < table.m_players.size(); ++seatI )
		{
			Seat const standingsSeat = ( Seat )seatI;
			if ( standingsSeat == seat )
			{
				table.ModifyPoints( round.GetPlayerID( standingsSeat ), winnings + pot.second );
			}
			else if ( standingsSeat == round.CurrentTurn() )
			{
				table.ModifyPoints( round.GetPlayerID( standingsSeat ), -winnings - pot.first );
			}
		}
	}

	if ( table.m_rules->NoMoreRounds( table, round ) )
	{
		table.Transition(
			TableStates::GameOver{ table },
			TableEvents::Ron{ i_tileDraw.m_tile, i_winners, round.CurrentTurn() }
		);
	}
	else
	{
		table.Transition(
			TableStates::BetweenRounds{ table },
			TableEvents::Ron{ i_tileDraw.m_tile, i_winners, round.CurrentTurn() }
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
Hand const& BaseTurn::GetHand
(
)	const
{
	Table& table = m_table.get();
	return table.GetRound().GetHand( m_seat );
}

//------------------------------------------------------------------------------
Option<Tile> BaseTurn::GetDrawnTile
(
)	const
{
	Table& table = m_table.get();
	Option<TileDraw> const& drawnTile = table.GetRound().DrawnTile( m_seat );
	if ( drawnTile.has_value() )
	{
		return drawnTile.value().m_tile;
	}
	return std::nullopt;
}

//------------------------------------------------------------------------------
void BaseTurn::TransitionToBetweenTurns
(
	Tile const& i_discardedTile,
	TableEvent&& i_tableEvent
)	const
{
	Table& table = m_table.get();

	Round& round = table.m_rounds.back();

	// TODO-RULES: call options (particularly chi) should be controllable by rules
	Seat const nextPlayer = NextPlayer( round.CurrentTurn(), table.m_players.size() );
	Pair<Seat, Vector<Pair<Tile, Tile>>> canChi{nextPlayer, round.GetHand( nextPlayer ).ChiOptions( i_discardedTile )};
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

		if ( !isRiichi && round.GetHand( seat ).CanPon( i_discardedTile ) )
		{
			canPon.Insert( seat );
		}
		if ( !isRiichi && round.GetHand( seat ).CanCallKan( i_discardedTile ) )
		{
			canKan.Insert( seat );
		}

		bool constexpr c_allowedToRiichi = false;
		auto const [ validWaits, canRiichi ] = table.m_rules->WaitsWithYaku(
			round,
			seat,
			round.GetHand( seat ),
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
	Tile const discardedTile = round.Discard( std::nullopt );

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
	Vector<Tile> i_riichiDiscards,
	bool i_isRiichi,
	Vector<Hand::DrawKanResult> i_kanOptions
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

	Ensure( m_canTsumo, "This user cannot tsumo" );

	Round& round = table.m_rounds.back();

	Ensure( round.DrawnTile( round.CurrentTurn() ).has_value(), "Cannot tsumo without drawn tile" );
	TileDraw const& tileDraw = round.DrawnTile( round.CurrentTurn() ).value();
	Hand const hand = round.GetHand( round.CurrentTurn() );

	HandScore const score = table.m_rules->CalculateBasicPoints(
		round,
		round.CurrentTurn(),
		hand,
		tileDraw
	);

	Tile const winningTile = round.AddWinner( round.CurrentTurn(), score );

	bool const isDealer = round.IsDealer( round.CurrentTurn() );
	Pair<Points, Points> const winnings = table.m_rules->PointsFromEachPlayerTsumo( score.first, isDealer );

	bool constexpr c_isTsumo = true;
	Pair<Points, Points> const pot = table.m_rules->PotPoints( round.HonbaSticks(), round.RiichiSticks(), c_isTsumo, 1 );

	for ( size_t seatI = 0; seatI < table.m_players.size(); ++seatI )
	{
		Seat const seat = ( Seat )seatI;
		if ( seat == round.CurrentTurn() )
		{
			Points const points = static_cast< Points >(
				isDealer
				? ( ( table.m_players.size() - 1 ) * winnings.second )
				: ( winnings.first + ( table.m_players.size() - 2 ) * winnings.second )
				) + static_cast< Points >( pot.first * ( table.m_players.size() - 1 ) ) + pot.second;
			table.ModifyPoints( round.GetPlayerID( seat ), points );
		}
		else if ( round.IsDealer( seat ) )
		{
			table.ModifyPoints( round.GetPlayerID( seat ), -winnings.first - pot.first );
		}
		else
		{
			table.ModifyPoints( round.GetPlayerID( seat ), -winnings.second - pot.first );
		}
	}

	if ( table.m_rules->NoMoreRounds( table, round ) )
	{
		table.Transition(
			TableStates::GameOver( table ),
			TableEvents::Tsumo{ winningTile, round.CurrentTurn() }
		);
	}
	else
	{
		table.Transition(
			TableStates::BetweenRounds( table ),
			TableEvents::Tsumo{ winningTile, round.CurrentTurn() }
		);
	}
}

//------------------------------------------------------------------------------
void Turn_User::Discard
(
	Option<Tile> const& i_handTileToDiscard
)	const
{
	Table& table = m_table.get();

	Round& round = table.m_rounds.back();
	Tile const discardedTile = round.Discard( i_handTileToDiscard );

	TransitionToBetweenTurns(
		discardedTile,
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

	Ensure( CanRiichi(), "This user cannot riichi" );

	Round& round = table.m_rounds.back();

	Ensure(
		i_handTileToDiscard.has_value()
		? std::ranges::contains( m_riichiDiscards, i_handTileToDiscard.value() )
		: std::ranges::contains( m_riichiDiscards, round.DrawnTile( round.CurrentTurn() ).value().m_tile )
		, "Invalid tile to riichi with"
	);

	// Pay the bet
	table.ModifyPoints( round.GetPlayerID( round.CurrentTurn() ), -table.m_rules->RiichiBet() );

	// Make the discard
	Tile const discardedTile = round.Riichi( i_handTileToDiscard );

	TransitionToBetweenTurns(
		discardedTile,
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

	Round& round = table.m_rounds.back();
	Hand::KanResult const kanResult = round.HandKan( i_tile );

	SeatSet canRon;
	if ( kanResult.m_upgradedFromPon )
	{
		for ( size_t seatI = 0; seatI < table.m_players.size(); ++seatI )
		{
			Seat const seat = ( Seat )seatI;
			if ( seat == round.CurrentTurn() )
			{
				continue;
			}
			bool constexpr c_allowedToRiichi = false;
			auto const [ validWaits, canRiichi ] = table.m_rules->WaitsWithYaku(
				round,
				seat,
				round.GetHand( seat ),
				{ i_tile, TileDrawType::KanTheft },
				c_allowedToRiichi
			);
			if ( !validWaits.empty() && !round.Furiten( seat, validWaits ) )
			{
				canRon.Insert( seat );
			}
		}
	}

	table.Transition(
		TableStates::RonAKanChance{table, { i_tile, TileDrawType::KanTheft }, std::move( canRon )},
		TableEvent{ TableEvent::Tag<TableEventType::HandKan>(), i_tile, !kanResult.m_open }
	);
}

//------------------------------------------------------------------------------
BetweenTurns::BetweenTurns
(
	Table& i_table,
	TileDraw i_discardedTile,
	Pair<Seat, Vector<Pair<Tile, Tile>>> i_canChi,
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
			if ( !HandAssessment( round.GetHand( seat ), *table.m_rules ).Waits().empty() )
			{
				round.AddFinishedInTenpai( seat );
				inTenpai.Insert( seat );
			}
		}

		auto const [ pointsForEachPlayer, pointsFromEachPlayer ] = table.m_rules->PointsEachPlayerInTenpaiDraw( inTenpai.Size() );
		for ( size_t seatI = 0; seatI < table.m_players.size(); ++seatI )
		{
			Seat const seat = ( Seat )seatI;
			if ( inTenpai.Contains( seat ) )
			{
				table.ModifyPoints( round.GetPlayerID( seat ), pointsForEachPlayer );
			}
			else
			{
				table.ModifyPoints( round.GetPlayerID( seat ), -pointsFromEachPlayer );
			}
		}

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
	Pair<Tile, Tile> const& i_option
)	const
{
	Table& table = m_table.get();

	Round& round = table.m_rounds.back();
	Pair<Seat, Tile> const calledTile = round.Chi( i_user, i_option );

	TransitionToTurn(
		std::nullopt,
		TableEvent{ TableEvent::Tag<TableEventType::Call>(), TableEvents::CallType::Chi, calledTile.second, calledTile.first }
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
	Pair<Seat, Tile> const calledTile = round.Pon( i_user );

	TransitionToTurn(
		std::nullopt,
		TableEvent{ TableEvent::Tag<TableEventType::Call>(), TableEvents::CallType::Pon, calledTile.second, calledTile.first }
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
	Pair<Seat, Tile> const calledTile = round.DiscardKan( i_user );

	TransitionToTurn(
		std::nullopt,
		TableEvent{ TableEvent::Tag<TableEventType::Call>(), TableEvents::CallType::Kan, calledTile.second, calledTile.first }
	);
}

//------------------------------------------------------------------------------
void BetweenTurns::UserRon
(
	SeatSet const& i_users
)	const
{
	Ensure( m_canRon.ContainsAllOf( i_users ), "Players tried to ron when not allowed." );

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
	Ensure( m_canRon.ContainsAllOf( i_players ), "Players tried to ron a kan when not allowed." );

	HandleRon( i_players, m_kanTile );
}

}