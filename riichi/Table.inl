#pragma once

#include "Table.hpp"

namespace Riichi
{

//------------------------------------------------------------------------------
template<typename T_Visitor>
void Table::VisitPlayers
(
	T_Visitor&& i_visitor
)	 const
{
	for ( auto const& player : m_players )
	{
		i_visitor( player );
	}
}

}