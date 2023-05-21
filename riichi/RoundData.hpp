#pragma once

#include "Declare.hpp"
#include "Hand.hpp"
#include "Random.hpp"
#include "Seat.hpp"

#include <optional>
#include <vector>

namespace Riichi
{

//------------------------------------------------------------------------------
class RoundData
{
	struct RoundPlayerData
	{
		std::reference_wrapper<Player const> m_player;
		std::optional<size_t> m_riichiDiscardTile;
		std::vector<Tile> m_discards;
		Hand m_hand;

		RoundPlayerData( Player const& i_p ) : m_player( i_p ) {}
	};
	std::vector<RoundPlayerData> m_players; // Sorted in seat order
	std::vector<Tile> m_wall;
	std::vector<Tile> m_deadWall;
	Seat m_roundWind;

public:
	// const interface

	Seat Wind() const;
	bool IsDealer( Seat i_player ) const;
	bool CalledRiichi( Seat i_player ) const;
	bool CalledDoubleRiichi( Seat i_player ) const;
	bool RiichiIppatsuValid( Seat i_player ) const;
	std::vector<Tile> const& Discards( Seat i_player ) const;
	size_t WallSize() const;
	bool CallsMade() const;

public:
	// non-const interface, only accessible by table states
	RoundData
	(
		Seat i_roundWind,
		std::vector<Player> const& i_players,
		Rules const& i_rules,
		ShuffleRNG& i_shuffleRNG
	);

	RoundData
	(
		RoundData const& i_lastRound,
		Rules const& i_rules,
		ShuffleRNG& i_shuffleRNG
	);
};

}