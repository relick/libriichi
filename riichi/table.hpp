#pragma once

#include "player.hpp"
#include "playercount.hpp"
#include "round.hpp"
#include "rules.hpp"
#include "seat.hpp"
#include "tablestate.hpp"
#include "tableevent.hpp"
#include "utils.hpp"

#include <array>
#include <algorithm>
#include <iostream>
#include <memory>
#include <optional>
#include <vector>

namespace Riichi
{

// The table runs the game, including each round - let's work on the basis that a round is not self-sufficient

template<PlayerCount t_PlayerCount>
struct Standings
{
	std::array<Points, t_PlayerCount> m_points;

	explicit Standings( Points i_initialPoints ) { m_points.fill( i_initialPoints ); }

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

template<PlayerCount t_PlayerCount>
class Table
{
public:
	using State = TableState<t_PlayerCount>;
	using Event = TableEvent;

private:
	std::unique_ptr<Rules<t_PlayerCount>> m_rules;
	std::array<std::optional<Player>, t_PlayerCount> m_players;
	Standings<t_PlayerCount> m_standings;
	std::vector<Round> m_rounds;
	State m_state;
	Event m_mostRecentEvent;

public:
	Table
	(
		std::unique_ptr<Rules<t_PlayerCount>>&& i_rules,
		unsigned int i_shuffleSeed, // rng seed for tile shuffling
		unsigned int i_aiSeed // rng seed for AI behaviour
	)
		: m_rules{ std::move( i_rules ) }
		, m_standings{ m_rules->InitialPoints() }
		, m_state{ typename State::template Tag<TableStateType::Setup>{}, *this }
	{}

	// Setup
	template<Seat t_Seat>
	void SetPlayer( Player&& i_player )
	{
		static_assert( ( size_t )t_Seat < t_PlayerCount, "Invalid seat for table player count" );
		m_players[ ( size_t )t_Seat ] = std::move( i_player );
	}
	
	// General data access
	Standings<t_PlayerCount> Standings() const { return m_standings; }

	// Simulation
	bool Playing() const;
	State const& GetState() { return m_state; }
	Event const& GetEvent() const { return m_mostRecentEvent; }
};

}

#include "table.ipp"