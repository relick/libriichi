#pragma once

#include "playercount.hpp"
#include "seat.hpp"
#include "tile.hpp"
#include "utils.hpp"

namespace Riichi
{

/*

Rough notes on game flow:
// TODO remove this

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

enum class TableStateType
{
	// Game not yet begun
	Setup,

	// No round active
	BetweenRounds,
	GameOver,

	// Round active
	Turn_AI,
	Turn_Player,
	BetweenTurns,
	RobAKanChance, // It's kinda amusing to me that this is a special state, but it makes sense
};

inline constexpr char const* ToString( TableStateType i_type )
{
	constexpr char const* strs[] =
	{
		"Setup",
		"BetweenRounds",
		"GameOver",
		"Turn_AI",
		"Turn_Player",
		"BetweenTurns",
		"RobAKanChance",
	};
	return strs[ ( size_t )i_type ];
}

template<PlayerCount t_PlayerCount>
class Table;

namespace TableStates
{

template<PlayerCount t_PlayerCount>
struct Base
{
	Base( Table<t_PlayerCount>& i_table ) : m_table{ i_table } {}
	std::reference_wrapper<Table<t_PlayerCount>> m_table;
};

template<PlayerCount t_PlayerCount>
struct Setup
	: Base<t_PlayerCount>
{
	using Base<t_PlayerCount>::Base;

	void StartGame() const;
};

template<PlayerCount t_PlayerCount>
struct BetweenRounds
	: Base<t_PlayerCount>
{
	using Base<t_PlayerCount>::Base;

	void StartRound() const;
};

template<PlayerCount t_PlayerCount>
struct GameOver
	: Base<t_PlayerCount>
{
	using Base<t_PlayerCount>::Base;

	// No options
};

template<PlayerCount t_PlayerCount>
struct Turn_AI
	: Base<t_PlayerCount>
{
	using Base<t_PlayerCount>::Base;

	// TODO data
	Seat Seat() const { return Seat::East; }

	void Discard() const;
};

template<PlayerCount t_PlayerCount>
struct Turn_Player
	: Base<t_PlayerCount>
{
	using Base<t_PlayerCount>::Base;

	// TODO data
	Seat Seat() const { return Seat::East; }

	// TODO other options
	void Discard( Tile const& i_tile ) const;
};

template<PlayerCount t_PlayerCount>
struct BetweenTurns
	: Base<t_PlayerCount>
{
	using Base<t_PlayerCount>::Base;

	// TODO other options
	void Pass() const;
};

template<PlayerCount t_PlayerCount>
struct RobAKanChance
	: Base<t_PlayerCount>
{
	using Base<t_PlayerCount>::Base;

	// TODO other options
	void Pass() const;
};

}

template<PlayerCount t_PlayerCount>
using TableState = Utils::NamedVariant<
	TableStateType,

	TableStates::Setup<t_PlayerCount>,
	TableStates::BetweenRounds<t_PlayerCount>,
	TableStates::GameOver<t_PlayerCount>,
	TableStates::Turn_AI<t_PlayerCount>,
	TableStates::Turn_Player<t_PlayerCount>,
	TableStates::BetweenTurns<t_PlayerCount>,
	TableStates::RobAKanChance<t_PlayerCount>
>;

}

#include "tablestate.ipp"