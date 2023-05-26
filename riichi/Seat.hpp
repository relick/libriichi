#pragma once

#include "Base.hpp"
#include "Tile.hpp" // to compare with wind tiles
#include "Utils.hpp"

namespace Riichi
{

//------------------------------------------------------------------------------
enum class Seat : EnumValueType
{
	East,
	South,
	West,
	North,
};
inline constexpr size_t c_seatCount = 4;
using Seats = Utils::EnumRange<Seat, c_seatCount>;
using SeatSet = Utils::EnumSet<Seat, c_seatCount>;

//------------------------------------------------------------------------------
inline constexpr char const* ToString( Seat i_seat )
{
	constexpr char const* strs[] = {
		"East",
		"South",
		"West",
		"North",
	};

	return strs[ ( size_t )i_seat ];
}

//------------------------------------------------------------------------------
inline bool operator==( Seat i_seat, WindTileType i_windTile )
{
	return static_cast< EnumValueType >( i_seat ) == static_cast< EnumValueType >( i_windTile );
}

//------------------------------------------------------------------------------
inline Seat NextPlayer( Seat i_seat, size_t i_playerCount )
{
	do
	{
		i_seat = i_seat == Seat::North ? Seat::East : ( Seat )( ( EnumValueType )i_seat + 1 );
	} while ( ( size_t )i_seat >= i_playerCount );
	return i_seat;
}

}
