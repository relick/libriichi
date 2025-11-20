#pragma once

#include "Containers.hpp"
#include "Declare.hpp"
#include "Hand.hpp"
#include "NamedUnion.hpp"
#include "Random.hpp"
#include "Tile.hpp"
#include "Utils.hpp"

namespace Riichi::TableStates
{
struct BaseTurn;
}

namespace Riichi::AI
{

//------------------------------------------------------------------------------
enum class TurnDecision
{
	Pending,

	Tsumo,
	Discard,
	Riichi,
	Kan,
};

using TurnDecisionData = NamedUnion<
	TurnDecision,
	std::monostate,       // Pending
	
	std::monostate,       // Tsumo
	Option<TileInstance>, // Discard
	Option<TileInstance>, // Riichi
	HandKanOption         // Kan
>;


//------------------------------------------------------------------------------
enum class BetweenTurnsDecision
{
	Pending,

	Pass,
	Chi,
	Pon,
	Kan,
	Ron,
};

using BetweenTurnsDecisionData = NamedUnion<
	BetweenTurnsDecision,
	std::monostate, // Pending
	
	std::monostate, // Pass
	ChiOption,      // Chi
	PonOption,      // Pon
	KanOption,      // Kan
	std::monostate  // Ron
>;

//------------------------------------------------------------------------------
// A new token will be generated each time a new decision needs to be made.
// This allows an agent to make decisions over multiple frames by returning the Pending
// decision state, which will lead to the call being made again with the same token.
// Receiving a new token means the agent should start a new decision process.
//------------------------------------------------------------------------------
using DecisionToken = TypeSafeID<struct DecisionTokenTag>;

//------------------------------------------------------------------------------
// Base AI interface
//------------------------------------------------------------------------------
struct Agent
{
	virtual ~Agent() = default;

	virtual TurnDecisionData MakeTurnDecision
	(
		DecisionToken i_token,
		AIRNG& io_rng,
		Seat i_agentSeat,
		Table const& i_table,
		Round const& i_round,
		TableStates::Turn_AI const& i_turnData
	) = 0;

	virtual BetweenTurnsDecisionData MakeBetweenTurnsDecision
	(
		DecisionToken i_token,
		AIRNG& io_rng,
		Seat i_agentSeat,
		Table const& i_table,
		Round const& i_round,
		TableStates::BetweenTurns const& i_turnData
	) = 0;
};

//------------------------------------------------------------------------------
// Base strategy-based AI interface
// 
// A strategy follows the same interface as an Agent, but provides a 'strength'
// value of how strongly a decision should be considered. Once all strategies
// are processed, the decision with the greatest value is chosen.
//------------------------------------------------------------------------------
struct Strategy
{
	using Strength = int32_t;

	virtual ~Strategy() = default;

	virtual Pair<TurnDecisionData, Strength> MakeTurnDecision
	(
		DecisionToken i_token,
		AIRNG& io_rng,
		Seat i_agentSeat,
		Table const& i_table,
		Round const& i_round,
		TableStates::Turn_AI const& i_turnData,
		bool i_strategyPreviouslyChosen
	) = 0;

	virtual Pair<BetweenTurnsDecisionData, Strength> MakeBetweenTurnsDecision
	(
		DecisionToken i_token,
		AIRNG& io_rng,
		Seat i_agentSeat,
		Table const& i_table,
		Round const& i_round,
		TableStates::BetweenTurns const& i_turnData,
		bool i_strategyPreviouslyChosen
	) = 0;
};

struct StrategyAgent
	: Agent
{
	Vector<std::unique_ptr<Strategy>> m_strategies;
	Strategy* m_mostRecentlyUsedStrategy{ nullptr };

	template<std::derived_from<Strategy>... T_Strategies>
		requires ( sizeof...( T_Strategies ) > 0 )
	StrategyAgent( T_Strategies... i_strategies )
		: m_strategies{ std::make_unique<T_Strategies>( std::move( i_strategies ) )... }
	{
	}

	TurnDecisionData MakeTurnDecision
	(
		DecisionToken i_token,
		AIRNG& io_rng,
		Seat i_agentSeat,
		Table const& i_table,
		Round const& i_round,
		TableStates::Turn_AI const& i_turnData
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