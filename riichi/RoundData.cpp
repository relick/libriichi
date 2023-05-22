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
	// TODO
	return false;
}

//------------------------------------------------------------------------------
std::vector<Tile> const& RoundData::Discards
(
	Seat i_player
)	const
{
	return m_players[ ( size_t )i_player ].m_discards;
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
	std::vector<Player> const& i_players,
	Rules const& i_rules,
	ShuffleRNG& i_shuffleRNG
)
	: m_deadWallSize{ i_rules.DeadWallSize() }
	, m_deadWallDrawsRemaining{ i_rules.DeadWallDrawsAvailable() }
{
	// Randomly determine initial seats
	std::vector<size_t> playerIndices( i_players.size() );
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

	// TODO: work out the situations when we should rotate player based on last round win
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

	// Start with turning dTotal into a player index:
	// i = (dTotal - 1) % playerCount
	// As our wall is clockwise but we want to count counter-clockwise, we can flip it around
	// i_2 = (playerCount - i) % playerCount
	// Multiplying this by the number of tiles in the wall per player, we should get the start of the wall
	// It's a bit of an awkward calculation (especially doing mod twice) but simplifications involved floor/mod with negatives which isn't nice in C++ either so...
	size_t const wallIndexCCW = ( m_players.size() - ( ( dTotal - 1 ) % m_players.size() ) ) % m_players.size();
	size_t const perPlayerWall = m_wall.size() / m_players.size();
	size_t const breakingWallStartTile = wallIndexCCW * perPlayerWall;

	// Lastly we just add the dice again
	m_breakPointFromDealerRight = breakingWallStartTile + dTotal;

	// Tada! Put into position and reverse ready for draws
	std::ranges::rotate( m_wall, m_wall.begin() + m_breakPointFromDealerRight );
	std::ranges::reverse( m_wall );
}

//------------------------------------------------------------------------------
void RoundData::DealHands
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

	m_players.front().m_hand.AddFreeTiles( DrawTiles( 2 ) );
	for ( size_t playerI = 1; playerI < m_players.size(); ++playerI )
	{
		m_players[ playerI ].m_hand.AddFreeTiles( DrawTiles( 1 ) );
	}
}

//------------------------------------------------------------------------------
std::vector<Tile> RoundData::DrawTiles
(
	size_t i_num
)
{
	Ensure( WallTilesRemaining() >= i_num, "Tried to draw more tiles than in wall" );

	std::vector<Tile> tiles;
	tiles.reserve( i_num );
	for ( size_t i = 0; i < i_num; ++i )
	{
		tiles.emplace_back( std::move( m_wall.back() ) );
		m_wall.pop_back();
	}
	return tiles;
}

}