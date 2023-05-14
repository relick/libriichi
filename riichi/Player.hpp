#pragma once

namespace Riichi
{

enum class PlayerType
{
	User,
	AI,
};

class Player
{
	PlayerType m_type;
public:

	Player( PlayerType i_type )
		: m_type{ i_type }
	{}
};

}