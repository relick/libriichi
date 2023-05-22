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
		size_t m_playerIndex; // index to Table's player list
		std::optional<size_t> m_riichiDiscardTile;
		std::vector<Tile> m_discards;
		Hand m_hand;
		std::optional<Tile> m_draw; // Currently drawn tile

		explicit RoundPlayerData( size_t i_playerI ) : m_playerIndex( i_playerI ) {}
	};
	size_t m_initialPlayerIndex{ SIZE_MAX };
	std::vector<RoundPlayerData> m_players; // Sorted in seat order

	// Wall is ordered in columns, clockwise from the dealer's right corner (initially), and reversed
	// i.e. drawing is pop_back and dead wall is the 14 tiles at the start of the vector
	// This makes it easy to draw, but complicates the maths a bit for wall breaking
	std::vector<Tile> m_wall;
	size_t m_breakPointFromDealerRight{ 0 }; // Unused other than for indicating to the user where to put the wall visuals
	size_t m_deadWallSize{ 0 };
	size_t m_deadWallDrawsRemaining{ 0 }; // This will decrement as dead wall draws are made

	Seat m_roundWind{ Seat::East };
	Seat m_currentTurn{ Seat::East };

public:
	// const interface

	Seat Wind() const;
	Seat CurrentTurn() const;
	bool IsDealer( Seat i_player ) const;
	bool CalledRiichi( Seat i_player ) const;
	bool CalledDoubleRiichi( Seat i_player ) const;
	bool RiichiIppatsuValid( Seat i_player ) const;
	std::vector<Tile> const& Discards( Seat i_player ) const;
	Hand const& Hand( Seat i_player ) const;
	std::optional<Tile> const& DrawnTile( Seat i_player ) const;
	size_t WallTilesRemaining() const;
	bool CallsMade() const;
	Player const& GetPlayer( Seat i_player, Table const& i_table ) const;

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

	void BreakWall( ShuffleRNG& i_shuffleRNG );
	Tile DealHands();

	std::vector<Tile> DrawTiles( size_t i_num );
	Tile DrawTile();

	// Player turn actions
	Tile DiscardDrawn();
	Tile PassCalls(); // draws for next player
};

}