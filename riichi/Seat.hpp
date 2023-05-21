#pragma once

#include "Base.hpp"
#include "Tile.hpp" // to compare with wind tiles

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

}
