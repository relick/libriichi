#include "RoundData.hpp"

#include "Player.hpp"
#include "Rules.hpp"
#include "Table.hpp"

#include <numeric>
#include <ranges>

namespace Riichi
{

//------------------------------------------------------------------------------
void RoundData::RoundPlayerData::UpdateForTurn
(
)
{
	if ( !m_riichiDiscardTile.has_value() )
	{
		m_tempFuriten = false;
	}
}

//------------------------------------------------------------------------------
Seat RoundData::Wind
(
)	const
{
	return m_roundWind;
}

//------------------------------------------------------------------------------
Seat RoundData::CurrentTurn
(
)	const
{
	return m_currentTurn;
}

//------------------------------------------------------------------------------
bool RoundData::IsDealer
(
	Seat i_player
)	const
{
	return i_player == Seat::East;
}

//------------------------------------------------------------------------------
bool RoundData::CalledRiichi
(
	Seat i_player
)	const
{
	return m_players[ ( size_t )i_player ].m_riichiDiscardTile.has_value();
}

//------------------------------------------------------------------------------
bool RoundData::CalledDoubleRiichi
(
	Seat i_player
)	const
{
	return m_players[ ( size_t )i_player ].m_riichiDiscardTile.value_or( 1u ) == 0u;
}

//------------------------------------------------------------------------------
bool RoundData::RiichiIppatsuValid
(
	Seat i_player
)	const
{
	// TODO-MVP
	return false;
}

//------------------------------------------------------------------------------
bool RoundData::Furiten
(
	Seat i_player,
	Set<Tile> const& i_waits
)	const
{
	RoundPlayerData const& player = m_players[ ( size_t )i_player ];
	return player.m_tempFuriten
		|| std::ranges::any_of( player.m_discards, [ & ]( Tile const& i_tile ) { return i_waits.contains( i_tile ); } );
}

//------------------------------------------------------------------------------
Vector<Tile> const& RoundData::Discards
(
	Seat i_player
)	const
{
	return m_players[ ( size_t )i_player ].m_discards;
}

//------------------------------------------------------------------------------
Vector<Tile> const& RoundData::VisibleDiscards
(
	Seat i_player
)	const
{
	return m_players[ ( size_t )i_player ].m_visibleDiscards;
}

//------------------------------------------------------------------------------
Hand const& RoundData::GetHand
(
	Seat i_player
)	const
{
	return m_players[ ( size_t )i_player ].m_hand;
}

//------------------------------------------------------------------------------
Option<TileDraw> const& RoundData::DrawnTile
(
	Seat i_player
)	const
{
	return m_players[ ( size_t )i_player ].m_draw;
}

//------------------------------------------------------------------------------
bool RoundData::IsWinner
(
	Seat i_player
)	const
{
	return m_players[ ( size_t )i_player ].m_winningScore.has_value();
}

//------------------------------------------------------------------------------
Option<HandScore> const& RoundData::WinnerScore
(
	Seat i_player
)	const
{
	return m_players[ ( size_t )i_player ].m_winningScore;
}

//------------------------------------------------------------------------------
bool RoundData::FinishedInTenpai
(
	Seat i_player
)	const
{
	return m_players[ ( size_t )i_player ].m_finishedInTenpai;
}

//------------------------------------------------------------------------------
size_t RoundData::WallTilesRemaining
(
)	const
{
	Ensure( m_wall.size() >= m_deadWallSize, "Wall decremented into dead wall! Oh no!" );
	return m_wall.size() - m_deadWallSize;
}

//------------------------------------------------------------------------------
bool RoundData::CallsMade
(
)	const
{
	return std::ranges::any_of( m_players,
		[]( RoundPlayerData const& player )
		{
			return !player.m_hand.Melds().empty();
		}
	);
}

//------------------------------------------------------------------------------
Player const& RoundData::GetPlayer
(
	Seat i_player,
	Table const& i_table
)	const
{
	return i_table.GetPlayer( m_players[ ( size_t )i_player ].m_playerIndex );
}

//------------------------------------------------------------------------------
PlayerID RoundData::GetPlayerID
(
	Seat i_player,
	Table const& i_table
)	const
{
	return PlayerID( m_players[ ( size_t )i_player ].m_playerIndex );
}

//------------------------------------------------------------------------------
Seat RoundData::GetSeat
(
	PlayerID i_playerID
)	const
{
	size_t const playerIndex = i_playerID; // TODO-DEBT: actual id type?
	for ( size_t playerI = 0; playerI < m_players.size(); ++playerI )
	{
		if ( m_players[ playerI ].m_playerIndex == playerIndex )
		{
			return ( Seat )playerI;
		}
	}

	Error( "Did not find player in this round" );
	return Seat::East;
}

//------------------------------------------------------------------------------
bool RoundData::NoMoreRounds
(
	Rules const& i_rules
)	const
{
	// TODO-RULES: Should account for extension rounds
	bool const roundWindWillIncrement = NextRoundRotateSeat( i_rules )
		&& m_players[ ( size_t )NextPlayer( Seat::East, m_players.size() ) ].m_playerIndex == m_initialPlayerIndex;
	return roundWindWillIncrement && i_rules.LastRound() == m_roundWind;
}

//------------------------------------------------------------------------------
bool RoundData::NextRoundRotateSeat
(
	Rules const& i_rules
)	const
{
	// TODO-RULES: This is a bit hardcoded and in principle could be controlled by rules
	// We rotate if dealer did not win, or there was a draw and dealer was not in tenpai whilst others in tenpai
	return !IsWinner( Seat::East ) || ( !AnyWinners() && !FinishedInTenpai( Seat::East ) && AnyFinishedInTenpai() );
}

//------------------------------------------------------------------------------
bool RoundData::AnyWinners
(
)	const
{
	return std::ranges::any_of( m_players, []( RoundPlayerData const& i_player ) { return i_player.m_winningScore.has_value(); } );
}

//------------------------------------------------------------------------------
bool RoundData::AnyFinishedInTenpai
(
)	const
{
	return std::ranges::any_of( m_players, []( RoundPlayerData const& i_player ) { return i_player.m_finishedInTenpai; } );
}

//------------------------------------------------------------------------------
RoundData::RoundData
(
	Seat i_roundWind,
	Vector<Player> const& i_players,
	Rules const& i_rules,
	ShuffleRNG& i_shuffleRNG
)
	: m_deadWallSize{ i_rules.DeadWallSize() }
	, m_deadWallDrawsRemaining{ i_rules.DeadWallDrawsAvailable() }
{
	// Randomly determine initial seats
	Vector<size_t> playerIndices( i_players.size() );
	std::ranges::iota( playerIndices, 0 );
	std::ranges::shuffle( playerIndices, i_shuffleRNG );
	m_players.reserve( i_players.size() );
	for ( size_t index : playerIndices )
	{
		m_players.emplace_back(index );
	}
	m_initialPlayerIndex = m_players.front().m_playerIndex;

	// Shuffle the tiles to build the wall
	m_wall = i_rules.Tileset();
	std::ranges::shuffle( m_wall, i_shuffleRNG );
}

//------------------------------------------------------------------------------
RoundData::RoundData
(
	RoundData const& i_lastRound,
	Rules const& i_rules,
	ShuffleRNG& i_shuffleRNG
)
	: m_initialPlayerIndex{ i_lastRound.m_initialPlayerIndex }
	, m_deadWallSize{ i_rules.DeadWallSize() }
	, m_deadWallDrawsRemaining{ i_rules.DeadWallDrawsAvailable() }
	, m_roundWind{ i_lastRound.m_roundWind }
{
	Ensure( !i_lastRound.NoMoreRounds( i_rules ), "Tried to start a new round after last round declared game was over!" );

	// Copy players but then clear their data
	m_players.reserve( i_lastRound.m_players.size() );
	for ( RoundPlayerData const& player : i_lastRound.m_players )
	{
		m_players.emplace_back( player.m_playerIndex );
	}

	// TODO-RULES: honba
	if ( i_lastRound.IsWinner( Seat::East ) || !i_lastRound.AnyWinners() )
	{
		// Dealer won, or there was a draw
		// TODO-MVP: honba
	}

	bool rotated = false;
	if ( i_lastRound.NextRoundRotateSeat( i_rules ) )
	{
		std::ranges::rotate( m_players, m_players.end() - 1 );
		rotated = true;
	}

	// Increment round wind if we've done a full circuit
	if ( rotated && m_players[ ( size_t )Seat::East ].m_playerIndex == m_initialPlayerIndex )
	{
		m_roundWind = ( Seat )( ( EnumValueType )m_roundWind + 1 );
	}

	// Shuffle the tiles to build the wall
	m_wall = i_rules.Tileset();
	std::ranges::shuffle( m_wall, i_shuffleRNG );
}

//------------------------------------------------------------------------------
void RoundData::BreakWall
(
	ShuffleRNG& i_shuffleRNG
)
{
	std::uniform_int_distribution<size_t> dice{1, 6};
	size_t const d1 = dice( i_shuffleRNG );
	size_t const d2 = dice( i_shuffleRNG );
	size_t const dTotal = d1 + d2;

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
TileDraw RoundData::DealHands
(
)
{
	// Deal 4 at a time to each player until they have 12, then 1 at a time but 2 to the dealer
	for ( size_t i = 0; i < 3; ++i )
	{
		for ( RoundPlayerData& player : m_players )
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
Tile RoundData::Discard
(
	Option<Tile> const& i_handTileToDiscard
)
{
	RoundPlayerData& player = m_players[ ( size_t )m_currentTurn ];
	Tile discarded = [ & ]()
	{
		if ( i_handTileToDiscard.has_value() )
		{
			return i_handTileToDiscard.value();
		}
		Ensure( player.m_draw.has_value(), "Tried to discard drawn tile but didn't have one" );
		return player.m_draw.value().m_tile;
	}();
	player.m_discards.emplace_back( discarded );
	player.m_visibleDiscards.emplace_back( discarded );
	if ( i_handTileToDiscard.has_value() )
	{
		player.m_hand.Discard( discarded, player.m_draw );
	}
	player.m_draw.reset();
	return discarded;
}

//------------------------------------------------------------------------------
Tile RoundData::Riichi
(
	Option<Tile> const& i_handTileToDiscard
)
{
	RoundPlayerData& player = m_players[ ( size_t )m_currentTurn ];
	player.m_riichiDiscardTile = player.m_discards.size();
	return Discard( i_handTileToDiscard );
}

//------------------------------------------------------------------------------
TileDraw RoundData::PassCalls
(
	SeatSet const& i_couldRon
)
{
	for ( Seat seat : i_couldRon )
	{
		m_players[ ( size_t )seat ].m_tempFuriten = true;
	}

	m_currentTurn = NextPlayer( m_currentTurn, m_players.size() );

	RoundPlayerData& newPlayer = m_players[ ( size_t )m_currentTurn ];
	newPlayer.UpdateForTurn();

	newPlayer.m_draw = SelfDraw();
	return newPlayer.m_draw.value();
}

//------------------------------------------------------------------------------
Hand::KanResult RoundData::HandKan
(
	Tile const& i_tile
)
{
	RoundPlayerData& player = m_players[ ( size_t )m_currentTurn ];
	return player.m_hand.MakeKan( i_tile, std::nullopt );
}

//------------------------------------------------------------------------------
TileDraw RoundData::HandKanRonPass
(
)
{
	RoundPlayerData& player = m_players[ ( size_t )m_currentTurn ];

	player.m_draw = DeadWallDraw();
	return player.m_draw.value();
}

//------------------------------------------------------------------------------
Pair<Seat, Tile> RoundData::Chi
(
	Seat i_caller,
	Pair<Tile, Tile> const& i_meldTiles
)
{
	RoundPlayerData& current = m_players[ ( size_t )m_currentTurn ];
	Pair<Seat, Tile> const ret{ m_currentTurn, current.m_discards.back() };

	// Disappear it from the visible discards in front of the player
	current.m_visibleDiscards.pop_back();

	RoundPlayerData& caller = m_players[ ( size_t )i_caller ];
	caller.m_hand.MakeMeld( ret, { i_meldTiles.first, i_meldTiles.second }, GroupType::Sequence );

	m_currentTurn = i_caller;
	caller.UpdateForTurn();

	return ret;
}

//------------------------------------------------------------------------------
Pair<Seat, Tile> RoundData::Pon
(
	Seat i_caller
)
{
	// TODO-RULES: Uhhhh I guess I forgot that you might want to pon and have different options too like a chi
	RoundPlayerData& current = m_players[ ( size_t )m_currentTurn ];
	Pair<Seat, Tile> const ret{ m_currentTurn, current.m_discards.back() };

	// Disappear it from the visible discards in front of the player
	current.m_visibleDiscards.pop_back();

	RoundPlayerData& caller = m_players[ ( size_t )i_caller ];
	auto tile1 = std::find( caller.m_hand.FreeTiles().begin(), caller.m_hand.FreeTiles().end(), ret.second);
	auto tile2 = std::find( tile1 + 1, caller.m_hand.FreeTiles().end(), ret.second );

	caller.m_hand.MakeMeld( ret, { *tile1, *tile2 }, GroupType::Triplet );

	m_currentTurn = i_caller;
	caller.UpdateForTurn();

	return ret;
}

//------------------------------------------------------------------------------
Pair<Seat, Tile> RoundData::DiscardKan
(
	Seat i_caller
)
{
	RoundPlayerData& current = m_players[ ( size_t )m_currentTurn ];
	Pair<Seat, Tile> const ret{ m_currentTurn, current.m_discards.back() };

	// Disappear it from the visible discards in front of the player
	current.m_visibleDiscards.pop_back();

	RoundPlayerData& caller = m_players[ ( size_t )i_caller ];
	caller.m_hand.MakeKan( ret.second, ret.first );

	m_currentTurn = i_caller;
	caller.UpdateForTurn();

	return ret;
}

//------------------------------------------------------------------------------
Tile RoundData::AddWinner
(
	Seat i_player,
	HandScore const& i_score
)
{
	RoundPlayerData& player = m_players[ ( size_t )i_player ];
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
void RoundData::AddFinishedInTenpai
(
	Seat i_player
)
{
	RoundPlayerData& player = m_players[ ( size_t )i_player ];
	player.m_finishedInTenpai = true;
}

//------------------------------------------------------------------------------
Vector<Tile> RoundData::DealTiles
(
	size_t i_num
)
{
	Ensure( WallTilesRemaining() >= i_num, "Tried to draw more tiles than in wall" );

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
TileDraw RoundData::SelfDraw
(
)
{
	Ensure( WallTilesRemaining() >= 1, "Tried to draw more tiles than in wall" );

	Tile drawn = std::move( m_wall.back() );
	m_wall.pop_back();
	return { drawn, TileDrawType::SelfDraw, };
}

//------------------------------------------------------------------------------
TileDraw RoundData::DeadWallDraw
(
)
{
	Ensure( WallTilesRemaining() >= 1, "Tried to draw more tiles than in wall" );
	Ensure( m_deadWallDrawsRemaining >= 1, "Tried to draw more tiles than in dead wall pool" );

	Tile drawn = std::move( m_wall.front() );
	m_wall.erase( m_wall.begin() );
	return { drawn, TileDrawType::DeadWallDraw, };
}

}
