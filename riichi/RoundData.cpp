#include "RoundData.hpp"

#include "Player.hpp"
#include "Rules.hpp"
#include "Table.hpp"

#include <numeric>
#include <ranges>

namespace Riichi
{

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
Option<Tile> const& RoundData::DrawnTile
(
	Seat i_player
)	const
{
	return m_players[ ( size_t )i_player ].m_draw;
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
	// Copy players but then clear their data
	m_players.reserve( i_lastRound.m_players.size() );
	for ( RoundPlayerData const& player : i_lastRound.m_players )
	{
		m_players.emplace_back( player.m_playerIndex );
	}

	// TODO-MVP: work out the situations when we should rotate player based on last round win
	// for now always rotate
	std::ranges::rotate( m_players, m_players.begin() + 1 );
	bool const rotated = true;

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
Tile RoundData::DealHands
(
)
{
	// Deal 4 at a time to each player until they have 12, then 1 at a time but 2 to the dealer
	for ( size_t i = 0; i < 3; ++i )
	{
		for ( RoundPlayerData& player : m_players )
		{
			player.m_hand.AddFreeTiles( DrawTiles( 4 ) );
		}
	}

	m_players.front().m_hand.AddFreeTiles( DrawTiles( 1 ) );
	m_players.front().m_draw = DrawTile();
	for ( size_t playerI = 1; playerI < m_players.size(); ++playerI )
	{
		m_players[ playerI ].m_hand.AddFreeTiles( DrawTiles( 1 ) );
	}

	return m_players.front().m_draw.value();
}

//------------------------------------------------------------------------------
Vector<Tile> RoundData::DrawTiles
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
Tile RoundData::DrawTile
(
)
{
	Ensure( WallTilesRemaining() >= 1, "Tried to draw more tiles than in wall" );

	Tile drawn = std::move( m_wall.back() );
	m_wall.pop_back();
	return drawn;
}

//------------------------------------------------------------------------------
Tile RoundData::DiscardDrawn
(
)
{
	RoundPlayerData& player = m_players[ ( size_t )m_currentTurn ];
	Tile discarded = player.m_draw.value();
	player.m_discards.emplace_back( discarded );
	player.m_visibleDiscards.emplace_back( discarded );
	player.m_draw.reset();
	return discarded;
}

//------------------------------------------------------------------------------
Tile RoundData::DiscardHandTile
(
	Tile const& i_discard
)
{
	RoundPlayerData& player = m_players[ ( size_t )m_currentTurn ];
	player.m_discards.emplace_back( i_discard );
	player.m_visibleDiscards.emplace_back( i_discard );
	player.m_hand.Discard( i_discard, player.m_draw.value() );
	player.m_draw.reset();
	return i_discard;
}

//------------------------------------------------------------------------------
Tile RoundData::PassCalls
(
)
{
	m_currentTurn = NextPlayer( m_currentTurn, m_players.size() );
	m_players[ ( size_t )m_currentTurn ].m_draw = DrawTile();
	return m_players[ ( size_t )m_currentTurn ].m_draw.value();
}

//------------------------------------------------------------------------------
Tile RoundData::HandKan
(
	Tile const& i_tile
)
{
	RoundPlayerData& player = m_players[ ( size_t )m_currentTurn ];
	player.m_hand.MakeKan( i_tile, std::nullopt );

	Ensure( WallTilesRemaining() >= 1, "Tried to draw more tiles than in wall" );
	Ensure( m_deadWallDrawsRemaining >= 1, "Tried to draw more tiles than in dead wall pool" );

	Tile drawn = std::move( m_wall.front() );
	m_wall.erase( m_wall.begin() );
	return drawn;
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

	return ret;
}

}
