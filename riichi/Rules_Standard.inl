#pragma once

#include "Rules_Standard.hpp"

namespace Riichi
{

//------------------------------------------------------------------------------
template<Seat t_GameLength>
bool StandardYonma<t_GameLength>::NoMoreRounds
(
	Table const& i_table,
	Round const& i_previousRound
)	const
{
	return StandardYonmaCore::NoMoreRoundsCore( i_table, i_previousRound, t_GameLength );
}

}