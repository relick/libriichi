#pragma once

#include "table.hpp"

namespace Riichi
{

template<PlayerCount t_PlayerCount>
bool Table<t_PlayerCount>::Playing
(
)	const
{
	return m_state.Type() != TableStateType::Setup && m_state.Type() != TableStateType::GameOver;
}

}