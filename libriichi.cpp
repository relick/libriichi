#include "riichi/riichi.hpp"

#include <iostream>
#include <random>

int main()
{
	// Able to define rules + scoring of a game in some generic and extensible way
	//// As proof of above, should be able to define standard yonma, yonma with some variant hands, sanma, and wareme
	// Create a table, define players, then start game
	//// Just as if it was a windowing engine, events need to be handled
	//// Should be serialisable and deserialisable, as well as replayable

	Riichi::Table<4_Players> table(
		std::make_unique<Riichi::StandardYonma>(),
		std::random_device()(),
		std::random_device()()
	);

	table.SetPlayer<Riichi::Seat::East>( Riichi::Player{
		Riichi::PlayerType::AI
	} );
	table.SetPlayer<Riichi::Seat::South>( Riichi::Player{
		Riichi::PlayerType::AI
	} );
	table.SetPlayer<Riichi::Seat::West>( Riichi::Player{
		Riichi::PlayerType::AI
	} );
	table.SetPlayer<Riichi::Seat::North>( Riichi::Player{
		Riichi::PlayerType::AI
	} );

	do
	{
		Riichi::TableEvent event = table.Step();
		switch ( event.Type() )
		{
		using enum Riichi::TableEventType;

		case Error:
		{
			std::cerr << "Error: " << event.Get<Error>() << std::endl;
			break;
		}
		default:
		{
			std::cout << "Event: " << ToString(event.Type()) << std::endl;
			break;
		}

		}
	} while ( table.Playing() );

	std::cout << table.Standings() << std::endl;

	return 0;
}
