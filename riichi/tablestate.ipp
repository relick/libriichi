#pragma once

#include "table.hpp"
#include "tablestate.hpp"

namespace Riichi::TableStates
{

template<PlayerCount t_PlayerCount>
void Setup<t_PlayerCount>::StartGame
(
)	const
{}

template<PlayerCount t_PlayerCount>
void BetweenRounds<t_PlayerCount>::StartRound
(
)	const
{}

template<PlayerCount t_PlayerCount>
void Turn_AI<t_PlayerCount>::Discard
(
)	const
{}

template<PlayerCount t_PlayerCount>
void Turn_Player<t_PlayerCount>::Discard
(
	Tile const& i_tile
)	const
{}

template<PlayerCount t_PlayerCount>
void BetweenTurns<t_PlayerCount>::Pass
(
)	const
{}

template<PlayerCount t_PlayerCount>
void RobAKanChance<t_PlayerCount>::Pass
(
)	const
{}

}