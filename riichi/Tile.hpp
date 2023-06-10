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

	bool Equivalent( SuitTile const& );
	friend bool operator==( SuitTile const&, SuitTile const& ) = default;
	friend SuitTile NextTile( SuitTile const& i_tile );
	friend bool operator<( SuitTile const&, SuitTile const& );

	template<Suit t_Suit, SuitTileValue::CoreType t_Value>
	static SuitTile Make()
	{
		static_assert( t_Value >= SuitTileValue::Min.m_val, "Value needs to be within range" );
		static_assert( t_Value <= SuitTileValue::Max.m_val, "Value needs to be within range" );
		return { t_Suit, SuitTileValue{ t_Value } };
	}
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
using TileData = NamedUnion<
	TileType,

	SuitTile,
	DragonTileType,
	WindTileType
>;

//------------------------------------------------------------------------------
class Tile
{
	TileData m_data;
	uint32_t m_tileID{ UINT32_MAX };

public:
	Tile( TileData i_data, uint32_t i_tileID ) : m_data{ std::move( i_data ) }, m_tileID{ i_tileID } {}
	Tile( SuitTile i_tile, uint32_t i_tileID = UINT32_MAX ) : Tile{ TileData{ i_tile }, i_tileID } {}
	Tile( DragonTileType i_tile, uint32_t i_tileID = UINT32_MAX ) : Tile{ TileData{ i_tile }, i_tileID } {}
	Tile( WindTileType i_tile, uint32_t i_tileID = UINT32_MAX ) : Tile{ TileData{ i_tile }, i_tileID } {}

	TileType Type() const { return m_data.Type(); }
	template<TileType t_Type>
	auto Get() const { return m_data.template Get<t_Type>(); }

	bool HasID() const { return m_tileID != UINT32_MAX; }
	uint32_t ID() const { return m_tileID; }

	// a == b => a and b share tile type and tile face
	// a.Equivalent(b) => a and b share all tile features (such as being akadora)
	// a.Is(b) => matching IDs, if either a or b have IDs, otherwise returns a.Equivalent(b)
	bool Equivalent( Tile const& i_o ) const;
	bool Is( Tile const& i_o ) const;
	friend bool operator==( Tile const& i_a, Tile const& i_b ) { return i_a.m_data == i_b.m_data; }
	friend bool operator<( Tile const& i_a, Tile const& i_b ) { return i_a.m_data < i_b.m_data; }
	friend Tile NextTile( Tile const& i_tile );
};

//------------------------------------------------------------------------------
struct EquivalentTile
{
	bool operator()( Tile const& i_a, Tile const& i_b ) const
	{
		return i_a.Equivalent( i_b );
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