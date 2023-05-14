#pragma once

#include "tile.hpp"

#include <cstdint>

namespace Riichi
{

enum class Seat : uint8_t
{
	East,
	South,
	West,
	North,
};

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

inline bool operator==( Seat i_seat, WindTileType i_windTile )
{
	return static_cast< uint8_t >( i_seat ) == static_cast< uint8_t >( i_windTile );
}

}
