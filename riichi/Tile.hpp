#pragma once

#include "NamedUnion.hpp"
#include "Utils.hpp"

#include <iostream>

namespace Riichi
{

//------------------------------------------------------------------------------
// Shared definitions for all tiles
//------------------------------------------------------------------------------
enum class Face : EnumValueType
{
	// Suit numbers
	One = 1,
	Two = 2,
	Three = 3,
	Four = 4,
	Five = 5,
	Six = 6,
	Seven = 7,
	Eight = 8,
	Nine = 9,

	// Dragons
	Haku,
	Hatsu,
	Chun,

	// Winds
	East,
	South,
	West,
	North,
};
using Faces = Utils::EnumRange<Face::One, Face::North>;

//------------------------------------------------------------------------------
// Number tiles
//------------------------------------------------------------------------------
enum class Suit : EnumValueType
{
	Manzu,
	Pinzu,
	Souzu,

	None,
};
using Suits = Utils::EnumRange<Suit::Manzu, Suit::Souzu>;
using Numbers = Utils::EnumRange<Face::One, Face::Nine>;
using Simples = Utils::EnumRange<Face::Two, Face::Eight>;

//inline Number operator+( Number i_num, size_t i_offset ) { return Numbers::IndexToValue( ( Numbers::ValueToIndex( i_num ) + i_offset ) % Numbers::Count() ); }
//inline Number operator-( Number i_num, size_t i_offset ) { return Numbers::IndexToValue( ( Numbers::ValueToIndex( i_num ) - ( i_offset % Numbers::Count() ) + Numbers::Count() ) % Numbers::Count() ); }

//------------------------------------------------------------------------------
// Honour tiles
//------------------------------------------------------------------------------
using Honours = Utils::EnumRange<Face::Haku, Face::North>;
using Dragons = Utils::EnumRange<Face::Haku, Face::Chun>;
using Winds = Utils::EnumRange<Face::East, Face::North>;

template<Face t_Dragon>
concept Dragon = Dragons::InRange( t_Dragon );

//------------------------------------------------------------------------------
// A TileKind stores only the pure properties of a tile (its suit, if it has one, and its face).
// Generally used for assessing hands for combinations and yaku, where specific instance data doesn't matter.
//------------------------------------------------------------------------------
class TileKind
{
protected:
	Suit m_suit;
	Face m_face;

public:
	constexpr TileKind( Suit i_suit, Face i_number )
		: m_suit{ i_suit }, m_face{ i_number }
	{
		riEnsure( Suits::InRange( i_suit ), "Failed to give a real suit to the 'suit + number' TileKind constructor" );
		riEnsure( IsNumber(), "Failed to give a number to the 'suit + number' TileKind constructor");
	}
	constexpr TileKind( Face i_honour )
		: m_suit{ Suit::None }, m_face{ i_honour }
	{
		riEnsure( IsHonour(), "Failed to give an honour to the 'honour' TileKind constructor" );
	}

	inline constexpr Suit Suit() const { return m_suit; }
	inline constexpr Face Face() const { return m_face; }

	inline constexpr bool IsNumber() const { return Numbers::InRange( m_face ); }
	inline constexpr bool IsSimple() const { return Simples::InRange( m_face ); }
	inline constexpr bool IsTerminal() const { return m_face == Face::One || m_face == Face::Nine; }

	inline constexpr bool IsHonour() const { return Honours::InRange( m_face ); }
	inline constexpr bool IsDragon() const { return Dragons::InRange( m_face ); }
	inline constexpr bool IsWind() const { return Winds::InRange( m_face ); }

	inline constexpr bool IsHonourOrTerminal() const { return IsHonour() || IsTerminal(); }

	inline constexpr TileKind Next() const
	{
		if ( IsDragon() )
		{
			return { Dragons::NextWrapped( m_face ) };
		}
		else if ( IsWind() )
		{
			return { Winds::NextWrapped( m_face ) };
		}
		else
		{
			return { m_suit, Numbers::NextWrapped( m_face ) };
		}
	}

	inline constexpr TileKind Prev() const
	{
		if ( IsDragon() )
		{
			return { Dragons::PrevWrapped( m_face ) };
		}
		else if ( IsWind() )
		{
			return { Winds::PrevWrapped( m_face ) };
		}
		else
		{
			return { m_suit, Numbers::PrevWrapped( m_face ) };
		}
	}

