#include "Riichi.hpp"

#include <iostream>
#include <random>

int main()
{
	// Able to define rules + scoring of a game in some generic and extensible way
	//// As proof of above, should be able to define standard yonma, yonma with some variant hands, sanma, and wareme
	// Create a table, define players, then start game
	//// Just as if it was a windowing engine, events need to be handled
	//// Should be serialisable and deserialisable, as well as replayable

	Riichi::Table table(
		std::make_unique<Riichi::StandardYonma>(),
		std::random_device()(),
		std::random_device()()
	);

	table.AddPlayer( Riichi::Player{
		0,
		Riichi::PlayerType::AI
	} );
	table.AddPlayer( Riichi::Player{
		1,
		Riichi::PlayerType::AI
	} );
	table.AddPlayer( Riichi::Player{
		2,
		Riichi::PlayerType::AI
	} );
	table.AddPlayer( Riichi::Player{
		3,
		Riichi::PlayerType::AI
	} );


	int loopCount = 0;
	// TODO: Let's start with the manual loop + manual switch approach
	// then see if we can tidy it up with something better later
	do
	{
		std::cout << "--" << loopCount++ << "--" << std::endl;
		// First, get current state, and process (providing any input necessary)
		Riichi::TableState const& state = table.GetState();
		switch ( state.Type() )
		{
		using enum Riichi::TableStateType;

		case Setup:
		{
			std::cout << "Starting game" << std::endl;
			state.Get<Setup>().StartGame();
			break;
		}
		case BetweenRounds:
		{
			std::cout << "Starting next round\n";
			state.Get<BetweenRounds>().StartRound();

			std::cout << "Initial Hands: \n";
			for ( Riichi::Seat seat : Riichi::Seats{} )
			{
				std::cout << Riichi::ToString( seat ) << ": " << table.GetRoundData().Hand( seat );
			}
			std::cout << std::endl;
			break;
		}
		case GameOver:
		{
			std::cout << "Nothing to do, game is over" << std::endl;
			break;
		}
		case Turn_AI:
		{
			Riichi::TableStates::Turn_AI const& turn = state.Get<Turn_AI>();
			std::cout << "AI in seat " << ToString( turn.GetSeat() ) << " taking turn" << std::endl;
			turn.Discard();
			break;
		}
		case Turn_Player:
		{
			Riichi::TableStates::Turn_Player const& turn = state.Get<Turn_Player>();
			std::cout << "Player in seat " << ToString( turn.GetSeat() ) << " taking turn" << std::endl;
			turn.Discard( Riichi::DragonTileType::White );
			break;
		}
		case BetweenTurns:
		{
			std::cout << "Passing to next turn" << std::endl;
			state.Get<BetweenTurns>().Pass();
			break;
		}
		case RobAKanChance:
		{
			std::cout << "Passing on rob-a-kan chance" << std::endl;
			state.Get<RobAKanChance>().Pass();
			break;
		}

		}

		// Then, get resulting event, and process into output. This part is technically optional,
		// but required if you want meaningful output
		Riichi::TableEvent const& event = table.GetEvent();
		switch ( event.Type() )
		{
		using enum Riichi::TableEventType;

		case None:
		{
			break;
		}
		case Error:
		{
			std::cerr << "Error: " << event.Get<Error>() << std::endl;
			break;
		}

		case DealerDraw:
		{
			Riichi::TableEvents::Draw const& draw = event.Get<DealerDraw>();
			std::cout << "Round started with dealer drawing tile " << draw.TileDrawn() << std::endl;
			break;
		}

		case Draw:
		{
			Riichi::TableEvents::Draw const& draw = event.Get<Draw>();
			std::cout << Riichi::ToString( table.GetRoundData().CurrentTurn() ) << " drew tile " << draw.TileDrawn() << std::endl;
			break;
		}

		case Discard:
		{
			Riichi::TableEvents::Discard const& discard = event.Get<Discard>();
			std::cout << Riichi::ToString( table.GetRoundData().CurrentTurn() ) << " discarded tile " << discard.TileDiscarded() << std::endl;
			break;
		}

		default:
		{
			std::cout << "Event: " << ToString( event.Type() ) << std::endl;
			break;
		}

		}

	} while ( table.Playing() );

	std::cout << "Game over!\n";
	std::cout << table.Standings() << std::endl;

	return 0;
}
