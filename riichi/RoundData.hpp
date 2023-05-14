#pragma once

#include "Seat.hpp"

#include <vector>

namespace Riichi
{

class RoundData
{
public:
	// TODO
	Seat Wind() const { return Seat::East; }
	bool IsDealer( Seat i_player ) const { return i_player == Seat::East; }
	// TODO
	bool CalledRiichi( Seat i_player ) const { return false; }
	// TODO
	bool CalledDoubleRiichi( Seat i_player ) const { return false; }
	// TODO
	bool RiichiIppatsuValid( Seat i_player ) const { return false; }
	// TODO
	std::vector<Tile> const& Discards( Seat i_player ) const { static std::vector<Tile> v; return v; }
	// TODO
	uint32_t WallSize() const { return 0u; }
	// TODO
	bool CallsMade() const { return false; }
};

}