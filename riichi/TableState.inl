#pragma once

#include "Table.hpp"
#include "TableState.hpp"

namespace Riichi::TableStates
{

void Setup::StartGame
(
)	const
{
	m_table.get().m_state = TableStates::BetweenRounds{ m_table.get() };
}

void BetweenRounds::StartRound
(
)	const
{
	m_table.get().m_state = TableStates::GameOver{ m_table.get() };
}

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