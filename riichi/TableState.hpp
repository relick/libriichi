#pragma once

#include "Declare.hpp"
#include "Hand.hpp"
#include "NamedUnion.hpp"
#include "PlayerCount.hpp"
#include "Seat.hpp"
#include "Tile.hpp"

namespace Riichi
{

/*

Rough notes on game flow:
// TODO-MVP remove this

- Start
+ Draw (dealer)

- Draw
+ NoMoreTiles -> GameOver (draw)
+ Tsumo (if hand would win) -> GameOver (tsumo)
+ RiichiDiscard (if hand is 1 away) -> BetweenTurns
+ DoubleRiichiDiscard (if hand is 1 away and double riichi valid) -> BetweenTurns
+ Kan (upgrade) -> RobKanChance
+ Kan (closed) -> DeadWallDraw
+ Discard -> BetweenTurns

- BetweenTurns
+ Pon (other players) -> Draw options
+ Chi (next player) -> Draw options
+ Ron (other players) -> GameOver (ron)
+ Pass -> Draw (next player)

- RobKanChance
+ Ron (other players)
+ Pass -> DeadWallDraw

- DeadWallDraw -> Draw options

- GameOver

State is any of the -
An event is any transition, i.e. any +

In other words! Game FSM is quite Simple!!

*/

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
struct Setup
	: Base
{
	using Base::Base;

	void StartGame() const;
};

//------------------------------------------------------------------------------
struct BetweenRounds
	: Base
{
	using Base::Base;

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
	Seat GetSeat() const { return m_seat; }

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
	Turn_User( Table& i_table, Seat i_seat, bool i_canTsumo, bool i_canRiichi, Vector<Hand::DrawKanResult> i_kanOptions );

	bool CanTsumo() const { return m_canTsumo; }
	bool CanRiichi() const { return m_canTsumo; }
	bool CanKan() const { return !m_kanOptions.empty(); }

	void Tsumo() const;
	void Discard( Tile const& i_tile ) const;
	void Riichi( Tile const& i_tile ) const;
	void Kan( Tile const& i_tile ) const; // Will meld the 4 matching tiles if a closed kan

private:
	bool m_canTsumo;
	bool m_canRiichi;
	Vector<Hand::DrawKanResult> m_kanOptions;
};

//------------------------------------------------------------------------------
struct BetweenTurns
	: Base
{
	BetweenTurns( Table& i_table, Pair<Seat, Vector<Pair<Tile, Tile>>> i_canChi, SeatSet i_canPon, SeatSet i_canKan, SeatSet i_canRon );

	// TODO-AI indication about AI intent (to allow AI to jump in before user, depending on game implementation)

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
	Pair<Seat, Vector<Pair<Tile, Tile>>> m_canChi;
	SeatSet m_canPon;
	SeatSet m_canKan;
	SeatSet m_canRon;
};

//------------------------------------------------------------------------------
struct RonAKanChance
	: Base
{
	RonAKanChance( Table& i_table, SeatSet i_canRon );

	SeatSet const& CanRon() const { return m_canRon; }
	
	void Pass() const;
	void Ron( SeatSet const& i_players ) const;

private:
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
