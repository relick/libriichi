#pragma once

#include "Rules.hpp"

namespace Riichi
{

//------------------------------------------------------------------------------
template<typename T_Visitor>
void Rules::VisitInterpreters
(
	T_Visitor&& i_visitor
)	 const
{
	for ( auto const& intepreter : m_interpreters )
	{
		i_visitor( *intepreter );
	}
}

//------------------------------------------------------------------------------
template<typename T_Visitor>
void Rules::VisitYaku
(
	T_Visitor&& i_visitor
)	const
{
	for ( auto const& yaku : m_yaku )
	{
		i_visitor( *yaku );
	}
}

}