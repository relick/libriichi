#pragma once

#include "Declare.hpp"
#include "Hand.hpp"
#include "NamedUnion.hpp"
#include "PlayerCount.hpp"
#include "Seat.hpp"
#include "TableEvent.hpp"
#include "Tile.hpp"

namespace Riichi
{

//------------------------------------------------------------------------------
enum class TableStateType : EnumValueType
{
	// Game not yet begun
	Setup,

	// No round active
	BetweenRounds,
	GameOver,

	// Round active
	Turn_AI,
	Turn_User,
	BetweenTurns,
	RonAKanChance, // It's kinda amusing to me that this is a special state, but it makes sense
};

//------------------------------------------------------------------------------
inline constexpr char const* ToString( TableStateType i_type )
{
	constexpr char const* strs[] =
	{
		"Setup",
		"BetweenRounds",
		"GameOver",
		"Turn_AI",
		"Turn_User",
		"BetweenTurns",
		"RonAKanChance",
	};
	return strs[ ( size_t )i_type ];
}

//------------------------------------------------------------------------------
namespace TableStates
{

//------------------------------------------------------------------------------
struct Base
{
	Base( Table& i_table ) : m_table{ i_table } {}
protected:
	std::reference_wrapper<Table> m_table;
};

//------------------------------------------------------------------------------
// Common function for transitioning to a turn
//------------------------------------------------------------------------------
struct BetweenTurnsBase
	: Base
{
	using Base::Base;

protected:
	void TransitionToTurn( Option<TileDraw> const& i_tileDraw, TableEvent&& i_tableEvent ) const;
	void HandleRon( SeatSet const& i_winners, TileDraw const& i_tileDraw ) const;
};

//------------------------------------------------------------------------------
struct Setup
	: Base
{
	using Base::Base;

	void StartGame() const;
};

//------------------------------------------------------------------------------
struct BetweenRounds
	: BetweenTurnsBase
{
	using BetweenTurnsBase::BetweenTurnsBase;

	void StartRound() const;
};

//------------------------------------------------------------------------------
struct GameOver
	: Base
{
	using Base::Base;

	// Nothing. Game over.
};

//------------------------------------------------------------------------------
struct BaseTurn
	: Base
{
	BaseTurn( Table& i_table, Seat i_seat );

	Hand const& GetHand() const;
	Option<Tile> GetDrawnTile() const;
	Seat GetSeat() const { return m_seat; }

protected:
	void TransitionToBetweenTurns( Tile const& i_discardedTile, TableEvent&& i_tableEvent ) const;

private:
	Seat m_seat;
};

//------------------------------------------------------------------------------
struct Turn_AI
	: BaseTurn
{
	Turn_AI( Table& i_table, Seat i_seat );

	void MakeDecision() const;
};

//------------------------------------------------------------------------------
struct Turn_User
	: BaseTurn
{
	Turn_User( Table& i_table, Seat i_seat, bool i_canTsumo, Vector<Tile> i_riichiDiscards, bool i_isRiichi, Vector<Hand::DrawKanResult> i_kanOptions );

	bool CanTsumo() const { return m_canTsumo; }
	bool CanRiichi() const { return !m_riichiDiscards.empty(); }
	Vector<Tile> const& RiichiOptions() const { return m_riichiDiscards; }
	bool IsRiichi() const { return m_isRiichi; } // if true, only valid options are tsumo and discard
	bool CanKan() const { return !m_kanOptions.empty(); }
	Vector<Hand::DrawKanResult> const& KanOptions() const { return m_kanOptions; }

	void Tsumo() const;
	void Discard( Option<Tile> const& i_handTileToDiscard ) const; // nullopt will discard drawn tile
	void Riichi( Option<Tile> const& i_handTileToDiscard ) const; // nullopt will discard drawn tile
	void Kan( Tile const& i_tile ) const; // Will meld the 4 matching tiles if a closed kan

private:
	bool m_canTsumo;
	Vector<Tile> m_riichiDiscards;
	bool m_isRiichi;
	Vector<Hand::DrawKanResult> m_kanOptions;
};

//------------------------------------------------------------------------------
struct BetweenTurns
	: BetweenTurnsBase
{
	BetweenTurns( Table& i_table, TileDraw i_discardedTile, Pair<Seat, Vector<Pair<Tile, Tile>>> i_canChi, SeatSet i_canPon, SeatSet i_canKan, SeatSet i_canRon );

	// TODO-AI indication about AI intent (to allow AI to jump in before user, depending on game implementation)

	// TODO-DEBT: chi option data structure sucks
	Pair<Seat, Vector<Pair<Tile, Tile>>> const& CanChi() const { return m_canChi; }
	SeatSet const& CanPon() const { return m_canPon; }
	SeatSet const& CanKan() const { return m_canKan; }
	SeatSet const& CanRon() const { return m_canRon; }

	// If any calls are made by AI, they will only happen in UserPass or UserRon
	void UserPass() const;
	void UserChi( Seat i_user, Pair<Tile, Tile> const& i_option ) const;
	void UserPon( Seat i_user ) const;
	void UserKan( Seat i_user ) const;
	void UserRon( SeatSet const& i_users ) const;

private:
	TileDraw m_discardedTile;
	Pair<Seat, Vector<Pair<Tile, Tile>>> m_canChi;
	SeatSet m_canPon;
	SeatSet m_canKan;
	SeatSet m_canRon;
};

//------------------------------------------------------------------------------
struct RonAKanChance
	: BetweenTurnsBase
{
	RonAKanChance( Table& i_table, TileDraw i_kanTile, SeatSet i_canRon );

	SeatSet const& CanRon() const { return m_canRon; }
	
	void Pass() const;
	void Ron( SeatSet const& i_players ) const;

private:
	TileDraw m_kanTile;
	SeatSet m_canRon;
};

}

//------------------------------------------------------------------------------
using TableState = NamedUnion<
	TableStateType,

	TableStates::Setup,
	TableStates::BetweenRounds,
	TableStates::GameOver,
	TableStates::Turn_AI,
	TableStates::Turn_User,
	TableStates::BetweenTurns,
	TableStates::RonAKanChance
>;

}
