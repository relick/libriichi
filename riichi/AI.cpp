#include "AI.hpp"

namespace Riichi::AI
{

//------------------------------------------------------------------------------
template<std::derived_from<Strategy>... T_Strategies>
	requires ( sizeof...( T_Strategies ) > 0 )
StrategyAgent::StrategyAgent( T_Strategies... i_strategies )
	: m_strategies{ std::make_unique<T_Strategies>( std::move( i_strategies ) )... }
{
}

//------------------------------------------------------------------------------
TurnDecisionData StrategyAgent::MakeTurnDecision
(
	DecisionToken i_token,
	AIRNG& io_rng,
	Seat i_agentSeat,
	Table const& i_table,
	Round const& i_round,
	TableStates::Turn_AI const& i_turnData
)
{
	Vector<Pair<TurnDecisionData, Strategy::Strength>> results;
	results.reserve( m_strategies.size() );
	bool anyPending = false;
	for ( std::unique_ptr<Strategy> const& strategy : m_strategies )
	{
		results.push_back( strategy->MakeTurnDecision( i_token, io_rng, i_agentSeat, i_table, i_round, i_turnData, strategy.get() == m_mostRecentlyUsedStrategy ) );
		anyPending |= ( results.back().first.Type() == TurnDecision::Pending );
	}

	if ( anyPending )
	{
		return TurnDecisionData{};
	}
	else
	{
		auto bestStrategy = std::ranges::max_element( results, []( auto const& a, auto const& b ) { return a.second < b.second; } );
		m_mostRecentlyUsedStrategy = m_strategies[ std::distance( results.begin(), bestStrategy ) ].get();
		return bestStrategy->first;
	}
}

//------------------------------------------------------------------------------
BetweenTurnsDecisionData StrategyAgent::MakeBetweenTurnsDecision
(
	DecisionToken i_token,
	AIRNG& io_rng,
	Seat i_agentSeat,
	Table const& i_table,
	Round const& i_round,
	TableStates::BetweenTurns const& i_turnData
)
{
	Vector<Pair<BetweenTurnsDecisionData, Strategy::Strength>> results;
	results.reserve( m_strategies.size() );
	bool anyPending = false;
	for ( std::unique_ptr<Strategy> const& strategy : m_strategies )
	{
		results.push_back( strategy->MakeBetweenTurnsDecision( i_token, io_rng, i_agentSeat, i_table, i_round, i_turnData, strategy.get() == m_mostRecentlyUsedStrategy ) );
		anyPending |= ( results.back().first.Type() == BetweenTurnsDecision::Pending );
	}

	if ( anyPending )
	{
		return BetweenTurnsDecisionData{};
	}
	else
	{
		auto bestStrategy = std::ranges::max_element( results, []( auto const& a, auto const& b ) { return a.second < b.second; } );
		m_mostRecentlyUsedStrategy = m_strategies[ std::distance( results.begin(), bestStrategy ) ].get();
		return bestStrategy->first;
	}
}

}
