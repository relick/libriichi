#pragma once

#include "Base.hpp"

#include <any>

namespace Riichi
{

//-----------------------------------------------------------------------------
enum class PlayerType : EnumValueType
{
	User,
	AI,
};

//-----------------------------------------------------------------------------
using PlayerUserID = std::any;

//-----------------------------------------------------------------------------
class Player
{
	PlayerUserID m_userID;
	PlayerType m_type;
public:

	Player( std::any i_userID, PlayerType i_type )
		: m_userID{ std::move( i_userID ) }
		, m_type{ i_type }
	{}
};

}