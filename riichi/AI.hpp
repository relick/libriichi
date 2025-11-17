#pragma once

#include "Containers.hpp"
#include "Declare.hpp"
#include "NamedUnion.hpp"
#include "Utils.hpp"
#include "Tile.hpp"
#include "Hand.hpp"

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
	std::monostate, // Pending
	
	std::monostate, // Tsumo
	Option<TileInstance>, // Discard
	Option<TileInstance>, // Riichi
	HandKanOption  // Kan
>;

using DecisionToken = TypeSafeID<struct DecisionTokenTag>;

//------------------------------------------------------------------------------
struct Agent
{
	virtual ~Agent() = default;

	virtual TurnDecisionData MakeTurnDecision( DecisionToken i_token, Table const& i_table, Round const& i_round, TableStates::BaseTurn const& i_turnData ) = 0;
};

}