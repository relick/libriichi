#include "Round.hpp"

#include "Table.hpp"

#include <numeric>
#include "range/v3/algorithm.hpp"

namespace Riichi
{

//------------------------------------------------------------------------------
void Round::PlayerData::UpdateForTurn
(
)
{
	if ( !m_riichiDiscardTile.has_value() )
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
bool Round::IsDealer
(
	Seat i_player
)	const
{
	return i_player == Seat::East;
}

//------------------------------------------------------------------------------
bool Round::CalledRiichi
(
	Seat i_player
)	const
{
	return m_players[ ( size_t )i_player ].m_riichiDiscardTile.has_value();
}

//------------------------------------------------------------------------------
Option<size_t> Round::RiichiTileIndex
(
	Seat i_player
)	const
{
	return m_players[ ( size_t )i_player ].m_riichiDiscardTile;
}

//------------------------------------------------------------------------------
bool Round::CalledDoubleRiichi
(
	Seat i_player
)	const
{
	return m_players[ ( size_t )i_player ].m_riichiDiscardTile.value_or( 1u ) == 0u;
}

//------------------------------------------------------------------------------
bool Round::RiichiIppatsuValid
(
	Seat i_player
)	const
{
	// Ippatsu is valid if there have been no calls or further discards since the riichi of the player
	return m_players[ ( size_t )i_player ].m_riichiIppatsuValid;
}

//------------------------------------------------------------------------------
bool Round::Furiten
(
	Seat i_player,
	Set<Tile> const& i_waits
)	const
{
	PlayerData const& player = m_players[ ( size_t )i_player ];
	return player.m_tempFuriten
		|| ranges::any_of( player.m_discards, [ & ]( Tile const& i_tile ) { return i_waits.contains( i_tile ); } );
}

//------------------------------------------------------------------------------
Vector<Tile> const& Round::Discards
(
	Seat i_player
)	const
{
	return m_players[ ( size_t )i_player ].m_discards;
}

//------------------------------------------------------------------------------
Vector<Tile> const& Round::VisibleDiscards
(
	Seat i_player
)	const
{
	return m_players[ ( size_t )i_player ].m_visibleDiscards;
}

//------------------------------------------------------------------------------
Hand const& Round::GetHand
(
	Seat i_player
)	const
{
	return m_players[ ( size_t )i_player ].m_hand;
}

//------------------------------------------------------------------------------
Option<TileDraw> const& Round::DrawnTile
(
	Seat i_player
)	const
{
	return m_players[ ( size_t )i_player ].m_draw;
}

//------------------------------------------------------------------------------
bool Round::IsWinner
(
	Seat i_player
)	const
{
	return m_players[ ( size_t )i_player ].m_winningScore.has_value();
}

//------------------------------------------------------------------------------
Option<HandScore> const& Round::WinnerScore
(
	Seat i_player
)	const
{
	return m_players[ ( size_t )i_player ].m_winningScore;
}

//------------------------------------------------------------------------------
bool Round::FinishedInTenpai
(
	Seat i_player
)	const
{
	return m_players[ ( size_t )i_player ].m_finishedInTenpai;
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
	return ranges::any_of( m_players,
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
	return i_table.GetPlayer( m_players[ ( size_t )i_player ].m_playerID );
}

//------------------------------------------------------------------------------
PlayerID Round::GetPlayerID
(
	Seat i_player
)	const
{
	return m_players[ ( size_t )i_player ].m_playerID;
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
	return ranges::any_of( m_players, []( PlayerData const& i_player ) { return i_player.m_winningScore.has_value(); } );
}

//------------------------------------------------------------------------------
bool Round::AnyFinishedInTenpai
(
)	const
{
	return ranges::any_of( m_players, []( PlayerData const& i_player ) { return i_player.m_finishedInTenpai; } );
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
Vector<Tile> Round::GatherDoraTiles
(
	bool i_indicatedValue
)	const
{
	Vector<Tile> doraTiles;

	size_t firstDoraTileI = m_deadWallDrawsRemaining + 1;

	for ( size_t i = 0; i < m_doraCount; ++i )
	{
		// TODO-RULES: actually, some rulesets don't have all tiles, and they still need to work for indicated value.
		Tile const& wallTile = m_wall[ firstDoraTileI + ( i * 2 ) ];
		doraTiles.push_back( i_indicatedValue ? NextTile( wallTile ) : wallTile );
	}

	return doraTiles;
}

//------------------------------------------------------------------------------
Vector<Tile> Round::GatherUradoraTiles
(
	bool i_indicatedValue
)	const
{
	Vector<Tile> doraTiles;

	size_t firstUradoraTileI = m_deadWallDrawsRemaining;

	for ( size_t i = 0; i < m_doraCount; ++i )
	{
		// TODO-RULES: actually, some rulesets don't have all tiles, and they still need to work for indicated value.
		Tile const& wallTile = m_wall[ firstUradoraTileI + ( i * 2 ) ];
		doraTiles.push_back( i_indicatedValue ? NextTile( wallTile ) : wallTile );
	}

	return doraTiles;
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
	riEnsure( i_playerIDs.size() == i_rules.GetPlayerCount().Get(), "Did not provide enough players to start round" );

	// Randomly determine initial seats
	m_players.reserve( i_playerIDs.size() );
	for ( PlayerID playerID : i_playerIDs )
	{
		m_players.emplace_back( playerID );
	}
	ranges::shuffle( m_players, i_shuffleRNG );
	m_initialPlayerID = m_players.front().m_playerID;

	// Shuffle the tiles to build the wall
	m_wall = i_rules.Tileset();
	riEnsure( ranges::all_of( m_wall, []( Tile const& i_tile ) { return i_tile.HasID(); } ), "Not all tiles were assigned IDs in ruleset" );
	ranges::shuffle( m_wall, i_shuffleRNG );

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
		ranges::rotate( m_players, m_players.begin() + 1 );
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
	ranges::shuffle( m_wall, i_shuffleRNG );

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
	ranges::rotate( m_wall, m_wall.begin() + m_breakPointFromDealerRight );
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
	m_players.front().m_draw = SelfDraw();
	for ( size_t playerI = 1; playerI < m_players.size(); ++playerI )
	{
		m_players[ playerI ].m_hand.AddFreeTiles( DealTiles( 1 ) );
	}

	return m_players.front().m_draw.value();
}

//------------------------------------------------------------------------------
Tile Round::Discard
(
	Option<Tile> const& i_handTileToDiscard
)
{
	PlayerData& player = m_players[ ( size_t )m_currentTurn ];
	Tile discarded = [ & ]()
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
	player.m_riichiIppatsuValid = false;
	return discarded;
}

//------------------------------------------------------------------------------
Tile Round::Riichi
(
	Option<Tile> const& i_handTileToDiscard
)
{
	PlayerData& player = m_players[ ( size_t )m_currentTurn ];
	player.m_riichiDiscardTile = player.m_discards.size();

	Tile const discarded = Discard( i_handTileToDiscard );
	// Set ippatsu valid afterwards as Discard will reset it
	player.m_riichiIppatsuValid = true;

	// Add the stick
	++m_riichiSticks;

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
		m_players[ ( size_t )seat ].m_tempFuriten = true;
	}

	m_currentTurn = NextPlayer( m_currentTurn, m_players.size() );

	PlayerData& newPlayer = m_players[ ( size_t )m_currentTurn ];
	newPlayer.UpdateForTurn();

	newPlayer.m_draw = SelfDraw();
	return newPlayer.m_draw.value();
}

//------------------------------------------------------------------------------
Hand::KanResult Round::HandKan
(
	Option<Tile> const& i_handTileToKan
)
{
	PlayerData& player = m_players[ ( size_t )m_currentTurn ];
	Hand::KanResult res = player.m_hand.MakeKan( i_handTileToKan.value_or( player.m_draw.value().m_tile ), i_handTileToKan.has_value(), std::nullopt );
	player.m_draw.reset();
	return res;
}

//------------------------------------------------------------------------------
TileDraw Round::HandKanRonPass
(
)
{
	PlayerData& player = m_players[ ( size_t )m_currentTurn ];

	player.m_draw = DeadWallDraw();
	return player.m_draw.value();
}

//------------------------------------------------------------------------------
Pair<Seat, Tile> Round::Chi
(
	Seat i_caller,
	Pair<Tile, Tile> const& i_meldTiles
)
{
	PlayerData& current = m_players[ ( size_t )m_currentTurn ];
	Pair<Seat, Tile> const ret{ m_currentTurn, current.m_discards.back() };

	// Disappear it from the visible discards in front of the player
	current.m_visibleDiscards.pop_back();

	PlayerData& caller = m_players[ ( size_t )i_caller ];
	caller.m_hand.MakeMeld( ret, { i_meldTiles.first, i_meldTiles.second }, GroupType::Sequence );

	// Invalidate riichi ippatsu as call made
	for ( PlayerData& player : m_players )
	{
		player.m_riichiIppatsuValid = false;
	}

	m_currentTurn = i_caller;
	caller.UpdateForTurn();

	return ret;
}

//------------------------------------------------------------------------------
Pair<Seat, Tile> Round::Pon
(
	Seat i_caller
)
{
	// TODO-RULES: Uhhhh I guess I forgot that you might want to pon and have different options too like a chi
	PlayerData& current = m_players[ ( size_t )m_currentTurn ];
	Pair<Seat, Tile> const ret{ m_currentTurn, current.m_discards.back() };

	// Disappear it from the visible discards in front of the player
	current.m_visibleDiscards.pop_back();

	PlayerData& caller = m_players[ ( size_t )i_caller ];
	auto tile1 = std::find( caller.m_hand.FreeTiles().begin(), caller.m_hand.FreeTiles().end(), ret.second);
	auto tile2 = std::find( tile1 + 1, caller.m_hand.FreeTiles().end(), ret.second );

	caller.m_hand.MakeMeld( ret, { *tile1, *tile2 }, GroupType::Triplet );

	// Invalidate riichi ippatsu as call made
	for ( PlayerData& player : m_players )
	{
		player.m_riichiIppatsuValid = false;
	}

	m_currentTurn = i_caller;
	caller.UpdateForTurn();

	return ret;
}

//------------------------------------------------------------------------------
Pair<TileDraw, Pair<Seat, Tile>> Round::DiscardKan
(
	Seat i_caller
)
{
	PlayerData& current = m_players[ ( size_t )m_currentTurn ];
	Pair<Seat, Tile> const ret{ m_currentTurn, current.m_discards.back() };

	// Disappear it from the visible discards in front of the player
	current.m_visibleDiscards.pop_back();

	PlayerData& caller = m_players[ ( size_t )i_caller ];
	caller.m_hand.MakeKan( ret.second, false, ret.first );

	// Invalidate riichi ippatsu as call made
	for ( PlayerData& player : m_players )
	{
		player.m_riichiIppatsuValid = false;
	}

	m_currentTurn = i_caller;
	caller.UpdateForTurn();

	caller.m_draw = DeadWallDraw();

	return { caller.m_draw.value(), ret };
}

//------------------------------------------------------------------------------
Tile Round::AddWinner
(
	Seat i_player,
	HandScore const& i_score
)
{
	PlayerData& player = m_players[ ( size_t )i_player ];
	player.m_winningScore = i_score;

	if ( player.m_draw.has_value() )
	{
		// Was tsumo
		return player.m_draw.value().m_tile;
	}

	// Was ron
	return m_players[ ( size_t )m_currentTurn ].m_discards.back();
}

//------------------------------------------------------------------------------
void Round::AddFinishedInTenpai
(
	Seat i_player
)
{
	PlayerData& player = m_players[ ( size_t )i_player ];
	player.m_finishedInTenpai = true;
}

//------------------------------------------------------------------------------
Vector<Tile> Round::DealTiles
(
	size_t i_num
)
{
	riEnsure( WallTilesRemaining() >= i_num, "Tried to draw more tiles than in wall" );

	Vector<Tile> tiles;
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

	Tile drawn = std::move( m_wall.back() );
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

	Tile drawn = std::move( m_wall.front() );
	m_wall.erase( m_wall.begin() );
	return { drawn, TileDrawType::DeadWallDraw, };
}

}
