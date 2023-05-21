#include "RoundData.hpp"

#include "Player.hpp"
#include "Rules.hpp"

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
size_t RoundData::WallSize
(
)	const
{
	return m_wall.size();
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
RoundData::RoundData
(
	Seat i_roundWind,
	std::vector<Player> const& i_players,
	Rules const& i_rules,
	ShuffleRNG& i_shuffleRNG
)
{
	// Randomly determine initial seats
	std::vector<size_t> playerIndices( i_players.size() );
	std::ranges::iota( playerIndices, 0 );
	std::ranges::shuffle( playerIndices, i_shuffleRNG );
	m_players.reserve( i_players.size() );
	for ( size_t index : playerIndices )
	{
		m_players.emplace_back( i_players[ index ] );
	}

	// TODO
	//m_wall = i_rules.AllTiles();
}

//------------------------------------------------------------------------------
RoundData::RoundData
(
	RoundData const& i_lastRound,
	Rules const& i_rules,
	ShuffleRNG& i_shuffleRNG
)
{
	// TODO
	//m_players = std::ranges::rotate_copy()
}

}