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
	TileKind( Suit i_suit, Face i_number )
		: m_suit{ i_suit }, m_face{ i_number }
	{
		riEnsure( Suits::InRange( i_suit ), "Failed to give a real suit to the 'suit + number' TileKind constructor" );
		riEnsure( IsNumber(), "Failed to give a number to the 'suit + number' TileKind constructor");
	}
	TileKind( Face i_honour )
		: m_suit{ Suit::None }, m_face{ i_honour }
	{
		riEnsure( IsHonour(), "Failed to give an honour to the 'honour' TileKind constructor" );
	}

	Suit Suit() const { return m_suit; }
	Face Face() const { return m_face; }

	inline bool IsNumber() const { return Numbers::InRange( m_face ); }
	inline bool IsSimple() const { return Simples::InRange( m_face ); }
	inline bool IsTerminal() const { return m_face == Face::One || m_face == Face::Nine; }

	inline bool IsHonour() const { return Honours::InRange( m_face ); }
	inline bool IsDragon() const { return Dragons::InRange( m_face ); }
	inline bool IsWind() const { return Winds::InRange( m_face ); }

	inline bool IsHonourOrTerminal() const { return IsHonour() || IsTerminal(); }

	inline TileKind Next() const
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

	inline TileKind Prev() const
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

	friend bool operator==( TileKind const& i_a, TileKind const& i_b ) { return ( i_a.m_suit == i_b.m_suit ) && ( i_a.m_face == i_b.m_face ); }
	friend bool operator<( TileKind const& i_a, TileKind const& i_b ) { return ( i_a.m_suit == i_b.m_suit ) ? ( i_a.m_face < i_b.m_face ) : ( i_a.m_suit < i_b.m_suit ); }
	template <typename T> friend class std::hash;
};

//------------------------------------------------------------------------------
// Special properties can be set on a tile. These are fully customisable by the ruleset, by instantiating Property.
//------------------------------------------------------------------------------
namespace Detail
{
static inline uint8_t s_flagCount = 0;
static inline bool s_registeringProperties = false;
inline uint8_t MakeTilePropertyFlag() { ++s_flagCount; riEnsure( s_flagCount < 8, "Too many properties registered" ); return ( 1 << ( s_flagCount - 1 ) ); }
inline bool RegisteringTileProperties() { return s_registeringProperties; }
}

template<typename T_Tag>
struct TileProperty
{
	static uint8_t BitFlag()
	{
		static uint8_t s_flag = 0;
		if ( Detail::RegisteringTileProperties() )
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
	Detail::s_registeringProperties = true;
	Detail::s_flagCount = 0;
	( i_properties.BitFlag(), ... );
	Detail::s_registeringProperties = false;
}

class TileProperties
{
protected:
	uint8_t m_flags{ 0 };

public:
	template<typename... T_Tags>
	TileProperties( TileProperty<T_Tags>... i_properties )
		: m_flags{ uint8_t( ( 0 | ... | i_properties.BitFlag() ) ) }
	{}

	template<typename T_Property>
	bool HasProperty() const { return ( m_flags & T_Property::BitFlag() ) != 0; }

	// 'more' properties set = 'higher' in a sorted list
	friend bool operator==( TileProperties const& i_a, TileProperties const& i_b ) { return i_a.m_flags == i_b.m_flags; }
	friend bool operator<( TileProperties const& i_a, TileProperties const& i_b ) { return i_a.m_flags < i_b.m_flags; }
	template <typename T> friend class std::hash;
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
	Tile( Riichi::Suit i_suit, Riichi::Face i_number, TileProperties i_properties = {} )
		: TileKind{ i_suit, i_number }, TileProperties{ i_properties }
	{}
	Tile( Riichi::Face i_honour, TileProperties i_properties = {} )
		: TileKind{ i_honour }, TileProperties{ i_properties }
	{}
	Tile( TileKind i_kind, TileProperties i_properties = {} )
		: TileKind{ i_kind }, TileProperties{ i_properties }
	{}

	inline Tile Next() const
	{
		return { TileKind::Next(), *this };
	}

	inline Tile Prev() const
	{
		return { TileKind::Prev(), *this };
	}

	inline TileKind const& Kind() const { return *this; }
	inline TileProperties const& Properties() const { return *this; }

	friend bool operator==( Tile const& i_a, Tile const& i_b )
	{
		return ( i_a.Kind() == i_b.Kind() ) && ( i_a.Properties() == i_b.Properties() );
	}
	friend bool operator<( Tile const& i_a, Tile const& i_b )
	{
		if ( i_a.Kind() == i_b.Kind() )
		{
			return i_a.Properties() < i_b.Properties();
		}
		
		return i_a.Kind() < i_b.Kind();
	}

	// Also allowed to compare tiles directly to kinds or properties, just to check that aspect of them
	friend bool operator==( Tile const& i_a, TileKind const& i_b )
	{
		return ( TileKind const& )i_a.Kind() == i_b;
	}
	friend bool operator==( TileKind const& i_a, Tile const& i_b )
	{
		return i_b == i_a;
	}
	friend bool operator==( Tile const& i_a, TileProperties const& i_b )
	{
		return i_a.Properties() == i_b;
	}
	friend bool operator==( TileProperties const& i_a, Tile const& i_b )
	{
		return i_b == i_a;
	}
};

//------------------------------------------------------------------------------
// A TileInstance combines a Tile with an instance ID, uniquely representing a
// particular tile that exists in a game.
// The ID is given priority over the properties, but this class is mostly just a 'pair'
// that makes it easier to identify a specific tile for visual purposes, rather than
// that the ID is supposed to impact the game.
//------------------------------------------------------------------------------
class TileInstance
{
protected:
	uint32_t m_id{ UINT32_MAX };
	Tile m_tile;

public:
	TileInstance( Tile i_tile, uint32_t i_tileID = UINT32_MAX )
		: m_id{ i_tileID }, m_tile{ i_tile }
	{}

	Tile const& Tile() const { return m_tile; }

	bool HasID() const { return m_id != UINT32_MAX; }
	uint32_t ID() const { return m_id; }

	// Instance IDs are meant to be unique identifiers, and tile instances are meant to be
	// constant once the game begins, so if IDs match then we consider that to mean equality.
	friend bool operator==( TileInstance const& i_a, TileInstance const& i_b )
	{
		if ( i_a.HasID() && i_b.HasID() )
		{
			return i_a.m_id == i_b.m_id;
		}
		return i_a.m_tile == i_b.m_tile;
	}
};

//------------------------------------------------------------------------------
struct OrderTileInstanceByTile
{
	bool operator()( TileInstance const& i_a, TileInstance const& i_b ) const
	{
		return i_a.Tile() < i_b.Tile();
	}
};
struct OrderTileInstanceByID
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
	KanTheft,
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
struct std::hash<Riichi::Tile>
{
	std::size_t operator()( Riichi::Tile const& i_tile ) const noexcept
	{
		using namespace Riichi;

		std::size_t h1 = std::hash<Riichi::TileKind>{}( i_tile );
		std::size_t h2 = std::hash<uint8_t>{}( i_tile.m_flags );
		return h1 ^ ( h2 << 1 );
	}
};