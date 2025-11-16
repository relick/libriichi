#pragma once

#include "Containers.hpp"
#include "Declare.hpp"
#include "HandInterpreter.hpp"
#include "PlayerCount.hpp"
#include "Seat.hpp"
#include "Tile.hpp"
#include "Utils.hpp"
#include "Yaku.hpp"

#include <memory>
#include <numeric>

namespace Riichi
{

//------------------------------------------------------------------------------
struct HandScore
{
	// Each of these values is usable however a ruleset wishes. None of them actually affect table points directly.
	Points m_basicPoints;
	Points m_fu;
	Vector<Pair<char const*, HanValue>> m_yaku;

	Points FuTotal() const { return m_fu; }
	Han HanTotal() const { return std::accumulate( m_yaku.begin(), m_yaku.end(), 0, []( int a, auto const& yaku ) { return a + yaku.second.Get(); } ); }
};
struct FinalScore
{
	Points m_fromPlayers;
	Points m_fromPot;

	Points Total() const { return m_fromPlayers + m_fromPot; }
};
struct TablePayments
{
	Utils::EnumArray<Points, Seats> m_pointsPerSeat;

	TablePayments()
		: m_pointsPerSeat{0, 0, 0, 0}
	{}

	TablePayments& operator+=( TablePayments const& other )
	{
		for ( Seat seat : Seats{} )
		{
			m_pointsPerSeat[ seat ] += other.m_pointsPerSeat[ seat ];
		}
		return *this;
	}
	friend TablePayments operator+( TablePayments const& a, TablePayments const& b )
	{
		TablePayments total = a;
		return total += b;
	}
};

//------------------------------------------------------------------------------
struct Rules
{
	virtual ~Rules() = default;

	// Main settings
	virtual size_t GetPlayerCount() const = 0;
	virtual Points InitialPoints() const = 0;
	virtual Points RiichiBetPoints() const = 0;
	virtual Vector<TileInstance> const& Tileset() const = 0;
	// Not sure there's a reason to vary this, but it's a nice place to put it
	// Dead wall size derived from this, and kan call maximum is treated as equal
	virtual size_t DeadWallDrawsAvailable() const = 0;
	size_t DeadWallSize() const { return ( 1 + DeadWallDrawsAvailable() ) * 2 + DeadWallDrawsAvailable(); }
	virtual bool HasPermissionToRiichi( Seat i_player, Points i_currentPoints ) const = 0; // mainly to vary whether players with negative points are allowed to riichi

	// Hand evaluation
	// Returns valid waits for a win, and valid discards for riichi
	virtual Pair<Set<TileKind>, Vector<TileInstance>> WaitsWithYaku
	(
		Round const& i_round,
		Seat const& i_playerSeat,
		Hand const& i_hand,
		TileDraw const& i_lastTile,
		bool i_considerForRiichi
	) const = 0;

	// Returns the best score of a completed hand
	virtual HandScore CalculateBasicPoints
	(
		Round const& i_round,
		Seat const& i_playerSeat,
		Hand const& i_hand,
		TileDraw const& i_lastTile
	) const = 0;

	// Round control
	virtual bool NoMoreRounds( Table const& i_table, Round const& i_previousRound ) const = 0;
	virtual bool RepeatRound( Round const& i_previousRound ) const = 0;
	virtual bool ShouldAddHonba( Round const& i_previousRound ) const = 0;
	// TODO-RULES: special draw actions (9-honour hand, 4 discarded winds, 5 kantsu)

	// Score transfers, paid as specified
	virtual TablePayments RiichiBetPayments( Seat i_riichiPlayer ) const // paid when the next player takes their turn following a riichi call
	{
		// default implementation provided, assumes that rule sets usually only want to
		// override the value of a bet and not how it gets applied
		TablePayments bet;
		bet.m_pointsPerSeat[ i_riichiPlayer ] = -RiichiBetPoints();
		return bet;
	}
	virtual TablePayments HonbaPotPayments( size_t i_honbaSticks, SeatSet const& i_winners, Option<Seat> i_ronLoser ) const = 0; // paid on any round end (i.e. may be no winners)
	virtual TablePayments RiichiBetPotPayments( size_t i_riichiSticks, SeatSet const& i_winners, Option<Seat> i_ronLoser ) const = 0; // paid on any round end (i.e. may be no winners)
	virtual TablePayments TsumoPayments( HandScore const& i_handScore, Seat i_winner ) const = 0; // paid on tsumo win
	virtual TablePayments RonPayments( HandScore const& i_handScore, Seat i_winner, Seat i_loser ) const = 0; // paid on ron win
	virtual TablePayments ExhaustiveDrawPayments( SeatSet const& i_playersInTenpai ) const = 0; // paid on exhaustive draw

	// Common to all rulesets
	inline auto Interpreters() const { return m_interpreters | DerefConst; }
	inline auto YakuEvaluators() const { return m_yakuEvaluators | DerefConst; }

protected:
	template<std::derived_from<HandInterpreter> T_Interpreter, typename... Args>
		requires std::constructible_from<T_Interpreter, Args...>
	void AddInterpreter( Args&&... i_args )
	{
		m_interpreters.push_back( std::make_unique<T_Interpreter>( std::forward<Args>( i_args )... ) );
	}

	template<std::derived_from<YakuEvaluator> T_Evaluator, typename... Args>
		requires std::constructible_from<T_Evaluator, Args...>
	void AddYakuEvaluator( Args&&... i_args )
	{
		m_yakuEvaluators.push_back( std::make_unique<T_Evaluator>( std::forward<Args>( i_args )... ) );
	}

private:
	Vector<std::unique_ptr<HandInterpreter>> m_interpreters;
	Vector<std::unique_ptr<YakuEvaluator>> m_yakuEvaluators;
};

}
