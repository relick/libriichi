#pragma once

#include "Base.hpp"

#include <any>

namespace Riichi
{

//------------------------------------------------------------------------------
enum class PlayerType : EnumValueType
{
	User,
	AI,
};

//------------------------------------------------------------------------------
class Player
{
	PlayerType m_type;
public:
	PlayerType Type() const { return m_type; }

	Player( PlayerType i_type )
		: m_type{ i_type }
	{}
};

}