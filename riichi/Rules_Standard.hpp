#pragma once

#include "Rules.hpp"
#include "Seat.hpp"

namespace Riichi
{

//------------------------------------------------------------------------------
class StandardYonmaCore
	: public Rules
{
	Vector<TileInstance> m_tileSet;
public:
	StandardYonmaCore();

	size_t GetPlayerCount() const override { return 4_Players; }
	Points InitialPoints() const override { return 25'000; }
	Points RiichiBet() const override { return 1'000; }
	Vector<TileInstance> const& Tileset() const override { return m_tileSet; }
	size_t DeadWallDrawsAvailable() const override { return 4u; }

	Pair<Set<TileKind>, Vector<TileInstance>> WaitsWithYaku
	(
		Round const& i_round,
		Seat const& i_playerSeat,
		Hand const& i_hand,
		TileDraw const& i_lastTile,
		bool i_considerForRiichi
	) const override;

	HandScore CalculateBasicPoints
	(
		Round const& i_round,
		Seat const& i_playerSeat,
		Hand const& i_hand,
		TileDraw const& i_lastTile
	) const override;

	bool NoMoreRoundsCore( Table const& i_table, Round const& i_previousRound, Seat i_gameLength ) const;
	bool RepeatRound( Round const& i_previousRound ) const override;
	bool ShouldAddHonba( Round const& i_previousRound ) const override;

	Pair<Points, Points> PotPoints( size_t i_honbaSticks, size_t i_riichiSticks, bool i_isTsumo, size_t i_winners ) const override;
	Pair<Points, Points> PointsFromEachPlayerTsumo( Points i_basicPoints, bool i_isDealer ) const override;
	Points PointsFromPlayerRon( Points i_basicPoints, bool i_isDealer ) const override;
	Pair<Points, Points> PointsEachPlayerInTenpaiDraw( size_t i_playersInTenpai ) const override;

private:
	static Points RoundTo100( Points i_finalPoints );
};

//------------------------------------------------------------------------------
template<Seat t_GameLength>
class StandardYonma
	: public StandardYonmaCore
{
public:
	using StandardYonmaCore::StandardYonmaCore;

	bool NoMoreRounds( Table const& i_table, Round const& i_previousRound ) const override;
};

}

#include "Rules_Standard.inl"
