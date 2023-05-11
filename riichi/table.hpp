#pragma once

#include "player.hpp"
#include "round.hpp"
#include "rules.hpp"
#include "seat.hpp"
#include "utils.hpp"

#include <array>
#include <algorithm>
#include <iostream>
#include <memory>
#include <optional>
#include <vector>

namespace Riichi
{

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

enum class TableEventType
{
	Error,
	RoundStart,
	PlayerDraw,
	PlayerDiscard,
	PlayerChance,
	PlayerPon,
	PlayerChi,
	PlayerKan,
	PlayerRon,
	PlayerTsumo,
};

inline constexpr char const* ToString( TableEventType i_type )
{
	constexpr char const* strs[] =
	{
		"Error",
		"RoundStart",
		"PlayerDraw",
		"PlayerDiscard",
		"PlayerChance",
		"PlayerPon",
		"PlayerChi",
		"PlayerKan",
		"PlayerRon",
		"PlayerTsumo",
	};
	return strs[ ( size_t )i_type ];
}

using TableEvent = Utils::NamedVariant<
	TableEventType,

	std::string,
	Utils::NullType,
	Utils::NullType,
	Utils::NullType,
	Utils::NullType,
	Utils::NullType,
	Utils::NullType,
	Utils::NullType,
	Utils::NullType,
	Utils::NullType,
	Utils::NullType,
	Utils::NullType
>;

enum class TableInputType
{
	None,
	PlayerDiscard,
	PlayerChance,
};

using TableInput = Utils::NamedVariant<
	TableInputType,

	Utils::NullType,
	Utils::NullType,
	Utils::NullType
>;

template<PlayerCount t_PlayerCount>
class Table
{
private:
	enum class State
	{
		Setup,
		PlayingRound,
		GameOver,
	};

private:
	std::unique_ptr<Rules<t_PlayerCount>> m_rules;
	std::array<std::optional<Player>, t_PlayerCount> m_players;
	Standings<t_PlayerCount> m_standings;
	std::vector<Round> m_rounds;
	State m_state{ State::Setup };

public:
	Table
	(
		std::unique_ptr<Rules<t_PlayerCount>>&& i_rules,
		unsigned int i_shuffleSeed, // rng seed for tile shuffling
		unsigned int i_aiSeed // rng seed for AI behaviour
	)
		: m_rules{ std::move( i_rules ) }
		, m_standings{ m_rules->InitialPoints() }
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
	TableEvent Step( TableInput&& i_input = TableInput() );
};

}

#include "table.ipp"