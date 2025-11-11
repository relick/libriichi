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
using Seats = Utils::EnumRange<Seat::East, Seat::North>;
using SeatSet = Utils::EnumSet<Seats>;

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
inline bool operator==( Seat i_seat, Face i_windTile )
{
	return Seats::ValueToIndex( i_seat ) == Winds::ValueToIndex( i_windTile );
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

//------------------------------------------------------------------------------
inline bool IsDealer
(
	Seat i_player
)
{
	return i_player == Seat::East;
}

}