	friend constexpr bool operator==( TileKind const& i_a, TileKind const& i_b ) { return ( i_a.m_suit == i_b.m_suit ) && ( i_a.m_face == i_b.m_face ); }
	friend constexpr bool operator<( TileKind const& i_a, TileKind const& i_b ) { return ( i_a.m_suit == i_b.m_suit ) ? ( i_a.m_face < i_b.m_face ) : ( i_a.m_suit < i_b.m_suit ); }
	template <typename T> friend struct std::hash;
};

//------------------------------------------------------------------------------
// Special properties can be set on a tile. These are fully customisable by the ruleset, by instantiating Property.
//------------------------------------------------------------------------------
namespace Detail
{
static inline uint8_t s_flagCount = 0;
inline uint8_t MakeTilePropertyFlag() { ++s_flagCount; riEnsure( s_flagCount < 8, "Too many properties registered" ); return ( 1 << ( s_flagCount - 1 ) ); }
inline bool RegisteringTileProperties( bool i_flip ) { static bool s_registeringProperties = false; if ( i_flip ) { s_registeringProperties = !s_registeringProperties; } return s_registeringProperties; }
}

template<typename T_Tag>
struct TileProperty
{
	static uint8_t BitFlag()
	{
		static uint8_t s_flag = 0;
		if ( Detail::RegisteringTileProperties( false ) )
		{
			s_flag = Detail::MakeTilePropertyFlag();
		}
		return s_flag;
	}
};

// Call when setting up a ruleset.
template<typename... T_Tags>
inline void RegisterTileProperties( TileProperty<T_Tags>... i_properties )
{
	Detail::s_flagCount = 0;
	Detail::RegisteringTileProperties( true );
	( i_properties.BitFlag(), ... );
	Detail::RegisteringTileProperties( true );
}

class TileProperties
{
protected:
	uint8_t m_flags{ 0 };

	// 'more' properties set = 'higher' in a sorted list
	friend constexpr bool operator==( TileProperties const& i_a, TileProperties const& i_b ) { return i_a.m_flags == i_b.m_flags; }
	friend constexpr bool operator<( TileProperties const& i_a, TileProperties const& i_b ) { return i_a.m_flags < i_b.m_flags; }
	template <typename T> friend struct std::hash;

public:
	constexpr TileProperties() = default;

	template<typename... T_Tags>
	TileProperties( TileProperty<T_Tags>... i_properties )
		: m_flags{ uint8_t( ( i_properties.BitFlag() | ... ) ) }
	{}

	template<typename T_Property>
	constexpr bool HasProperty() const { return ( m_flags & T_Property::BitFlag() ) != 0; }
};

//------------------------------------------------------------------------------
// Common properties, predefined
//------------------------------------------------------------------------------
using Akadora = TileProperty<struct AkadoraTag>;

//------------------------------------------------------------------------------
// A Tile combines a TileKind with customisable TileProperties, representing the
// full meaning of a tile to a player.
//------------------------------------------------------------------------------
class Tile
	: public TileKind
	, public TileProperties
{
public:
	constexpr Tile( Riichi::Suit i_suit, Riichi::Face i_number, TileProperties i_properties = {} )
		: TileKind{ i_suit, i_number }, TileProperties{ i_properties }
	{}
	constexpr Tile( Riichi::Face i_honour, TileProperties i_properties = {} )
		: TileKind{ i_honour }, TileProperties{ i_properties }
	{}
	constexpr Tile( TileKind i_kind, TileProperties i_properties = {} )
		: TileKind{ i_kind }, TileProperties{ i_properties }
	{}

	inline constexpr Tile Next() const
	{
		return Tile{ TileKind::Next(), *this };
	}

	inline constexpr Tile Prev() const
	{
		return Tile{ TileKind::Prev(), *this };
	}
	
	inline constexpr TileKind const& Kind() const { return *this; }
};

//------------------------------------------------------------------------------
// A TileInstance combines a Tile with an instance ID, uniquely representing a
// particular tile that exists in a game.
// The ID is given priority over the properties, but this class is mostly just a 'pair'
// that makes it easier to identify a specific tile for visual purposes, rather than
// that the ID is supposed to impact the game.
//------------------------------------------------------------------------------
using TileInstanceID = TypeSafeID<struct TileInstanceIDTag>;
using TileInstanceIDGenerator = TypeSafeIDGenerator<TileInstanceID>;
class TileInstance
{
protected:
	TileInstanceID m_id;
	Tile m_tile;

public:
	explicit constexpr TileInstance( Tile i_tile, TileInstanceID i_tileID )
		: m_id{ i_tileID }, m_tile{ i_tile }
	{}

