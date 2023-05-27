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

	{
		Riichi::Hand nineGatesHand;
		for ( Riichi::SuitTileValue val : Riichi::SuitTileValue::InclusiveRange( Riichi::SuitTileValue::Min, Riichi::SuitTileValue::Max ) )
		{
			Riichi::SuitTile const tile{Riichi::Suit::Manzu, val};
			if ( val == Riichi::SuitTileValue::Min || val == Riichi::SuitTileValue::Max )
			{
				nineGatesHand.AddFreeTiles( { tile, tile, tile } );
			}
			else
			{
				nineGatesHand.AddFreeTiles( { tile } );
			}
		}

		Riichi::HandAssessment nineGatesAssessment( nineGatesHand );

		Riichi::Hand threeFourHand;
		threeFourHand.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Manzu, Riichi::SuitTileValue::Set<1>()} } );
		threeFourHand.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Manzu, Riichi::SuitTileValue::Set<1>()} } );
		threeFourHand.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Manzu, Riichi::SuitTileValue::Set<2>()} } );
		threeFourHand.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Manzu, Riichi::SuitTileValue::Set<2>()} } );
		threeFourHand.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Manzu, Riichi::SuitTileValue::Set<2>()} } );
		threeFourHand.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Manzu, Riichi::SuitTileValue::Set<3>()} } );
		threeFourHand.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Manzu, Riichi::SuitTileValue::Set<3>()} } );
		threeFourHand.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Manzu, Riichi::SuitTileValue::Set<3>()} } );
		threeFourHand.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Manzu, Riichi::SuitTileValue::Set<9>()} } );
		threeFourHand.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Manzu, Riichi::SuitTileValue::Set<9>()} } );
		threeFourHand.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Manzu, Riichi::SuitTileValue::Set<5>()} } );
		threeFourHand.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Manzu, Riichi::SuitTileValue::Set<5>()} } );
		threeFourHand.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Manzu, Riichi::SuitTileValue::Set<5>()} } );

		Riichi::HandAssessment threeFourAssessment( threeFourHand );

		Riichi::Hand allPairs;
		allPairs.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Manzu, Riichi::SuitTileValue::Set<2>()} } );
		allPairs.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Manzu, Riichi::SuitTileValue::Set<2>()} } );
		allPairs.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Manzu, Riichi::SuitTileValue::Set<3>()} } );
		allPairs.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Manzu, Riichi::SuitTileValue::Set<3>()} } );
		allPairs.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Pinzu, Riichi::SuitTileValue::Set<2>()} } );
		allPairs.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Pinzu, Riichi::SuitTileValue::Set<2>()} } );
		allPairs.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Pinzu, Riichi::SuitTileValue::Set<3>()} } );
		allPairs.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Pinzu, Riichi::SuitTileValue::Set<3>()} } );
		allPairs.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Souzu, Riichi::SuitTileValue::Set<2>()} } );
		allPairs.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Souzu, Riichi::SuitTileValue::Set<2>()} } );
		allPairs.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Souzu, Riichi::SuitTileValue::Set<3>()} } );
		allPairs.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Souzu, Riichi::SuitTileValue::Set<3>()} } );
		allPairs.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Souzu, Riichi::SuitTileValue::Set<3>()} } );

		Riichi::HandAssessment allPairsAssessment( allPairs );

		Riichi::Hand seqTrip;
		seqTrip.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Manzu, Riichi::SuitTileValue::Set<1>()} } );
		seqTrip.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Manzu, Riichi::SuitTileValue::Set<1>()} } );
		seqTrip.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Manzu, Riichi::SuitTileValue::Set<1>()} } );
		seqTrip.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Manzu, Riichi::SuitTileValue::Set<2>()} } );
		seqTrip.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Manzu, Riichi::SuitTileValue::Set<6>()} } );
		seqTrip.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Manzu, Riichi::SuitTileValue::Set<6>()} } );
		seqTrip.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Manzu, Riichi::SuitTileValue::Set<6>()} } );
		seqTrip.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Pinzu, Riichi::SuitTileValue::Set<6>()} } );
		seqTrip.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Pinzu, Riichi::SuitTileValue::Set<6>()} } );
		seqTrip.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Pinzu, Riichi::SuitTileValue::Set<6>()} } );
		seqTrip.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Souzu, Riichi::SuitTileValue::Set<6>()} } );
		seqTrip.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Souzu, Riichi::SuitTileValue::Set<6>()} } );
		seqTrip.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Souzu, Riichi::SuitTileValue::Set<6>()} } );

		Riichi::HandAssessment seqTripAssessment( seqTrip );

		Riichi::StandardYonma yonma;
		Riichi::ShuffleRNG shuffleRNG{ std::random_device()() };
		while ( true )
		{
			Riichi::Vector<Riichi::Tile> tileSet = yonma.Tileset();
			std::ranges::shuffle( tileSet, shuffleRNG );
			tileSet.erase( tileSet.begin() + 13, tileSet.end() );

			Riichi::Hand hand;
			hand.AddFreeTiles( tileSet );

			Riichi::HandAssessment ass( hand );
			bool const unequalGroups = std::ranges::adjacent_find( ass.Interpretations(),
				[]( Riichi::HandInterpretation const& i_a, Riichi::HandInterpretation const& i_b ) -> bool
				{
					return i_a.m_groups.size() != i_b.m_groups.size() && i_b.m_groups.size() >= 3 && i_a.m_groups.size() >= 3;
				}
			) != ass.Interpretations().end();
			bool const hasWaits = std::ranges::find_if( ass.Interpretations(),
				[]( Riichi::HandInterpretation const& i_i ) -> bool
				{
					return !i_i.m_waits.empty();
				}
			) != ass.Interpretations().end();
			if ( unequalGroups && hasWaits )
			{
				Error( "found one" );
			}
		}

		Riichi::Hand multiInterpFewGroups;
		seqTrip.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Manzu, Riichi::SuitTileValue::Set<2>()} } );
		seqTrip.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Manzu, Riichi::SuitTileValue::Set<3>()} } );
		seqTrip.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Manzu, Riichi::SuitTileValue::Set<7>()} } );
		seqTrip.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Manzu, Riichi::SuitTileValue::Set<8>()} } );
		seqTrip.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Manzu, Riichi::SuitTileValue::Set<9>()} } );
		seqTrip.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Manzu, Riichi::SuitTileValue::Set<9>()} } );
		seqTrip.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Manzu, Riichi::SuitTileValue::Set<9>()} } );
		seqTrip.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Pinzu, Riichi::SuitTileValue::Set<6>()} } );
		seqTrip.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Souzu, Riichi::SuitTileValue::Set<2>()} } );
		seqTrip.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Souzu, Riichi::SuitTileValue::Set<6>()} } );
		seqTrip.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Souzu, Riichi::SuitTileValue::Set<8>()} } );
		seqTrip.AddFreeTiles( { Riichi::DragonTileType::Red } );
		seqTrip.AddFreeTiles( { Riichi::DragonTileType::Red } );

		Riichi::Hand multiInterpNoWaits;
		seqTrip.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Manzu, Riichi::SuitTileValue::Set<3>()} } );
		seqTrip.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Manzu, Riichi::SuitTileValue::Set<4>()} } );
		seqTrip.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Manzu, Riichi::SuitTileValue::Set<5>()} } );
		seqTrip.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Pinzu, Riichi::SuitTileValue::Set<2>()} } );
		seqTrip.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Pinzu, Riichi::SuitTileValue::Set<5>()} } );
		seqTrip.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Pinzu, Riichi::SuitTileValue::Set<6>()} } );
		seqTrip.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Pinzu, Riichi::SuitTileValue::Set<7>()} } );
		seqTrip.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Souzu, Riichi::SuitTileValue::Set<7>()} } );
		seqTrip.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Souzu, Riichi::SuitTileValue::Set<8>()} } );
		seqTrip.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Souzu, Riichi::SuitTileValue::Set<8>()} } );
		seqTrip.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Souzu, Riichi::SuitTileValue::Set<8>()} } );
		seqTrip.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Souzu, Riichi::SuitTileValue::Set<9>()} } );
		seqTrip.AddFreeTiles( { Riichi::DragonTileType::Red } );

		Riichi::Hand multiInterpSomeWaits;
		seqTrip.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Manzu, Riichi::SuitTileValue::Set<3>()} } );
		seqTrip.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Manzu, Riichi::SuitTileValue::Set<4>()} } );
		seqTrip.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Manzu, Riichi::SuitTileValue::Set<5>()} } );
		seqTrip.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Pinzu, Riichi::SuitTileValue::Set<3>()} } );
		seqTrip.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Pinzu, Riichi::SuitTileValue::Set<4>()} } );
		seqTrip.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Pinzu, Riichi::SuitTileValue::Set<5>()} } );
		seqTrip.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Pinzu, Riichi::SuitTileValue::Set<5>()} } );
		seqTrip.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Pinzu, Riichi::SuitTileValue::Set<5>()} } );
		seqTrip.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Souzu, Riichi::SuitTileValue::Set<3>()} } );
		seqTrip.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Souzu, Riichi::SuitTileValue::Set<4>()} } );
		seqTrip.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Souzu, Riichi::SuitTileValue::Set<6>()} } );
		seqTrip.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Souzu, Riichi::SuitTileValue::Set<7>()} } );
		seqTrip.AddFreeTiles( { Riichi::SuitTile{Riichi::Suit::Souzu, Riichi::SuitTileValue::Set<8>()} } );

	}

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
				std::cout << Riichi::ToString( seat ) << ": " << table.GetRoundData().GetHand( seat );
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
			turn.MakeDecision();
			break;
		}
		case Turn_User:
		{
			Riichi::TableStates::Turn_User const& turn = state.Get<Turn_User>();
			std::cout << "Player in seat " << ToString( turn.GetSeat() ) << " taking turn" << std::endl;
			turn.Discard( Riichi::DragonTileType::White );
			break;
		}
		case BetweenTurns:
		{
			std::cout << "Passing to next turn" << std::endl;
			state.Get<BetweenTurns>().UserPass();
			break;
		}
		case RonAKanChance:
		{
			std::cout << "Passing on rob-a-kan chance" << std::endl;
			state.Get<RonAKanChance>().Pass();
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
			std::cout << Riichi::ToString( draw.Player() ) << " drew tile " << draw.TileDrawn() << std::endl;
			break;
		}

		case Discard:
		{
			Riichi::TableEvents::Discard const& discard = event.Get<Discard>();
			std::cout << Riichi::ToString( discard.Player() ) << " discarded tile " << discard.TileDiscarded() << std::endl;
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
