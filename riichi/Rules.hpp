#pragma once

#include "Containers.hpp"
#include "Declare.hpp"
#include "HandInterpreter.hpp"
#include "PlayerCount.hpp"
#include "Tile.hpp"
#include "Yaku.hpp"

#include <memory>

namespace Riichi
{

//------------------------------------------------------------------------------
using HandScore = Pair<Points, Vector<Pair<char const*, HanValue>>>;

//------------------------------------------------------------------------------
struct Rules
{
	virtual ~Rules() = default;

	virtual PlayerCount GetPlayerCount() const = 0;
	virtual Points InitialPoints() const = 0;
	virtual Vector<Tile> const& Tileset() const = 0;
	// Not sure there's a reason to vary this, but it's a nice place to put it
	// Dead wall size derived from this, and kan call maximum is treated as equal
	virtual size_t DeadWallDrawsAvailable() const = 0;
	size_t DeadWallSize() const { return ( 1 + DeadWallDrawsAvailable() ) * 2 + DeadWallDrawsAvailable(); }
	virtual Seat LastRound() const = 0;

	// Returns valid waits for a win, and a bool for whether a riichi is allowed
	virtual Pair<Set<Tile>, bool> WaitsWithYaku
	(
		RoundData const& i_round,
		Seat const& i_playerSeat,
		Hand const& i_hand,
		TileDraw const& i_lastTile,
		bool i_considerForRiichi
	) const = 0;

	virtual HandScore CalculateBasicPoints
	(
		RoundData const& i_round,
		Seat const& i_playerSeat,
		Hand const& i_hand,
		TileDraw const& i_lastTile
	) const = 0;

	virtual Pair<Points, Points> PointsFromEachPlayerTsumo( Points i_basicPoints, bool i_isDealer ) const = 0;
	virtual Points PointsFromPlayerRon( Points i_basicPoints, bool i_isDealer ) const = 0;
	virtual Pair<Points, Points> PointsEachPlayerInTenpaiDraw( size_t i_playersInTenpai ) const = 0;

	template<typename T_Visitor>
	void VisitInterpreters( T_Visitor&& i_visitor ) const;

	template<typename T_Visitor>
	void VisitYaku( T_Visitor&& i_visitor ) const;

protected:
	Vector<std::unique_ptr<HandInterpreter>> m_interpreters;
	Vector<std::unique_ptr<Yaku>> m_yaku;
};

}

#include "Rules.inl"
