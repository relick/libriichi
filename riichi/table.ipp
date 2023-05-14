#pragma once

#include "table.hpp"

namespace Riichi
{

bool Table::Playing
(
)	const
{
	return m_state.Type() != TableStateType::Setup && m_state.Type() != TableStateType::GameOver;
}

}