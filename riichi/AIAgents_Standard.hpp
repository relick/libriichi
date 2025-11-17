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
	) override
	{
		return { TurnDecisionData::Tag<TurnDecision::Discard>(), std::nullopt };
	}

	BetweenTurnsDecisionData MakeBetweenTurnsDecision
	(
		DecisionToken i_token,
		AIRNG& io_rng,
		Seat i_agentSeat,
		Table const& i_table,
		Round const& i_round,
		TableStates::BetweenTurns const& i_turnData
	)
	{
		return { BetweenTurnsDecisionData::Tag<BetweenTurnsDecision::Pass>() };
	}
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
	) override
	{
		if ( i_turnData.CanTsumo() )
		{
			return { TurnDecisionData::Tag<TurnDecision::Tsumo>() };
		}

		if ( i_turnData.CanKan() )
		{
			return { TurnDecisionData::Tag<TurnDecision::Kan>(), i_turnData.KanOptions().front() };
		}

		if ( i_turnData.CanRiichi() )
		{
			return { TurnDecisionData::Tag<TurnDecision::Riichi>(), i_turnData.RiichiOptions().front() };
		}

		Vector<TileInstance> discardOptions = i_turnData.GetCurrentHand().FreeTiles();
		auto tileDraw = i_turnData.GetCurrentTileDraw();
		if ( tileDraw )
		{
			discardOptions.push_back( *tileDraw );
		}
		std::ranges::shuffle( discardOptions, io_rng );

		if ( tileDraw && discardOptions.front().ID() == tileDraw->ID() )
		{
			return { TurnDecisionData::Tag<TurnDecision::Discard>(), std::nullopt };
		}
		else
		{
			return { TurnDecisionData::Tag<TurnDecision::Discard>(), discardOptions.front() };
		}
	}

	BetweenTurnsDecisionData MakeBetweenTurnsDecision
	(
		DecisionToken i_token,
		AIRNG& io_rng,
		Seat i_agentSeat,
		Table const& i_table,
		Round const& i_round,
		TableStates::BetweenTurns const& i_turnData
	)
	{
		if ( i_turnData.CanRon().Contains( i_agentSeat ) )
		{
			return { BetweenTurnsDecisionData::Tag<BetweenTurnsDecision::Ron>() };
		}

		if ( !i_turnData.CanKan()[ i_agentSeat ].empty() )
		{
			return { BetweenTurnsDecisionData::Tag<BetweenTurnsDecision::Kan>(), i_turnData.CanKan()[ i_agentSeat ].front() };
		}

		if ( !i_turnData.CanPon()[ i_agentSeat ].empty() )
		{
			return { BetweenTurnsDecisionData::Tag<BetweenTurnsDecision::Pon>(), i_turnData.CanPon()[ i_agentSeat ].front() };
		}

		if ( !i_turnData.CanChi()[ i_agentSeat ].empty() )
		{
			return { BetweenTurnsDecisionData::Tag<BetweenTurnsDecision::Chi>(), i_turnData.CanChi()[ i_agentSeat ].front() };
		}

		return { BetweenTurnsDecisionData::Tag<BetweenTurnsDecision::Pass>() };
	}
};

}