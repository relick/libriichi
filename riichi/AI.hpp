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

}