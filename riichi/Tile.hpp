#pragma once

#include "NamedUnion.hpp"
#include "Utils.hpp"

#include <iostream>

namespace Riichi
{

//------------------------------------------------------------------------------
enum class TileType : EnumValueType
{
	Suit,
	Dragon,
	Wind,
};
inline constexpr size_t c_tileTypeCount = 3;
using TileTypes = Utils::EnumRange<TileType, c_tileTypeCount>;

//------------------------------------------------------------------------------
enum class Suit : EnumValueType
{
	Manzu,
	Pinzu,
	Souzu,
};
inline constexpr size_t c_suitCount = 3;
using Suits = Utils::EnumRange<Suit, c_suitCount>;

//------------------------------------------------------------------------------
using SuitTileValue = Utils::RestrictedIntegral<uint8_t, 1, 1, 9>;

//------------------------------------------------------------------------------
struct SuitTile
{
	Suit m_suit;
	SuitTileValue m_value;

	friend bool operator==( SuitTile const&, SuitTile const& ) = default;
	friend std::strong_ordering operator<=>( SuitTile const&, SuitTile const& ) = default;
};

//------------------------------------------------------------------------------
enum class DragonTileType : EnumValueType
{
	White,
	Green,
	Red,
};
inline constexpr size_t c_dragonTileTypeCount = 3;
using DragonTileTypes = Utils::EnumRange<DragonTileType, c_dragonTileTypeCount>;

//------------------------------------------------------------------------------
enum class WindTileType : EnumValueType
{
	East,
	South,
	West,
	North,
};
inline constexpr size_t c_windTileTypeCount = 4;
using WindTileTypes = Utils::EnumRange<WindTileType, c_windTileTypeCount>;

//------------------------------------------------------------------------------
using Tile = NamedUnion<
	TileType,

	SuitTile,
	DragonTileType,
	WindTileType
>;

//------------------------------------------------------------------------------
inline std::ostream& operator<<( std::ostream& io_out, Tile const& i_tile )
{
	switch ( i_tile.Type() )
	{
	using enum TileType;
	case Suit:
	{
		SuitTile const& tile = i_tile.Get<Suit>();
		switch ( tile.m_suit )
		{
		using enum Suit;
		case Manzu: io_out << static_cast<int>( tile.m_value.m_val ) << "m"; return io_out;
		case Pinzu: io_out << static_cast<int>( tile.m_value.m_val ) << "p"; return io_out;
		case Souzu: io_out << static_cast<int>( tile.m_value.m_val ) << "s"; return io_out;
		}
		break;
	}
	case Dragon:
	{
		DragonTileType const& tile = i_tile.Get<Dragon>();
		switch ( tile )
		{
		using enum DragonTileType;
		case White: io_out << "白"; return io_out;
		case Green: io_out << "発"; return io_out;
		case Red: io_out << "中"; return io_out;
		}
		break;
	}
	case Wind:
	{
		WindTileType const& tile = i_tile.Get<Wind>();
		switch ( tile )
		{
		using enum WindTileType;
		case East: io_out << "東"; return io_out;
		case South: io_out << "南"; return io_out;
		case West: io_out << "西"; return io_out;
		case North: io_out << "北"; return io_out;
		}
		break;
	}
	}
	return io_out;
}

//------------------------------------------------------------------------------
enum class TileDrawType : EnumValueType
{
	SelfDraw,
	DiscardDraw,
	DeadWallDraw,
	KanTheft,
};

}

//------------------------------------------------------------------------------
template<>
struct std::hash<Riichi::Tile>
{
	std::size_t operator()( Riichi::Tile const& i_tile ) const noexcept
	{
		using namespace Riichi;

		std::size_t h1 = std::hash<EnumValueType>{}( static_cast< EnumValueType >( i_tile.Type() ) );
		std::size_t h2;
		switch ( i_tile.Type() )
		{
		using enum TileType;
		case Suit:
		{
			SuitTile const& suitTile = i_tile.Get<Suit>();
			std::size_t h3 = std::hash<EnumValueType>{}( static_cast< EnumValueType >( suitTile.m_suit ) );
			h2 = std::hash<uint8_t>{}( suitTile.m_value ) ^ ( h3 << 1 );
			break;
		}
		case Dragon:
		{
			h2 = std::hash<EnumValueType>{}( static_cast< EnumValueType >( i_tile.Get<Dragon>() ) );
			break;
		}
		case Wind:
		{
			h2 = std::hash<EnumValueType>{}( static_cast< EnumValueType >( i_tile.Get<Wind>() ) );
			break;
		}
		}
		return h1 ^ ( h2 << 1 );
	}
};