	// Projections:
	static inline constexpr Tile const& GetTile( TileInstance const& i_a ) { return i_a.Tile(); }
	static inline constexpr TileInstanceID GetID( TileInstance const& i_a ) { return i_a.ID(); }

	constexpr Tile const& Tile() const { return m_tile; }
	constexpr TileInstanceID ID() const { return m_id; }
};

template<typename R>
concept TileInstanceRange = Utils::RangeWithValueType<R, TileInstance>;
using DefaultTileInstanceRange = std::initializer_list<TileInstance>;

//------------------------------------------------------------------------------
inline constexpr TileKind const& GetKind( TileKind const& i_a ) { return i_a; }
inline constexpr TileKind const& GetKind( Tile const& i_a ) { return i_a.Kind(); }
inline constexpr TileKind const& GetKind( TileInstance const& i_a ) { return i_a.Tile().Kind(); }

template<typename T>
concept AnyTileType = requires( T a )
{
	{ GetKind( a ) } -> std::same_as<TileKind const&>;
};

//------------------------------------------------------------------------------
struct EqualsTileKindOp
{
	// Binary ops for containers
	// Comparisons between any types of tile
	template<AnyTileType T_TileTypeA, AnyTileType T_TileTypeB>
	constexpr bool operator()( T_TileTypeA const& i_a, T_TileTypeB const& i_b ) const
	{
		return GetKind( i_a ) == GetKind( i_b );
	}
};
struct EqualsTileKind
	: EqualsTileKindOp
{
	// Unary ops for search algorithms
	TileKind kind;

	// Implicitly construct from any type of tile
	template<AnyTileType T_TileType>
	constexpr EqualsTileKind( T_TileType const& i_tile ) : kind{ GetKind( i_tile ) } {}

	// Compare against any type of tile
	template<AnyTileType T_TileType>
	constexpr bool operator()( T_TileType const& i_a ) const
	{
		return EqualsTileKindOp::operator()( kind, i_a );
	}
};

//------------------------------------------------------------------------------
struct EqualsTileInstanceIDOp
{
	// Binary op for containers
	bool operator()( TileInstance const& i_a, TileInstance const& i_b ) const
	{
		return i_a.ID() == i_b.ID();
	}
};
struct EqualsTileInstanceID
	: EqualsTileInstanceIDOp
{
	// Unary op for search algorithms
	TileInstance instance;

	constexpr EqualsTileInstanceID( TileInstance const& i_tile ) : instance{ i_tile } {}

	bool operator()( TileInstance const& i_a ) const
	{
		return EqualsTileInstanceIDOp::operator()( instance, i_a );
	}
};

//------------------------------------------------------------------------------
struct CompareTileKindOp
{
	template<AnyTileType T_TileTypeA, AnyTileType T_TileTypeB>
	constexpr bool operator()( T_TileTypeA const& i_a, T_TileTypeB const& i_b ) const
	{
		return GetKind( i_a ) < GetKind( i_b );
	}
};
struct CompareTileInstanceIDOp
{
	bool operator()( TileInstance const& i_a, TileInstance const& i_b ) const
	{
		return i_a.ID() < i_b.ID();
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
	UpgradedKanTheft,
	ClosedKanTheft,
};

//------------------------------------------------------------------------------
struct TileDraw
{
	TileInstance m_tile;
	TileDrawType m_type;
};

} // Riichi

//------------------------------------------------------------------------------
template<>
struct std::hash<Riichi::TileKind>
{
	std::size_t operator()( Riichi::TileKind const& i_tile ) const noexcept
	{
		using namespace Riichi;

		std::size_t h1 = std::hash<EnumValueType>{}( static_cast< EnumValueType >( i_tile.Suit() ) );
		std::size_t h2 = std::hash<EnumValueType>{}( static_cast< EnumValueType >( i_tile.Face() ) );
		return h1 ^ ( h2 << 1 );
	}
};

//------------------------------------------------------------------------------
template<>
struct std::hash<Riichi::TileInstance>
{
	std::size_t operator()( Riichi::TileInstance const& i_tile ) const noexcept
	{
		return std::hash<Riichi::TileInstanceID>{}( i_tile.ID() );
	}
};