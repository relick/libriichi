﻿#pragma once

#include "Containers.hpp"
#include "Declare.hpp"
#include "Hand.hpp"
#include "Random.hpp"
#include "Seat.hpp"

namespace Riichi
{

//------------------------------------------------------------------------------
class RoundData
{
	struct RoundPlayerData
	{
		size_t m_playerIndex; // index to Table's player list
		Option<size_t> m_riichiDiscardTile;
		Vector<Tile> m_discards;
		Vector<Tile> m_visibleDiscards; // Called tiles removed from this list
		bool m_tempFuriten{ false }; // Passing up on a ron sets this true until next turn, or until the end of the game if riichi'd
		Hand m_hand;
		Option<Tile> m_draw; // Currently drawn tile

		explicit RoundPlayerData( size_t i_playerI ) : m_playerIndex( i_playerI ) {}

		void UpdateForTurn();
	};
	size_t m_initialPlayerIndex{ SIZE_MAX };
	Vector<RoundPlayerData> m_players; // Sorted in seat order

	// Wall is ordered in columns, clockwise from the dealer's right corner (initially), and reversed
	// i.e. drawing is pop_back and dead wall is the 14 tiles at the start of the vector
	// This makes it easy to draw, but complicates the maths a bit for wall breaking
	Vector<Tile> m_wall;
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
	bool Furiten( Seat i_player, Set<Tile> const& i_waits ) const;
	Vector<Tile> const& Discards( Seat i_player ) const;
	Vector<Tile> const& VisibleDiscards( Seat i_player ) const;
	Hand const& GetHand( Seat i_player ) const;
	Option<Tile> const& DrawnTile( Seat i_player ) const;
	size_t WallTilesRemaining() const;
	bool CallsMade() const;
	Player const& GetPlayer( Seat i_player, Table const& i_table ) const;

public:
	// non-const interface, only accessible by table states
	RoundData
	(
		Seat i_roundWind,
		Vector<Player> const& i_players,
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

	Vector<Tile> DrawTiles( size_t i_num );
	Tile DrawTile();

	// Player turn actions
	Tile DiscardDrawn(); // returns discarded tile
	Tile DiscardHandTile( Tile const& i_discard ); // returns discarded tile
	Tile PassCalls( SeatSet const& i_couldRon ); // draws for next player, returns draw
	Tile HandKan( Tile const& i_tile ); // returns dead wall draw ☠
	Pair<Seat, Tile> Chi( Seat i_caller, Pair<Tile, Tile> const& i_meldTiles ); // returns called tile and called from
	Pair<Seat, Tile> Pon( Seat i_caller ); // returns called tile and called from
	Pair<Seat, Tile> DiscardKan( Seat i_caller ); // returns called tile and called from
};

}