#pragma once

#include "AI.hpp"
#include "Round.hpp"
#include "Table.hpp"
#include "TableState.hpp"
#include "Utils.hpp"

namespace Riichi::AI
{

//------------------------------------------------------------------------------
// Discards all draws, as if they're not even there 👻
//------------------------------------------------------------------------------
struct GhostAgent
	: public Agent
{
	TurnDecisionData MakeTurnDecision
	(
		DecisionToken i_token,
		AIRNG& io_rng,
		Seat i_agentSeat,
		Riichi::Table const& i_table,
		Riichi::Round const& i_round,
		Riichi::TableStates::Turn_AI const& i_turnData
	) override;

	BetweenTurnsDecisionData MakeBetweenTurnsDecision
	(
		DecisionToken i_token,
		AIRNG& io_rng,
		Seat i_agentSeat,
		Table const& i_table,
		Round const& i_round,
		TableStates::BetweenTurns const& i_turnData
	) override;
};

//------------------------------------------------------------------------------
// Generally discards random tiles without thinking for how to progress hand.
// Will make any call available to it (for discards, riichi, or even wins), as soon as available.
//------------------------------------------------------------------------------
struct ButtonMasherAgent
	: public Agent
{
	TurnDecisionData MakeTurnDecision
	(
		DecisionToken i_token,
		AIRNG& io_rng,
		Seat i_agentSeat,
		Riichi::Table const& i_table,
		Riichi::Round const& i_round,
		Riichi::TableStates::Turn_AI const& i_turnData
	) override;

	BetweenTurnsDecisionData MakeBetweenTurnsDecision
	(
		DecisionToken i_token,
		AIRNG& io_rng,
		Seat i_agentSeat,
		Table const& i_table,
		Round const& i_round,
		TableStates::BetweenTurns const& i_turnData
	) override;
};

}