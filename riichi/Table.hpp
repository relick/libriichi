#pragma once

#include "Player.hpp"
#include "Random.hpp"
#include "RoundData.hpp"
#include "TableState.hpp"
#include "TableEvent.hpp"
#include "Utils.hpp"

#include <iostream>
#include <memory>
#include <vector>

namespace Riichi
{

//------------------------------------------------------------------------------
struct Standings
{
	// TODO: how do we relate these points with the players? Should we handle this differently?
	std::vector<Points> m_points;

	explicit Standings( Rules const& i_rules );

	friend std::ostream& operator<<( std::ostream& io_out, Standings const& i_standings );
};

//------------------------------------------------------------------------------
// The table runs the game, including each round (rounds are not self-sufficient)
//------------------------------------------------------------------------------
class Table
{
	// Table states all considered an extension of the table class
	friend TableStates::Setup;
	friend TableStates::BetweenRounds;
	friend TableStates::GameOver;
	friend TableStates::Turn_AI;
	friend TableStates::Turn_Player;
	friend TableStates::BetweenTurns;
	friend TableStates::RobAKanChance;

private:
	std::unique_ptr<Rules> m_rules;
	std::vector<Player> m_players;
	Standings m_standings;
	std::vector<RoundData> m_rounds;
	TableState m_state;
	TableEvent m_mostRecentEvent;
	ShuffleRNG m_shuffleRNG;
	AIRNG m_aiRNG;

public:
	Table
	(
		std::unique_ptr<Rules>&& i_rules,
		unsigned int i_shuffleSeed, // rng seed for tile shuffling
		unsigned int i_aiSeed // rng seed for AI behaviour
	);

	// Setup
	void AddPlayer( Player&& i_player );
	
	// General data access
	Standings Standings() const { return m_standings; }

	// Simulation
	bool Playing() const;
	TableState const& GetState() { return m_state; }
	RoundData const& GetRoundData( size_t i_roundIndex = SIZE_MAX ) { return i_roundIndex >= m_rounds.size() ? m_rounds.back() : m_rounds[ i_roundIndex ]; }
	TableEvent const& GetEvent() const { return m_mostRecentEvent; }
	Player const& GetPlayer( size_t i_playerIndex ) const { return m_players[ i_playerIndex ]; }

private:
	void Transition( TableState&& i_nextState, TableEvent&& i_nextEvent );
};

}
