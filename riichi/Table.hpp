#pragma once

#include "Containers.hpp"
#include "Player.hpp"
#include "Random.hpp"
#include "Round.hpp"
#include "TableState.hpp"
#include "TableEvent.hpp"

#include <iostream>
#include <memory>

namespace Riichi
{

//------------------------------------------------------------------------------
// The table runs the game, including each round (rounds are not self-sufficient)
//------------------------------------------------------------------------------
class Table
{
	// Table states all considered an extension of the table class
	friend TableStates::Setup;
	friend TableStates::BetweenRounds;
	friend TableStates::GameOver;
	friend TableStates::BaseTurn;
	friend TableStates::Turn_AI;
	friend TableStates::Turn_User;
	friend TableStates::BetweenTurnsBase;
	friend TableStates::BetweenTurns;
	friend TableStates::RonAKanChance;

private:
	uint32_t m_ident{ 0 }; // TODO-DEBT: come up with some way to generate idents

	std::unique_ptr<Rules> m_rules;
	Vector<PlayerID> m_playerIDs;
	Vector<Pair<Player, Points>> m_players;
	Vector<Round> m_rounds;
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

	// TODO-DEBT: disallow copies + moves because tablestates hold a direct reference to the table
	// It would be really nice to fix this
	Table( Table const& ) = delete;
	Table( Table&& ) = delete;
	Table& operator=( Table const& ) = delete;
	Table& operator=( Table&& ) = delete;

	// Setup
	PlayerID AddPlayer( Player&& i_player );
	
	// General data access
	Player const& GetPlayer( PlayerID i_playerID ) const;
	template<typename T_Visitor>
	void VisitPlayers( T_Visitor&& i_visitor )  const;
	Points GetPoints( PlayerID i_playerID ) const;
	Points ModifyPoints( PlayerID i_playerID, Points i_amount );
	std::ostream& PrintStandings( std::ostream& io_out ) const;

	// Simulation
	bool Playing() const;
	TableState const& GetState() { return m_state; }
	bool HasRounds() const { return !m_rounds.empty(); }
	Round const& GetRound( size_t i_roundIndex = SIZE_MAX ) { return i_roundIndex >= m_rounds.size() ? m_rounds.back() : m_rounds[ i_roundIndex ]; }
	TableEvent const& GetEvent() const { return m_mostRecentEvent; }

private:
	void Transition( TableState&& i_nextState, TableEvent&& i_nextEvent );
};

}

#include "Table.inl"
