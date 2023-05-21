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
using TableEvent = Utils::NamedVariant<
	TableEventType,

	Utils::NullType,
	Utils::NullType,
	Utils::NullType,
	Utils::NullType,
	Utils::NullType,
	Utils::NullType,
	Utils::NullType,
	Utils::NullType,
	Utils::NullType,
	Utils::NullType,
	std::string
>;

}