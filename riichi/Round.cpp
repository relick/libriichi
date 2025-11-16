#include "Round.hpp"

#include "Table.hpp"

#include <numeric>

namespace Riichi
{

//------------------------------------------------------------------------------
void Round::PlayerData::UpdateForTurn
(
)
{
	if ( !m_riichi )
	{
		m_tempFuriten = false;
	}
}

//------------------------------------------------------------------------------
SeatSet Round::Seats
(
)	const
{
	SeatSet seats;
	for ( size_t i = 0; i < m_players.size(); ++i )
	{
		seats.Insert( static_cast< Seat >( i ) );
	}
	return seats;
}

//------------------------------------------------------------------------------
Seat Round::Wind
(
)	const
{
	return m_roundWind;
}

//------------------------------------------------------------------------------
Seat Round::CurrentTurn
(
)	const
{
	return m_currentTurn;
}

//------------------------------------------------------------------------------
bool Round::CalledRiichi
(
	Seat i_player
)	const
{
	return Player( i_player ).m_riichi.has_value();
}

//------------------------------------------------------------------------------
bool Round::CalledDoubleRiichi
(
	Seat i_player
)	const
{
	return CalledRiichi( i_player ) && Player( i_player ).m_riichi->m_sidewaysDiscardIndex == 0u;
}

//------------------------------------------------------------------------------
bool Round::WaitingToPayRiichiBet
(
	Seat i_player
)	const
{
	return CalledRiichi( i_player ) && Player( i_player ).m_riichi->m_waitingToPayBet;
}

//------------------------------------------------------------------------------
bool Round::RiichiIppatsuValid
(
	Seat i_player
)	const
{
	// Ippatsu is valid if there have been no calls or further discards since the riichi of the player
	return CalledRiichi( i_player ) && Player( i_player ).m_riichi->m_ippatsuValid;
}

//------------------------------------------------------------------------------
bool Round::Furiten
(
	Seat i_player,
	Set<TileKind> const& i_waits
)	const
{
	PlayerData const& player = Player( i_player );
	return player.m_tempFuriten
		|| std::ranges::any_of( player.m_discards, [ & ]( TileInstance const& i_tile ) { return i_waits.contains( i_tile.Tile() ); } );
}

//------------------------------------------------------------------------------
Vector<TileInstance> const& Round::Discards
(
	Seat i_player
)	const
{
	return Player( i_player ).m_discards;
}

//------------------------------------------------------------------------------
Pair<Vector<TileInstance> const&, Option<size_t>> Round::VisibleDiscards
(
	Seat i_player
)	const
{
	return { Player( i_player ).m_visibleDiscards, CalledRiichi( i_player ) ? Option<size_t>( Player( i_player ).m_riichi->m_sidewaysDiscardIndex ) : Option<size_t>() };
}

//------------------------------------------------------------------------------
Hand const& Round::CurrentHand
(
	Seat i_player
)	const
{
	return Player( i_player ).m_hand;
}

//------------------------------------------------------------------------------
Option<TileDraw> const& Round::CurrentTileDraw
(
	Seat i_player
)	const
{
	return Player( i_player ).m_draw;
}

//------------------------------------------------------------------------------
bool Round::IsWinner
(
	Seat i_player
)	const
{
	return Player( i_player ).m_endOfRound && Player( i_player ).m_endOfRound->m_winScores;
}

//------------------------------------------------------------------------------
Option<Round::WinScores> const& Round::WinnerScores
(
	Seat i_player
)	const
{
	riEnsure( Player( i_player ).m_endOfRound.has_value(), "Cannot ask for winner score before end of round info received" );
	return Player( i_player ).m_endOfRound->m_winScores;
}

//------------------------------------------------------------------------------
Points Round::EndOfRoundTablePayment
(
	Seat i_player
)	const
{
	riEnsure( Player( i_player ).m_endOfRound.has_value(), "Cannot ask for end of round table payment before end of round info received" );
	return Player( i_player ).m_endOfRound->m_tablePayment;
}

//------------------------------------------------------------------------------
bool Round::FinishedInTenpai
(
	Seat i_player
)	const
{
	return Player( i_player ).m_endOfRound && Player( i_player ).m_endOfRound->m_finishedInTenpai;
}

//------------------------------------------------------------------------------
size_t Round::WallTilesRemaining
(
)	const
{
	riEnsure( m_wall.size() >= m_deadWallSize, "Wall decremented into dead wall! Oh no!" );
	return m_wall.size() - m_deadWallSize;
}

//------------------------------------------------------------------------------
bool Round::CallsMade
(
)	const
{
	return std::ranges::any_of( m_players,
		[]( PlayerData const& player )
		{
			return !player.m_hand.Melds().empty();
		}
	);
}

//------------------------------------------------------------------------------
Player const& Round::GetPlayer
(
	Seat i_player,
	Table const& i_table
)	const
{
	return i_table.GetPlayer( GetPlayerID( i_player ) );
}

//------------------------------------------------------------------------------
PlayerID Round::GetPlayerID
(
	Seat i_player
)	const
{
	return Player( i_player ).m_playerID;
}

//------------------------------------------------------------------------------
Seat Round::GetSeat
(
	PlayerID i_playerID
)	const
{
	for ( size_t playerI = 0; playerI < m_players.size(); ++playerI )
	{
		if ( m_players[ playerI ].m_playerID == i_playerID )
		{
			return ( Seat )playerI;
		}
	}

	riError( "Did not find player in this round" );
	return Seat::East;
}

//------------------------------------------------------------------------------
bool Round::AnyWinners
(
)	const
{
	return std::ranges::any_of( m_players, []( PlayerData const& i_player ) { return i_player.m_endOfRound && i_player.m_endOfRound->m_winScores; } );
}

//------------------------------------------------------------------------------
bool Round::AnyFinishedInTenpai
(
)	const
{
	return std::ranges::any_of( m_players, []( PlayerData const& i_player ) { return i_player.m_endOfRound && i_player.m_endOfRound->m_finishedInTenpai; } );
}

//------------------------------------------------------------------------------
size_t Round::HonbaSticks
(
)	const
{
	return m_honbaSticks;
}

//------------------------------------------------------------------------------
size_t Round::RiichiSticks
(
)	const
{
	return m_riichiSticks;
}

//------------------------------------------------------------------------------
size_t Round::BreakPointFromDealerRight
(
)	const
{
	return m_breakPointFromDealerRight;
}

//------------------------------------------------------------------------------
size_t Round::DeadWallDrawsRemaining
(
)	const
{
	return m_deadWallDrawsRemaining;
}

//------------------------------------------------------------------------------
bool Round::NextPlayerIsInitial
(
)	const
{
	if ( m_players.size() <= 1 )
	{
		return true;
	}
	return m_players[ 1 ].m_playerID == m_initialPlayerID;
}

//------------------------------------------------------------------------------
Vector<TileKind> Round::GetDoraTiles
(
	bool i_includeUradora
)	const
{
	Vector<TileKind> doraTiles;

	size_t firstDoraTileI = m_deadWallDrawsRemaining + 1;
	size_t firstUradoraTileI = m_deadWallDrawsRemaining;

	// TODO-RULES: using .Next() on the tile isn't great since e.g. sanma is missing tiles and would want to do it differently.

	for ( size_t i = 0; i < m_doraCount; ++i )
	{
		doraTiles.push_back( m_wall[ firstDoraTileI + ( i * 2 ) ].Tile().Next() );
	}

	if ( i_includeUradora )
	{
		for ( size_t i = 0; i < m_doraCount; ++i )
		{
			doraTiles.push_back( m_wall[ firstUradoraTileI + ( i * 2 ) ].Tile().Next() );
		}
	}

	return doraTiles;
}

//------------------------------------------------------------------------------
Vector<TileInstance> Round::GetDoraIndicatorTiles
(
	bool i_includeUradora
)	const
{
	Vector<TileInstance> doraIndicatorTiles;

	size_t firstDoraTileI = m_deadWallDrawsRemaining + 1;
	size_t firstUradoraTileI = m_deadWallDrawsRemaining;

	for ( size_t i = 0; i < m_doraCount; ++i )
	{
		doraIndicatorTiles.push_back( m_wall[ firstDoraTileI + ( i * 2 ) ] );
	}

	if ( i_includeUradora )
	{
		for ( size_t i = 0; i < m_doraCount; ++i )
		{
			doraIndicatorTiles.push_back( m_wall[ firstUradoraTileI + ( i * 2 ) ] );
		}
	}

	return doraIndicatorTiles;
}

//------------------------------------------------------------------------------
Round::Round
(
	Seat i_roundWind,
	Vector<PlayerID> const& i_playerIDs,
	Rules const& i_rules,
	ShuffleRNG& i_shuffleRNG
)
	: m_deadWallSize{ i_rules.DeadWallSize() }
	, m_deadWallDrawsRemaining{ i_rules.DeadWallDrawsAvailable() }
{
	riEnsure( i_playerIDs.size() == i_rules.GetPlayerCount(), "Did not provide enough players to start round" );

	// Randomly determine initial seats
	m_players.reserve( i_playerIDs.size() );
	for ( PlayerID playerID : i_playerIDs )
	{
		m_players.emplace_back( playerID );
	}
	std::ranges::shuffle( m_players, i_shuffleRNG );
	m_initialPlayerID = m_players.front().m_playerID;

	// Shuffle the tiles to build the wall
	m_wall = i_rules.Tileset();
	std::ranges::sort( m_wall, CompareTileInstanceIDOp{} );
	riEnsure( std::ranges::unique( m_wall, EqualsTileInstanceIDOp{} ).empty(), "Not all tiles were assigned unique IDs in ruleset");
	std::ranges::shuffle( m_wall, i_shuffleRNG );

	// Then break the wall
	BreakWall( i_shuffleRNG );
}

//------------------------------------------------------------------------------
Round::Round
(
	Table const& i_table,
	Round const& i_previousRound,
	Rules const& i_rules,
	ShuffleRNG& i_shuffleRNG
)
	: m_initialPlayerID{ i_previousRound.m_initialPlayerID }
	, m_deadWallSize{ i_rules.DeadWallSize() }
	, m_deadWallDrawsRemaining{ i_rules.DeadWallDrawsAvailable() }
	, m_roundWind{ i_previousRound.m_roundWind }
	, m_honbaSticks{ i_previousRound.m_honbaSticks }
	, m_riichiSticks{ i_previousRound.m_riichiSticks }
{
	riEnsure( !i_rules.NoMoreRounds( i_table, i_previousRound ), "Tried to start a new round after last round declared game was over!" );

	// Copy players but then clear their data
	m_players.reserve( i_previousRound.m_players.size() );
	for ( PlayerData const& player : i_previousRound.m_players )
	{
		m_players.emplace_back( player.m_playerID );
	}

	bool const repeatRound = i_rules.RepeatRound( i_previousRound );
	if ( !repeatRound )
	{
		std::ranges::rotate( m_players, m_players.begin() + 1 );
	}

	// Increment round wind if we've done a full circuit
	if ( !repeatRound && m_players.front().m_playerID == m_initialPlayerID )
	{
		m_roundWind = ( Seat )( ( EnumValueType )m_roundWind + 1 );
	}

	bool const addHonba = i_rules.ShouldAddHonba( i_previousRound );
	if ( i_previousRound.AnyWinners() )
	{
		m_riichiSticks = 0;
		if ( !addHonba )
		{
			m_honbaSticks = 0;
		}
	}

	if ( addHonba )
	{
		++m_honbaSticks;
	}

	// Shuffle the tiles to build the wall
	m_wall = i_rules.Tileset();
	std::ranges::shuffle( m_wall, i_shuffleRNG );

	// Then break the wall
	BreakWall( i_shuffleRNG );
}

//------------------------------------------------------------------------------
void Round::BreakWall
(
	ShuffleRNG& i_shuffleRNG
)
{
	std::uniform_int_distribution<size_t> dice{1, 6};
	size_t const d1 = dice( i_shuffleRNG );
	size_t const d2 = dice( i_shuffleRNG );
	size_t const dTotal = d1 + d2;

	// TODO-RULES: store this dice somewhere, needed for wareme
	// TODO-RULES: actually, do we also want rules to control the dice roll itself?

	// Tiles are currently ordered clockwise from dealer's right, starting at the back of the vector
	// We have to count down tiles, e.g. for yonma starting from 136 for (1)/5/9, 34 for 2/6/10, 68 for 3/7/11, 102 for 4/8/12
	// Can note that we can treat it as tilesPerPlayer * ((dTotal - 1) % playerCount) as long as 0 is then treated as max afterwards
	
	size_t const wallIndexCCW = ( dTotal - 1 ) % m_players.size();
	size_t const tilesPerPlayer = m_wall.size() / m_players.size();
	
	// Lastly we put it together and move it along the wall by the dice again for the point in the wall we actually break
	size_t const breakingWallStartTile = ( wallIndexCCW == 0 ? m_wall.size() : wallIndexCCW * tilesPerPlayer ) - ( dTotal * 2 );

	// Reverse saved break point to make visuals easier (this number therefore starts from 0 and goes clockwise)
	m_breakPointFromDealerRight = m_wall.size() - breakingWallStartTile;

	// Tada! Put into position ready for draws
	std::ranges::rotate( m_wall, m_wall.begin() + m_breakPointFromDealerRight );
}

//------------------------------------------------------------------------------
TileDraw Round::DealHands
(
)
{
	// Deal 4 at a time to each player until they have 12, then 1 at a time but 2 to the dealer
	for ( size_t i = 0; i < 3; ++i )
	{
		for ( PlayerData& player : m_players )
		{
			player.m_hand.AddFreeTiles( DealTiles( 4 ) );
		}
	}

	m_players.front().m_hand.AddFreeTiles( DealTiles( 1 ) );
	for ( size_t playerI = 1; playerI < m_players.size(); ++playerI )
	{
		m_players[ playerI ].m_hand.AddFreeTiles( DealTiles( 1 ) );
	}
	m_players.front().m_draw = SelfDraw();

	return m_players.front().m_draw.value();
}

//------------------------------------------------------------------------------
TileInstance Round::Discard
(
	Option<TileInstance> const& i_handTileToDiscard
)
{
	PlayerData& player = CurrentPlayer();
	TileInstance discarded = [ & ]()
	{
		if ( i_handTileToDiscard.has_value() )
		{
			return i_handTileToDiscard.value();
		}
		riEnsure( player.m_draw.has_value(), "Tried to discard drawn tile but didn't have one" );
		return player.m_draw.value().m_tile;
	}();
	player.m_discards.emplace_back( discarded );
	player.m_visibleDiscards.emplace_back( discarded );
	if ( i_handTileToDiscard.has_value() )
	{
		player.m_hand.Discard( discarded, player.m_draw );
	}
	player.m_draw.reset();

	if ( player.m_riichi )
	{
		player.m_riichi->m_ippatsuValid = false;
	}

	return discarded;
}

//------------------------------------------------------------------------------
TileInstance Round::Riichi
(
	Option<TileInstance> const& i_handTileToDiscard
)
{
	PlayerData& player = CurrentPlayer();

	riEnsure( !player.m_riichi, "Cannot riichi twice!" );

	size_t const sidewaysDiscardIndex = player.m_visibleDiscards.size();
	TileInstance const discarded = Discard( i_handTileToDiscard );

	// Set this after discarding to ensure discarding does not affect ippatsu
	player.m_riichi = PlayerData::Riichi{ sidewaysDiscardIndex };

	return discarded;
}

//------------------------------------------------------------------------------
TileDraw Round::PassCalls
(
	SeatSet const& i_couldRon
)
{
	for ( Seat seat : i_couldRon )
	{
		Player( seat ).m_tempFuriten = true;
	}

	bool constexpr c_callMade = false;
	PlayerData& newPlayer = StartTurn( NextPlayer( m_currentTurn, m_players.size() ), c_callMade );

	riEnsure( !newPlayer.m_draw, "Should not already have a drawn tile" );
	newPlayer.m_draw = SelfDraw();
	return newPlayer.m_draw.value();
}

//------------------------------------------------------------------------------
void Round::HandKan
(
	HandKanOption const& i_kanOption
)
{
	PlayerData& player = CurrentPlayer();

	Meld const& resultingMeld = player.m_hand.CallMeldFromHand( i_kanOption );
	riEnsure( resultingMeld.Quad()
		&& ( !resultingMeld.Open() == i_kanOption.m_closed )
		&& ( resultingMeld.UpgradedQuad() == !i_kanOption.m_closed ),
		"Failed to make expected quad from kan option" );

	// Handle the drawn tile now. It will be replaced with the dead wall draw soon
	if ( i_kanOption.m_drawnTileInvolved )
	{
		// Drawn tile is now part of the kan
		riEnsure( i_kanOption.m_drawnTileInvolved->ID() == player.m_draw->m_tile.ID(), "Unexpected drawn tile when calling kan" );
		player.m_draw.reset();
	}
	else
	{
		// Need to add the drawn tile that wasn't involved in the kan to the hand
		player.m_hand.AddFreeTiles( { player.m_draw->m_tile } );
		player.m_draw.reset();
	}
}

//------------------------------------------------------------------------------
TileDraw Round::HandKanRonPass
(
)
{
	// Invalidate riichi ippatsu as call made
	for ( PlayerData& player : m_players )
	{
		if ( player.m_riichi )
		{
			player.m_riichi->m_ippatsuValid = false;
		}
	}

	PlayerData& player = CurrentPlayer();

	riEnsure( !player.m_draw, "Should not already have a drawn tile" );
	player.m_draw = DeadWallDraw();
	return player.m_draw.value();
}

//------------------------------------------------------------------------------
Meld::CalledTile Round::Chi
(
	Seat i_caller,
	Pair<TileInstance, TileInstance> const& i_meldTiles
)
{
	PlayerData& current = CurrentPlayer();
	TileInstance const calledDiscard = current.m_discards.back();
	Seat const calledFrom = m_currentTurn;

	// Disappear it from the visible discards in front of the player
	current.m_visibleDiscards.pop_back();

	PlayerData& caller = Player( i_caller );
	Meld const& newMeld = caller.m_hand.CallMeld( calledDiscard, calledFrom, i_meldTiles.first, i_meldTiles.second );

	bool constexpr c_callMade = true;
	StartTurn( i_caller, c_callMade );

	return { newMeld.GetCalledTile(), newMeld.CalledTileFrom() };
}

//------------------------------------------------------------------------------
Meld::CalledTile Round::Pon
(
	Seat i_caller
)
{
	PlayerData& current = CurrentPlayer();
	TileInstance const calledDiscard = current.m_discards.back();
	Seat const calledFrom = m_currentTurn;

	// Disappear it from the visible discards in front of the player
	current.m_visibleDiscards.pop_back();

	// TODO-RULES: Uhhhh I guess I forgot that you might want to pon and have different options too like a chi
	PlayerData& caller = Player( i_caller );
	EqualsTileKind const sharesTileKind{ calledDiscard };
	auto tile1 = std::ranges::find_if( caller.m_hand.FreeTiles(), sharesTileKind );
	auto tile2 = std::find_if( tile1 + 1, caller.m_hand.FreeTiles().end(), sharesTileKind );

	Meld const& newMeld = caller.m_hand.CallMeld( calledDiscard, calledFrom, *tile1, *tile2 );

	bool constexpr c_callMade = true;
	StartTurn( i_caller, c_callMade );

	return { newMeld.GetCalledTile(), newMeld.CalledTileFrom() };
}

//------------------------------------------------------------------------------
Pair<TileDraw, Meld::CalledTile> Round::DiscardKan
(
	Seat i_caller
)
{
	PlayerData& current = CurrentPlayer();
	TileInstance const calledDiscard = current.m_discards.back();
	Seat const calledFrom = m_currentTurn;

	// Disappear it from the visible discards in front of the player
	current.m_visibleDiscards.pop_back();

	PlayerData& caller = Player( i_caller );

	// Get the free hand tiles associated with this kan
	Vector<TileInstance> freeHandTiles;
	freeHandTiles.reserve( 3 );

	EqualsTileKind const sharesTileKind{ calledDiscard };
	for ( TileInstance const& tile : caller.m_hand.FreeTiles() )
	{
		if ( sharesTileKind( tile ) )
		{
			freeHandTiles.push_back( tile );
		}
	}
	riEnsure( freeHandTiles.size() == 3, "Invalid discard kan" );

	// Make the meld
	Meld const& newMeld = caller.m_hand.CallMeld(
		calledDiscard, calledFrom,
		freeHandTiles[ 0 ],
		freeHandTiles[ 1 ],
		freeHandTiles[ 2 ]
	);

	bool constexpr c_callMade = true;
	StartTurn( i_caller, c_callMade );

	caller.m_draw = DeadWallDraw();

	return { caller.m_draw.value(), { newMeld.GetCalledTile(), newMeld.CalledTileFrom() } };
}

//------------------------------------------------------------------------------
void Round::RiichiBetPaid
(
	Seat i_player
)
{
	PlayerData& player = Player( i_player );

	riEnsure( player.m_riichi, "Player must have already called riichi, to add a riichi stick" );
	riEnsure( player.m_riichi->m_waitingToPayBet, "Player must not have already added a riichi stick" );

	++m_riichiSticks;
	player.m_riichi->m_waitingToPayBet = false;
}

//------------------------------------------------------------------------------
TileInstance Round::AddWinner
(
	Seat i_player,
	HandScore i_handScore,
	FinalScore i_finalScore
)
{
	PlayerData& player = Player( i_player );
	if ( !player.m_endOfRound )
	{
		player.m_endOfRound = PlayerData::EndOfRound{};
	}
	player.m_endOfRound->m_winScores = WinScores{ std::move( i_handScore ) , i_finalScore };

	if ( player.m_draw )
	{
		// Was tsumo
		return player.m_draw.value().m_tile;
	}

	// Was ron
	return CurrentPlayer().m_discards.back();
}

//------------------------------------------------------------------------------
void Round::AddFinishedInTenpai
(
	Seat i_player
)
{
	PlayerData& player = Player( i_player );
	if ( !player.m_endOfRound )
	{
		player.m_endOfRound = PlayerData::EndOfRound{};
	}
	player.m_endOfRound->m_finishedInTenpai = true;
}

//------------------------------------------------------------------------------
void Round::ApplyPayments
(
	TablePayments const& i_payments,
	Table& io_table
)
{
	for ( size_t i = 0; i < m_players.size(); ++i )
	{
		Seat const seat = ( Seat )i;
		io_table.ModifyPoints( GetPlayerID( seat ), i_payments.m_pointsPerSeat[ seat ] );
	}
}

//------------------------------------------------------------------------------
void Round::ApplyEndOfRoundPayments
(
	TablePayments const& i_payments,
	Table& io_table
)
{
	// Applies the payments, then saves that payment into the end of round data for each player
	ApplyPayments( i_payments, io_table );

	for ( size_t i = 0; i < m_players.size(); ++i )
	{
		Seat const seat = ( Seat )i;
		if ( !Player( seat ).m_endOfRound )
		{
			Player( seat ).m_endOfRound = PlayerData::EndOfRound{};
		}
		Player( seat ).m_endOfRound->m_tablePayment = i_payments.m_pointsPerSeat[ seat ];
	}
}

//------------------------------------------------------------------------------
Vector<TileInstance> Round::DealTiles
(
	size_t i_num
)
{
	riEnsure( WallTilesRemaining() >= i_num, "Tried to draw more tiles than in wall" );

	Vector<TileInstance> tiles;
	tiles.reserve( i_num );
	for ( size_t i = 0; i < i_num; ++i )
	{
		tiles.emplace_back( std::move( m_wall.back() ) );
		m_wall.pop_back();
	}
	return tiles;
}

//------------------------------------------------------------------------------
TileDraw Round::SelfDraw
(
)
{
	riEnsure( WallTilesRemaining() >= 1, "Tried to draw more tiles than in wall" );

	TileInstance drawn = std::move( m_wall.back() );
	m_wall.pop_back();
	return { drawn, TileDrawType::SelfDraw, };
}

//------------------------------------------------------------------------------
TileDraw Round::DeadWallDraw
(
)
{
	riEnsure( WallTilesRemaining() >= 1, "Tried to draw more tiles than in wall" );
	riEnsure( m_deadWallDrawsRemaining >= 1, "Tried to draw more tiles than in dead wall pool" );

	--m_deadWallDrawsRemaining;
	++m_doraCount;

	TileInstance drawn = std::move( m_wall.front() );
	m_wall.erase( m_wall.begin() );
	return { drawn, TileDrawType::DeadWallDraw, };
}

//------------------------------------------------------------------------------
Round::PlayerData& Round::StartTurn
(
	Seat i_player,
	bool i_callMade
)
{
	riEnsure( Seats().Contains( i_player ), "Started turn for invalid player" );

	if ( i_callMade )
	{
		// Invalidate riichi ippatsu as call made
		for ( PlayerData& player : m_players )
		{
			if ( player.m_riichi )
			{
				player.m_riichi->m_ippatsuValid = false;
			}
		}
	}

	// Set the current turn and apply PlayerData updates
	m_currentTurn = i_player;
	Player( i_player ).UpdateForTurn();

	return Player( i_player );
}

}
