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

class Table;

namespace TableStates
{

struct Base
{
	Base( Table& i_table ) : m_table{ i_table } {}
	std::reference_wrapper<Table> m_table;
};

struct Setup
	: Base
{
	using Base::Base;

	void StartGame() const;
};

struct BetweenRounds
	: Base
{
	using Base::Base;

	void StartRound() const;
};

struct GameOver
	: Base
{
	using Base::Base;

	// No options
};

struct Turn_AI
	: Base
{
	using Base::Base;

	// TODO data
	Seat Seat() const { return Seat::East; }

	void Discard() const;
};

struct Turn_Player
	: Base
{
	using Base::Base;

	// TODO data
	Seat Seat() const { return Seat::East; }

	// TODO other options
	void Discard( Tile const& i_tile ) const;
};

struct BetweenTurns
	: Base
{
	using Base::Base;

	// TODO other options
	void Pass() const;
};

struct RobAKanChance
	: Base
{
	using Base::Base;

	// TODO other options
	void Pass() const;
};

}

using TableState = Utils::NamedVariant<
	TableStateType,

	TableStates::Setup,
	TableStates::BetweenRounds,
	TableStates::GameOver,
	TableStates::Turn_AI,
	TableStates::Turn_Player,
	TableStates::BetweenTurns,
	TableStates::RobAKanChance
>;

}

#include "tablestate.ipp"