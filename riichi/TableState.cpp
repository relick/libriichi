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

		bool const allowedToRiichi = playerHand.Melds().empty() && !round.CalledRiichi( round.CurrentTurn() );
		auto const [ validWaits, canRiichi ] = table.m_rules->WaitsWithYaku(
			round,
			round.CurrentTurn(),
			playerHand,
			firstDrawnTile,
			allowedToRiichi
		);

		bool const canTsumo = validWaits.contains( firstDrawnTile.m_tile );

		Vector<Hand::DrawKanResult> kanOptions = round.GetHand( round.CurrentTurn() ).DrawKanOptions( &firstDrawnTile.m_tile );
		table.Transition(
			TableStates::Turn_User{table, round.CurrentTurn(), canTsumo, canRiichi, std::move( kanOptions )},
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

	TileDraw const discardedTileAsDraw{ discardedTile, TileDrawType::DiscardDraw };
	for ( size_t seatI = 0; seatI < table.m_players.size(); ++seatI )
	{
		Seat const seat = ( Seat )seatI;
		if ( seat == round.CurrentTurn() )
		{
			continue;
		}
		if ( round.GetHand( seat ).CanPon( discardedTile ) )
		{
			canPon.Insert( seat );
		}
		if ( round.GetHand( seat ).CanCallKan( discardedTile ) )
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
	Table& table = m_table.get();

	Ensure( m_canTsumo, "This user cannot tsumo" );

	RoundData& round = table.m_rounds.back();

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

	for ( size_t seatI = 0; seatI < table.m_players.size(); ++seatI )
	{
		Seat const seat = ( Seat )seatI;
		if ( seat == round.CurrentTurn() )
		{
			table.m_standings.m_points[ round.GetPlayerID( seat, table ) ] += static_cast<Points>(
				isDealer
				? ( ( table.m_players.size() - 1 ) * winnings.second )
				: ( winnings.first + ( table.m_players.size() - 2 ) * winnings.second )
				);
		}
		else if ( round.IsDealer( seat ) )
		{
			table.m_standings.m_points[ round.GetPlayerID( seat, table ) ] -= winnings.first;
		}
		else
		{
			table.m_standings.m_points[ round.GetPlayerID( seat, table ) ] -= winnings.second;
		}
	}

	// TODO-RULES: allow for negative points play
	if ( std::ranges::any_of( table.m_standings.m_points, []( Points i_points ) { return i_points < 0; } )
		|| round.NoMoreRounds( *table.m_rules ) )
	{
		table.Transition(
			TableStates::GameOver( table ),
			TableEvents::Tsumo{ winningTile, round.CurrentTurn() }
		);
	}

	table.Transition(
		TableStates::BetweenRounds( table ),
		TableEvents::Tsumo{ winningTile, round.CurrentTurn() }
	);
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

	TileDraw const discardedTileAsDraw{ discardedTile, TileDrawType::DiscardDraw };
	for ( size_t seatI = 0; seatI < table.m_players.size(); ++seatI )
	{
		Seat const seat = ( Seat )seatI;
		if ( seat == round.CurrentTurn() )
		{
			continue;
		}
		if ( round.GetHand( seat ).CanPon( discardedTile ) )
		{
			canPon.Insert( seat );
		}
		if ( round.GetHand( seat ).CanCallKan( discardedTile ) )
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

	TileDraw const discardedTileAsDraw{ discardedTile, TileDrawType::DiscardDraw };
	for ( size_t seatI = 0; seatI < table.m_players.size(); ++seatI )
	{
		Seat const seat = ( Seat )seatI;
		if ( seat == round.CurrentTurn() )
		{
			continue;
		}
		if ( round.GetHand( seat ).CanPon( discardedTile ) )
		{
			canPon.Insert( seat );
		}
		if ( round.GetHand( seat ).CanCallKan( discardedTile ) )
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
	: Base{ i_table }
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

	RoundData& round = table.m_rounds.back();

	// TODO-AI: Process AI call/wins

	if ( round.WallTilesRemaining() == 0u )
	{
		// TODO-MVP: nagashi mangan

		size_t totalInTenpai = 0;
		for ( size_t seatI = 0; seatI < table.m_players.size(); ++seatI )
		{
			Seat const seat = ( Seat )seatI;
			if ( !HandAssessment( round.GetHand( seat ), *table.m_rules ).Waits().empty() )
			{
				round.AddFinishedInTenpai( seat );
				++totalInTenpai;
			}
		}

		auto const [ pointsForEachPlayer, pointsFromEachPlayer ] = table.m_rules->PointsEachPlayerInTenpaiDraw( totalInTenpai );
		for ( size_t seatI = 0; seatI < table.m_players.size(); ++seatI )
		{
			Seat const seat = ( Seat )seatI;
			if ( round.FinishedInTenpai( seat ) )
			{
				table.m_standings.m_points[ round.GetPlayerID( seat, table ) ] += pointsForEachPlayer;
			}
			else
			{
				table.m_standings.m_points[ round.GetPlayerID( seat, table ) ] -= pointsForEachPlayer;
			}
		}

		// TODO-RULES: allow for negative points play
		if ( std::ranges::any_of( table.m_standings.m_points, []( Points i_points ) { return i_points < 0; } )
			|| round.NoMoreRounds( *table.m_rules ) )
		{
			table.Transition(
				TableStates::GameOver{ table },
				TableEvents::WallDepleted{}
			);
		}

		table.Transition(
			TableStates::BetweenRounds{ table },
			TableEvents::WallDepleted{}
		);
		return;
	}

	TileDraw const drawnTile = round.PassCalls( m_canRon );

	Player const& turnPlayer = round.GetPlayer( round.CurrentTurn(), table );

	switch ( turnPlayer.Type() )
	{
	case PlayerType::User:
	{
		Hand const& playerHand = round.GetHand( round.CurrentTurn() );

		bool const allowedToRiichi = playerHand.Melds().empty() && !round.CalledRiichi( round.CurrentTurn() );
		auto const [ validWaits, canRiichi ] = table.m_rules->WaitsWithYaku(
			round,
			round.CurrentTurn(),
			playerHand,
			drawnTile,
			allowedToRiichi
		);

		bool const canTsumo = validWaits.contains( drawnTile.m_tile );

		Vector<Hand::DrawKanResult> kanOptions = round.GetHand( round.CurrentTurn() ).DrawKanOptions( &drawnTile.m_tile );
		table.Transition(
			TableStates::Turn_User{table, round.CurrentTurn(), canTsumo, canRiichi, std::move( kanOptions )},
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
	Table& table = m_table.get();

	Ensure( m_canRon.ContainsAllOf( i_users ), "Players tried to ron when not allowed." );

	RoundData& round = table.m_rounds.back();

	// TODO-AI: assess whether any AI should join in ron
	// TODO-RULES: allow/disallow multiple ron

	for ( Seat seat : i_users )
	{
		Hand const hand = round.GetHand( seat );

		HandScore const score = table.m_rules->CalculateBasicPoints(
			round,
			seat,
			hand,
			m_discardedTile
		);

		round.AddWinner( seat, score );

		bool const isDealer = round.IsDealer( seat );
		Points const winnings = table.m_rules->PointsFromPlayerRon( score.first, isDealer );

		for ( size_t seatI = 0; seatI < table.m_players.size(); ++seatI )
		{
			Seat const standingsSeat = ( Seat )seatI;
			if ( standingsSeat == seat )
			{
				table.m_standings.m_points[ round.GetPlayerID( standingsSeat, table ) ] += winnings;
			}
			else if ( standingsSeat == round.CurrentTurn() )
			{
				table.m_standings.m_points[ round.GetPlayerID( standingsSeat, table ) ] -= winnings;
			}
		}
	}

	// TODO-RULES: allow for negative points play
	if ( std::ranges::any_of( table.m_standings.m_points, []( Points i_points ) { return i_points < 0; } )
		|| round.NoMoreRounds( *table.m_rules ) )
	{
		table.Transition(
			TableStates::GameOver{ table },
			TableEvents::Ron{ m_discardedTile.m_tile, i_users }
		);
	}

	table.Transition(
		TableStates::BetweenRounds{ table },
		TableEvents::Ron{ m_discardedTile.m_tile, i_users }
	);
}

//------------------------------------------------------------------------------
RonAKanChance::RonAKanChance
(
	Table& i_table,
	TileDraw i_kanTile,
	SeatSet i_canRon
)
	: Base{ i_table }
	, m_kanTile{ i_kanTile }
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

		bool const allowedToRiichi = playerHand.Melds().empty() && !round.CalledRiichi( round.CurrentTurn() );
		auto const [ validWaits, canRiichi ] = table.m_rules->WaitsWithYaku(
			round,
			round.CurrentTurn(),
			playerHand,
			deadWallDraw,
			allowedToRiichi
		);

		bool const canTsumo = validWaits.contains( deadWallDraw.m_tile );

		Vector<Hand::DrawKanResult> kanOptions = round.GetHand( round.CurrentTurn() ).DrawKanOptions( &deadWallDraw.m_tile );
		table.Transition(
			TableStates::Turn_User{table, round.CurrentTurn(), canTsumo, canRiichi, std::move( kanOptions )},
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
	Table& table = m_table.get();

	Ensure( m_canRon.ContainsAllOf( i_players ), "Players tried to ron a kan when not allowed." );

	RoundData& round = table.m_rounds.back();

	// TODO-AI: assess whether any AI should join in ron
	// TODO-RULES: allow/disallow multiple ron

	for ( Seat seat : i_players )
	{
		Hand const hand = round.GetHand( seat );

		HandScore const score = table.m_rules->CalculateBasicPoints(
			round,
			seat,
			hand,
			m_kanTile
		);

		round.AddWinner( seat, score );

		bool const isDealer = round.IsDealer( seat );
		Points const winnings = table.m_rules->PointsFromPlayerRon( score.first, isDealer );

		for ( size_t seatI = 0; seatI < table.m_players.size(); ++seatI )
		{
			Seat const standingsSeat = ( Seat )seatI;
			if ( standingsSeat == seat )
			{
				table.m_standings.m_points[ round.GetPlayerID( standingsSeat, table ) ] += winnings;
			}
			else if ( standingsSeat == round.CurrentTurn() )
			{
				table.m_standings.m_points[ round.GetPlayerID( standingsSeat, table ) ] -= winnings;
			}
		}
	}

	// TODO-RULES: allow for negative points play
	if ( std::ranges::any_of( table.m_standings.m_points, []( Points i_points ) { return i_points < 0; } )
		|| round.NoMoreRounds( *table.m_rules ) )
	{
		table.Transition(
			TableStates::GameOver{ table },
			TableEvents::Ron{ m_kanTile.m_tile, i_players }
		);
	}

	table.Transition(
		TableStates::BetweenRounds{ table },
		TableEvents::Ron{ m_kanTile.m_tile, i_players }
	);
}

}