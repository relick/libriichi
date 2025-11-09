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
enum class Number : EnumValueType
{
	One      = 1,
	Two      = 2,
	Three    = 3,
	Four     = 4,
	Five     = 5,
	Six      = 6,
	Seven    = 7,
	Eight    = 8,
	Nine     = 9,
};
inline constexpr size_t c_numberCount = 9;
using Numbers = Utils::EnumRange<Number, 10, 1>;

inline constexpr EnumValueType ValueOf( Number i_num ) { return ( EnumValueType )i_num; }
inline constexpr size_t IndexOf( Number i_num ) { return ValueOf( i_num ) - 1; }
inline constexpr Number FromValue( EnumValueType i_int ) { Number num = ( Number )i_int; riEnsure( num >= Number::One && num <= Number::Nine, "Value needs to be within range" ); return num; }
inline constexpr Number FromIndex( EnumValueType i_int ) { Number num = ( Number )( i_int + 1 ); riEnsure( num >= Number::One && num <= Number::Nine, "Value needs to be within range" ); return num; }
inline constexpr Number operator+( Number i_num, size_t i_offset ) { return FromIndex( ( IndexOf( i_num ) + i_offset ) % c_numberCount ); }
inline constexpr Number operator-( Number i_num, size_t i_offset ) { return FromIndex( ( IndexOf( i_num ) - ( i_offset % c_numberCount ) + c_numberCount ) % c_numberCount ); }

//------------------------------------------------------------------------------
struct SuitTile
{
	template<Suit t_Suit> static constexpr SuitTile One() { return { t_Suit, Number::One }; }
	template<Suit t_Suit> static constexpr SuitTile Two() { return { t_Suit, Number::Two }; }
	template<Suit t_Suit> static constexpr SuitTile Three() { return { t_Suit, Number::Three }; }
	template<Suit t_Suit> static constexpr SuitTile Four() { return { t_Suit, Number::Four }; }
	template<Suit t_Suit> static constexpr SuitTile Five() { return { t_Suit, Number::Five }; }
	template<Suit t_Suit> static constexpr SuitTile Six() { return { t_Suit, Number::Six }; }
	template<Suit t_Suit> static constexpr SuitTile Seven() { return { t_Suit, Number::Seven }; }
	template<Suit t_Suit> static constexpr SuitTile Eight() { return { t_Suit, Number::Eight }; }
	template<Suit t_Suit> static constexpr SuitTile Nine() { return { t_Suit, Number::Nine }; }

	Suit m_suit;
	Number m_number;

	bool Equivalent( SuitTile const& ) const;
	bool IsTerminal() const;
	friend bool operator==( SuitTile const&, SuitTile const& ) = default;
	friend SuitTile NextTile( SuitTile const& i_tile );
	friend bool operator<( SuitTile const&, SuitTile const& );
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

	// a == b => a and b share tile type and tile face i.e. red 5m == non-red 5m is TRUE
	// a.Equivalent(b) => a and b share all tile features (such as being akadora)
	// a.Is(b) => matching IDs, if both a and b have IDs, otherwise returns a.Equivalent(b)
	bool Equivalent( Tile const& i_o ) const;
	bool Is( Tile const& i_o ) const;
	friend bool operator==( Tile const& i_a, Tile const& i_b ) { return i_a.m_data == i_b.m_data; }
	friend bool operator<( Tile const& i_a, Tile const& i_b ) { return i_a.m_data < i_b.m_data; }
	friend Tile NextTile( Tile const& i_tile );

	// Useful compound properties
	inline bool IsHonour() const { return Type() == TileType::Dragon || Type() == TileType::Wind; }
	inline bool IsTerminal() const { return Type() == TileType::Suit && Get<TileType::Suit>().IsTerminal(); }
	inline bool IsHonourOrTerminal() const { return IsHonour() || IsTerminal(); }
	inline bool IsSimple() const { return !IsHonourOrTerminal(); }
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
		std::size_t h2 = 0;
		switch ( i_tile.Type() )
		{
		using enum TileType;
		case Suit:
		{
			SuitTile const& suitTile = i_tile.Get<Suit>();
			std::size_t h3 = std::hash<EnumValueType>{}( static_cast< EnumValueType >( suitTile.m_suit ) );
			h2 = std::hash<uint8_t>{}( ValueOf( suitTile.m_number ) ) ^ ( h3 << 1 );
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