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
	friend SuitTile NextTile( SuitTile const& i_tile );
	friend bool StrictEqualTo( SuitTile const&, SuitTile const& );
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
DragonTileType NextTile( DragonTileType i_tile );

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
WindTileType NextTile( WindTileType i_tile );

//------------------------------------------------------------------------------
using Tile = NamedUnion<
	TileType,

	SuitTile,
	DragonTileType,
	WindTileType
>;

//------------------------------------------------------------------------------
Tile NextTile( Tile const& i_tile );

//------------------------------------------------------------------------------
bool StrictEqualTo( Tile const&, Tile const& );

struct StrictEqualToTile
{
	bool operator()( Tile const& i_a, Tile const& i_b ) const
	{
		return StrictEqualTo( i_a, i_b );
	}
};

//------------------------------------------------------------------------------
std::ostream& operator<<( std::ostream& io_out, Tile const& i_tile );

//------------------------------------------------------------------------------
enum class TileDrawType : EnumValueType
{
	SelfDraw,
	DiscardDraw,
	DeadWallDraw,
	KanTheft,
};

//------------------------------------------------------------------------------
struct TileDraw
{
	Tile m_tile;
	TileDrawType m_type;
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