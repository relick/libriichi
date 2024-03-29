﻿#pragma once

#include "Containers.hpp"
#include "Declare.hpp"
#include "Hand.hpp"
#include "Player.hpp"
#include "Random.hpp"
#include "Rules.hpp"
#include "Seat.hpp"

namespace Riichi
{

//------------------------------------------------------------------------------
class Round
{
	struct PlayerData
	{
		PlayerID m_playerID; // index to Table's player list
		Option<size_t> m_riichiDiscardTile;
		bool m_riichiIppatsuValid{ false };
		Vector<Tile> m_discards;
		Vector<Tile> m_visibleDiscards; // Called tiles removed from this list
		bool m_tempFuriten{ false }; // Passing up on a ron sets this true until next turn, or until the end of the game if riichi'd
		Hand m_hand;
		Option<TileDraw> m_draw; // Currently drawn tile
		Option<HandScore> m_winningScore;
		bool m_finishedInTenpai{ false };

		explicit PlayerData( PlayerID i_playerID ) : m_playerID( i_playerID ) {}

		void UpdateForTurn();
	};
	PlayerID m_initialPlayerID;
	Vector<PlayerData> m_players; // Sorted in seat order

	// Wall is ordered in columns, clockwise from the dealer's right corner (initially), and reversed
	// i.e. drawing is pop_back and dead wall is the 14 tiles at the start of the vector
	// This makes it easy to draw, but complicates the maths a bit for wall breaking
	Vector<Tile> m_wall;
	size_t m_breakPointFromDealerRight{ 0 }; // Unused other than for indicating to the user where to put the wall visuals
	size_t m_deadWallSize{ 0 };
	size_t m_deadWallDrawsRemaining{ 0 }; // This will decrement as dead wall draws are made
	size_t m_doraCount{ 1 }; // This will increment as dead wall draws are made // TODO-RULES: kandora timing can change depending on ruleset

	Seat m_roundWind{ Seat::East };
	Seat m_currentTurn{ Seat::East };

	size_t m_honbaSticks{ 0 };
	size_t m_riichiSticks{ 0 };

public:
	// const interface

	SeatSet Seats() const;
	Seat Wind() const;
	Seat CurrentTurn() const;
	bool IsDealer( Seat i_player ) const;
	bool CalledRiichi( Seat i_player ) const;
	Option<size_t> RiichiTileIndex( Seat i_player ) const;
	bool CalledDoubleRiichi( Seat i_player ) const;
	bool RiichiIppatsuValid( Seat i_player ) const;
	bool Furiten( Seat i_player, Set<Tile> const& i_waits ) const;
	Vector<Tile> const& Discards( Seat i_player ) const;
	Vector<Tile> const& VisibleDiscards( Seat i_player ) const;
	Hand const& GetHand( Seat i_player ) const;
	Option<TileDraw> const& DrawnTile( Seat i_player ) const;
	bool IsWinner( Seat i_player ) const;
	Option<HandScore> const& WinnerScore( Seat i_player ) const;
	bool FinishedInTenpai( Seat i_player ) const;
	size_t WallTilesRemaining() const;
	bool CallsMade() const;
	Player const& GetPlayer( Seat i_player, Table const& i_table ) const;
	PlayerID GetPlayerID( Seat i_player ) const;
	Seat GetSeat( PlayerID i_playerID ) const;
	bool AnyWinners() const;
	bool AnyFinishedInTenpai() const;
	bool NextPlayerIsInitial() const;
	// If indicated value true -> returns the tiles that would score points, false -> returns the actual tiles of the wall
	Vector<Tile> GatherDoraTiles( bool i_indicatedValue ) const;
	Vector<Tile> GatherUradoraTiles( bool i_indicatedValue ) const;
	size_t HonbaSticks() const;
	size_t RiichiSticks() const;

	size_t BreakPointFromDealerRight() const;
	size_t DeadWallDrawsRemaining() const;

public:
	// non-const interface, only accessible by table states
	Round
	(
		Seat i_roundWind,
		Vector<PlayerID> const& i_playerIDs,
		Rules const& i_rules,
		ShuffleRNG& i_shuffleRNG
	);

	Round
	(
		Table const& i_table,
		Round const& i_previousRound,
		Rules const& i_rules,
		ShuffleRNG& i_shuffleRNG
	);

	TileDraw DealHands();

	// Player turn actions
	Tile Discard( Option<Tile> const& i_handTileToDiscard ); // returns discarded tile
	Tile Riichi( Option<Tile> const& i_handTileToDiscard ); // returns discarded tile
	Hand::KanResult HandKan( Option<Tile> const& i_handTileToKan ); // returns if kan was upgraded pon
	TileDraw PassCalls( SeatSet const& i_couldRon ); // draws for next player, returns draw
	TileDraw HandKanRonPass(); // returns dead wall draw ☠
	Pair<Seat, Tile> Chi( Seat i_caller, Pair<Tile, Tile> const& i_meldTiles ); // returns called tile and called from
	Pair<Seat, Tile> Pon( Seat i_caller ); // returns called tile and called from
	Pair<TileDraw, Pair<Seat, Tile>> DiscardKan( Seat i_caller ); // returns dead wall draw ☠, called tile and called from

	Tile AddWinner( Seat i_player, HandScore const& i_score ); // returns winning tile
	void AddFinishedInTenpai( Seat i_player );

private:
	void BreakWall( ShuffleRNG& i_shuffleRNG );
	Vector<Tile> DealTiles( size_t i_num );
	TileDraw SelfDraw();
	TileDraw DeadWallDraw();
};

}