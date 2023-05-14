#pragma once

#include "Player.hpp"
#include "PlayerCount.hpp"
#include "RoundData.hpp"
#include "Rules.hpp"
#include "Seat.hpp"
#include "TableState.hpp"
#include "TableEvent.hpp"
#include "Utils.hpp"

#include <array>
#include <algorithm>
#include <iostream>
#include <memory>
#include <optional>
#include <vector>

namespace Riichi
{

// The table runs the game, including each round - let's work on the basis that a round is not self-sufficient

struct Standings
{
	// TODO: how do we relate these points with the players? Should we handle this differently?
	std::vector<Points> m_points;

	explicit Standings( Rules const& i_rules ) : m_points( i_rules.GetPlayerCount(), i_rules.InitialPoints() ) {}

	friend std::ostream& operator<<( std::ostream& io_out, Standings const& i_standings )
	{
		io_out << "Standings:\n";
		for ( size_t i = 0; i < i_standings.m_points.size(); ++i )
		{
			if ( i > 0 ) { io_out << '\n'; }
			io_out << ToString( ( Seat )i ) << ":\t" << i_standings.m_points[ i ];
		}

		return io_out;
	}
};

class Table
{
private:
	std::unique_ptr<Rules> m_rules;
	std::vector<Player> m_players;
	Standings m_standings;
	std::vector<RoundData> m_rounds;
	TableState m_state;
	TableEvent m_mostRecentEvent;

public:
	Table
	(
		std::unique_ptr<Rules>&& i_rules,
		unsigned int i_shuffleSeed, // rng seed for tile shuffling
		unsigned int i_aiSeed // rng seed for AI behaviour
	)
		: m_rules{ std::move( i_rules ) }
		, m_standings{ *m_rules }
		, m_state{ TableState::Tag<TableStateType::Setup>{}, *this }
	{}

	// Setup
	void AddPlayer( Player&& i_player )
	{
		ensure( m_players.size() < m_rules->GetPlayerCount(), "Too many players added" );
		m_players.emplace_back( std::move( i_player ) );
	}
	
	// General data access
	Standings Standings() const { return m_standings; }

	// Simulation
	bool Playing() const;
	TableState const& GetState() { return m_state; }
	TableEvent const& GetEvent() const { return m_mostRecentEvent; }
};

}

#include "Table.inl"