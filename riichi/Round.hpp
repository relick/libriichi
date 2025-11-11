#pragma once

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
public:
	// These properties are fixed from the wall break, or earlier:
	SeatSet Seats() const;
	Seat Wind() const;
	size_t BreakPointFromDealerRight() const;

	// These properties change as the round progresses:
	Seat CurrentTurn() const;
	bool NextPlayerIsInitial() const;
	size_t HonbaSticks() const;
	size_t RiichiSticks() const;
	size_t WallTilesRemaining() const;
	size_t DeadWallDrawsRemaining() const;
	bool CallsMade() const;
	Vector<TileKind> GetDoraTiles( bool i_includeUradora ) const;
	Vector<TileInstance> GetDoraIndicatorTiles( bool i_includeUradora ) const;

	// These properties are set when the round ends:
	bool AnyWinners() const;
	bool AnyFinishedInTenpai() const;

	// These properties are per-player, and change as the round progresses:
	Hand const& CurrentHand( Seat i_player ) const;
	Option<TileDraw> const& CurrentTileDraw( Seat i_player ) const;
	Vector<TileInstance> const& Discards( Seat i_player ) const;
	Pair<Vector<TileInstance> const&, Option<size_t>> VisibleDiscards( Seat i_player ) const; // Discards still in front of the player, including index for a riichi tile

	bool CalledRiichi( Seat i_player ) const;
	bool CalledDoubleRiichi( Seat i_player ) const;
	bool WaitingToPayRiichiBet( Seat i_player ) const;
	bool RiichiIppatsuValid( Seat i_player ) const;
	bool Furiten( Seat i_player, Set<TileKind> const& i_waits ) const;

	// These properties are per-player, and are set when the round ends:
	bool IsWinner( Seat i_player ) const;
	bool FinishedInTenpai( Seat i_player ) const;
	struct WinScores
	{
		HandScore m_handScore;
		FinalScore m_finalScore;
	};
	Option<WinScores> const& WinnerScores( Seat i_player ) const;
	Points EndOfRoundTablePayment( Seat i_player ) const;

	// Convert back from round seats to table players
	Player const& GetPlayer( Seat i_player, Table const& i_table ) const;
	PlayerID GetPlayerID( Seat i_player ) const;
	Seat GetSeat( PlayerID i_playerID ) const;

public: // interface for the table states, which mutate the round state

	// Start a round
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

	// Turn actions
	TileInstance Discard( Option<TileInstance> const& i_handTileToDiscard ); // returns discarded tile
	TileInstance Riichi( Option<TileInstance> const& i_handTileToDiscard ); // returns discarded tile
	Hand::KanResult HandKan( Option<TileInstance> const& i_handTileToKan ); // returns if kan was upgraded pon
	TileDraw PassCalls( SeatSet const& i_couldRon ); // draws for next player, returns draw
	TileDraw HandKanRonPass(); // returns dead wall draw ☠
	Pair<Seat, TileInstance> Chi( Seat i_caller, Pair<TileInstance, TileInstance> const& i_meldTiles ); // returns called tile and called from
	Pair<Seat, TileInstance> Pon( Seat i_caller ); // returns called tile and called from
	Pair<TileDraw, Pair<Seat, TileInstance>> DiscardKan( Seat i_caller ); // returns dead wall draw ☠, called tile and called from
	void RiichiBetPaid( Seat i_player ); // adds 1 riichi stick to the pot, and clears the payment-pending status of the given player

	// Round finished actions
	TileInstance AddWinner( Seat i_player, HandScore i_handScore, FinalScore i_finalScore ); // returns winning tile
	void AddFinishedInTenpai( Seat i_player );

	// Apply seat-based payments to players at a table
	void ApplyPayments( TablePayments const& i_payments, Table& io_table );
	void ApplyEndOfRoundPayments( TablePayments const& i_payments, Table& io_table );

private:
	struct PlayerData
	{
		PlayerID m_playerID; // index to Table's player list

		Hand m_hand;
		Option<TileDraw> m_draw; // Currently drawn tile
		Vector<TileInstance> m_discards;
		Vector<TileInstance> m_visibleDiscards; // Called tiles removed from this list

		struct Riichi
		{
			size_t m_sidewaysDiscardIndex; // Index into visible discards for the tile that should be placed sideways
			bool m_waitingToPayBet{ true };
			bool m_ippatsuValid{ true };
		};
		Option<Riichi> m_riichi;

		bool m_tempFuriten{ false }; // Passing up on a ron sets this true until next turn, or until the end of the game if riichi'd

		struct EndOfRound
		{
			Option<WinScores> m_winScores;
			Points m_tablePayment;
			bool m_finishedInTenpai{ false };
		};
		Option<EndOfRound> m_endOfRound;

		explicit PlayerData( PlayerID i_playerID ) : m_playerID( i_playerID ) {}

		void UpdateForTurn();
	};
	PlayerID m_initialPlayerID;
	Vector<PlayerData> m_players; // Sorted in seat order

	// Wall is ordered in columns, clockwise from the dealer's right corner (initially), and reversed
	// i.e. drawing is pop_back and dead wall is the 14 tiles at the start of the vector
	// This makes it easy to draw, but complicates the maths a bit for wall breaking
	Vector<TileInstance> m_wall;
	size_t m_breakPointFromDealerRight{ 0 }; // Unused other than for indicating to the user where to put the wall visuals
	size_t m_deadWallSize{ 0 };
	size_t m_deadWallDrawsRemaining{ 0 }; // This will decrement as dead wall draws are made
	size_t m_doraCount{ 1 }; // This will increment as dead wall draws are made // TODO-RULES: kandora timing can change depending on ruleset

	Seat m_roundWind{ Seat::East };
	Seat m_currentTurn{ Seat::East };

	size_t m_honbaSticks{ 0 };
	size_t m_riichiSticks{ 0 };

private:
	PlayerData const& Player( Seat i_player ) const { return m_players[ ( size_t )i_player ]; }
	PlayerData& Player( Seat i_player ) { return m_players[ ( size_t )i_player ]; }
	PlayerData const& CurrentPlayer() const { return Player( CurrentTurn() ); }
	PlayerData& CurrentPlayer() { return Player( CurrentTurn() ); }

	void BreakWall( ShuffleRNG& i_shuffleRNG );
	Vector<TileInstance> DealTiles( size_t i_num );
	TileDraw SelfDraw();
	TileDraw DeadWallDraw();

	// Applies some common ops that need doing every time a new player starts their turn (to discard)
	PlayerData& StartTurn( Seat i_player, bool i_callMade );
};

}