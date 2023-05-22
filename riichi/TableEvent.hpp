#pragma once

#include "Base.hpp"
#include "Utils.hpp"

namespace Riichi
{

//------------------------------------------------------------------------------
enum class TableEventType : EnumValueType
{
	None,

	// Draw possibilities
	DealerDraw, // Equivalent to round start.
	Draw,
	Call,

	// Turn possibilities
	Discard,
	Riichi, // also a discard
	Kan,

	// Round end possibilities
	Tsumo,
	Ron,
	WallDepleted,

	Error,
};

//------------------------------------------------------------------------------
inline constexpr char const* ToString( TableEventType i_type )
{
	constexpr char const* strs[] =
	{
		"None",

		"DealerDraw",
		"Draw",
		"Call",
		"Discard",
		"Riichi",
		"Kan",
		"Tsumo",
		"Ron",
		"WallDepleted",

		"Error",
	};
	return strs[ ( size_t )i_type ];
}

//------------------------------------------------------------------------------
namespace TableEvents
{

//------------------------------------------------------------------------------
class Draw
{
	Tile m_tileDrawn;
public:
	explicit Draw( Tile const& i_tileDrawn )
		: m_tileDrawn{ i_tileDrawn }
	{}

	Tile const& TileDrawn() const { return m_tileDrawn; }
};

//------------------------------------------------------------------------------
class Discard
{
	Tile m_tileDiscarded;
public:
	explicit Discard( Tile const& i_tileDiscarded )
		: m_tileDiscarded{ i_tileDiscarded }
	{}

	Tile const& TileDiscarded() const { return m_tileDiscarded; }
};

//------------------------------------------------------------------------------
class WallDepleted
{
public:
	explicit WallDepleted()
	{}
};

}

//------------------------------------------------------------------------------
using TableEvent = Utils::NamedVariant<
	TableEventType,

	Utils::NullType,

	TableEvents::Draw, // DealerDraw is just a different type but identical data to a Draw
	TableEvents::Draw,
	Utils::NullType,
	TableEvents::Discard,
	Utils::NullType,
	Utils::NullType,
	Utils::NullType,
	Utils::NullType,
	TableEvents::WallDepleted,

	std::string
>;

}