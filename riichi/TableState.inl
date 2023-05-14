#pragma once

#include "Table.hpp"
#include "TableState.hpp"

namespace Riichi::TableStates
{

void Setup::StartGame
(
)	const
{}

void BetweenRounds::StartRound
(
)	const
{}

void Turn_AI::Discard
(
)	const
{}

void Turn_Player::Discard
(
	Tile const& i_tile
)	const
{}

void BetweenTurns::Pass
(
)	const
{}

void RobAKanChance::Pass
(
)	const
{}

}