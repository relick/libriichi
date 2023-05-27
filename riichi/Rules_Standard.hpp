#pragma once

#include "Rules.hpp"

namespace Riichi
{

//------------------------------------------------------------------------------
class StandardYonma
	: public Rules
{
	Vector<Tile> m_tileSet;
public:
	StandardYonma();

	PlayerCount GetPlayerCount() const override { return 4_Players; }
	Points InitialPoints() const override { return 25'000; }
	Vector<Tile> const& Tileset() const override { return m_tileSet; }
	size_t DeadWallDrawsAvailable() const override { return 4u; }

	Set<Tile> WaitsWithYaku
	(
		RoundData const& i_round,
		Seat const& i_playerSeat,
		Hand const& i_hand,
		TileDraw const& i_lastTile
	) const override;

	Pair<Points, Vector<Pair<char const*, HanValue>>> CalculateBasicPoints
	(
		RoundData const& i_round,
		Seat const& i_playerSeat,
		Hand const& i_hand,
		TileDraw const& i_lastTile
	) const override;

	Pair<Points, Points> PointsFromEachPlayerTsumo( Points i_basicPoints, bool i_isDealer ) const override;
	Points PointsFromPlayerRon( Points i_basicPoints, bool i_isDealer ) const override;

private:
	static Points RoundTo100( Points i_finalPoints );
};

}
