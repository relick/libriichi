#pragma once

#include "Table.hpp"

namespace Riichi
{

//------------------------------------------------------------------------------
auto Table::AllPlayers
(
) const
{
	return std::views::all( m_players );
}

}