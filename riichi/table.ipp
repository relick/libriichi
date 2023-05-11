#pragma once

#include "table.hpp"

namespace Riichi
{

template<PlayerCount t_PlayerCount>
bool Table<t_PlayerCount>::Playing
(
)	const
{
	return m_state == State::PlayingRound;
}

template<PlayerCount t_PlayerCount>
TableEvent Table<t_PlayerCount>::Step
(
	TableInput&& i_input
)
{
	switch ( m_state )
	{
	using enum State;
	case Setup:
	{
		m_state = State::PlayingRound;
		return TableEvent::Tag<TableEventType::RoundStart>();

		break;
	}
	case PlayingRound:
	{
		static int x = 0;
		if ( ++x == 3 )
		{
			m_state = State::GameOver;
		}
		return TableEvent::Tag<TableEventType::PlayerDraw>();
		break;
	}
	case GameOver:
	{
		return { TableEvent::Tag<TableEventType::Error>(), "Cannot step table simulation past game over." };
		break;
	}
	}

	return { TableEvent::Tag<TableEventType::Error>(), "Unknown table state!!" };
}


}