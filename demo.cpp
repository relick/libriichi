#include "Riichi.hpp"

#include "riichi/Rules_Standard.hpp"

#include <iostream>
#include <random>
#include <ranges>

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

	Riichi::PlayerID const player1 = table.AddPlayer( Riichi::Player{
		Riichi::PlayerType::User
	} );
	table.AddPlayer( Riichi::Player{
		Riichi::PlayerType::AI
	} );
	table.AddPlayer( Riichi::Player{
		Riichi::PlayerType::AI
	} );
	table.AddPlayer( Riichi::Player{
		Riichi::PlayerType::AI
	} );

	auto fnParseTile = []( std::string const& i_input ) -> Riichi::Option<Riichi::Tile>
	{
		if ( std::isdigit( i_input[ 0 ] ) )
		{
			if ( i_input[ 1 ] == 'm' )
			{
				return Riichi::SuitTile{Riichi::Suit::Manzu, Riichi::SuitTileValue(( uint8_t )i_input[ 0 ] - '0')};
			}
			else if ( i_input[ 1 ] == 'p' )
			{
				return Riichi::SuitTile{Riichi::Suit::Pinzu, Riichi::SuitTileValue(( uint8_t )i_input[ 0 ] - '0')};
			}
			else if ( i_input[ 1 ] == 's' )
			{
				return Riichi::SuitTile{Riichi::Suit::Souzu, Riichi::SuitTileValue(( uint8_t )i_input[ 0 ] - '0') };
			}
		}
		else if ( i_input == "east" )
		{
			return Riichi::WindTileType::East;
		}
		else if ( i_input == "south" )
		{
			return Riichi::WindTileType::South;
		}
		else if ( i_input == "west" )
		{
			return Riichi::WindTileType::West;
		}
		else if ( i_input == "north" )
		{
			return Riichi::WindTileType::North;
		}
		else if ( i_input == "white" )
		{
			return Riichi::DragonTileType::White;
		}
		else if ( i_input == "green" )
		{
			return Riichi::DragonTileType::Green;
		}
		else if ( i_input == "red" )
		{
			return Riichi::DragonTileType::Red;
		}

		return std::nullopt;
	};

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
				std::cout << Riichi::ToString( seat ) << ": " << table.GetRound().GetHand( seat ) << '\n';
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
			std::cout << "Player in seat " << ToString( turn.GetSeat() ) << " taking turn\n";
			std::cout << "Hand: " << turn.GetHand();
			if ( turn.GetDrawnTile().has_value() )
			{
				std::cout << " " << turn.GetDrawnTile().value();
			}
			std::cout << "\n";

			if ( turn.CanKan() )
			{
				std::cout << "/Kan ";
			}
			if ( turn.CanTsumo() )
			{
				std::cout << "/Tsumo ";
			}
			if ( turn.CanRiichi() )
			{
				std::cout << "/Riichi ";
			}
			std::cout << std::endl;

			while ( true )
			{
				std::string input;
				std::cin >> input;
				if ( input.starts_with( "kan" ) && input.size() > 4 && turn.CanKan() )
				{
					Riichi::Option<Riichi::Tile> const tile = fnParseTile( input.substr( 4 ) );
					if ( tile.has_value() && std::ranges::any_of( turn.KanOptions(), [ & ]( auto const& i_option ) -> bool
						{
							return i_option.kanTile == tile.value();
						} ) )
					{
						turn.Kan( tile.value() );
						break;
					}
				}
				else if ( input == "tsumo" && turn.CanTsumo() )
				{
					turn.Tsumo();
					break;
				}
				else if ( input.starts_with( "riichi" ) && input.size() > 7 && turn.CanRiichi() )
				{
					Riichi::Option<Riichi::Tile> const tile = fnParseTile( input.substr( 7 ) );
					if ( tile.has_value() )
					{
						if ( tile == turn.GetDrawnTile() )
						{
							turn.Riichi( std::nullopt );
							break;
						}
						else if ( std::ranges::contains( turn.GetHand().FreeTiles(), tile.value() ) )
						{
							turn.Riichi( tile );
							break;
						}
					}
				}
				else
				{
					Riichi::Option<Riichi::Tile> const tile = fnParseTile( input );
					if ( tile.has_value() )
					{
						if ( tile == turn.GetDrawnTile() )
						{
							turn.Discard( std::nullopt );
							break;
						}
						else if ( std::ranges::contains( turn.GetHand().FreeTiles(), tile.value() ) )
						{
							turn.Discard( tile );
							break;
						}
					}
				}
			}

			break;
		}
		case BetweenTurns:
		{
			Riichi::TableStates::BetweenTurns const& betweenTurns = state.Get<BetweenTurns>();
			Riichi::Seat const playerSeat = table.GetRound().GetSeat( player1 );
			bool passStraightAway = true;
			if ( betweenTurns.CanChi().first == playerSeat && !betweenTurns.CanChi().second.empty() )
			{
				std::cout << "/Chi ";
				passStraightAway = false;
			}
			if ( betweenTurns.CanPon().Contains( playerSeat ) )
			{
				std::cout << "/Pon ";
				passStraightAway = false;
			}
			if ( betweenTurns.CanKan().Contains( playerSeat ) )
			{
				std::cout << "/Kan ";
				passStraightAway = false;
			}
			if ( betweenTurns.CanRon().Contains( playerSeat ) )
			{
				std::cout << "/Ron ";
				passStraightAway = false;
			}

			if ( passStraightAway )
			{
				std::cout << "Passing to next turn" << std::endl;
				betweenTurns.UserPass();
				break;
			}
			else
			{
				std::cout << "/Pass ";
				std::cout << std::endl;
			}

			while ( true )
			{
				std::string input;
				std::cin >> input;
				if ( input == "chi" && betweenTurns.CanChi().first == playerSeat && !betweenTurns.CanChi().second.empty() )
				{
					betweenTurns.UserChi( playerSeat, betweenTurns.CanChi().second.front() );
					break;
				}
				else if ( input == "pon" && betweenTurns.CanPon().Contains( playerSeat ) )
				{
					betweenTurns.UserPon( playerSeat );
					break;
				}
				else if ( input == "kan" && betweenTurns.CanKan().Contains( playerSeat ) )
				{
					betweenTurns.UserKan( playerSeat );
					break;
				}
				else if ( input == "ron" && betweenTurns.CanRon().Contains( playerSeat ) )
				{
					betweenTurns.UserRon( Riichi::SeatSet{ playerSeat } );
					break;
				}
				else
				{
					betweenTurns.UserPass();
					break;
				}
			}

			break;
		}
		case RonAKanChance:
		{
			Riichi::TableStates::RonAKanChance const& ronAKanChance = state.Get<RonAKanChance>();
			Riichi::Seat const playerSeat = table.GetRound().GetSeat( player1 );
			bool passStraightAway = true;
			if ( ronAKanChance.CanRon().Contains( playerSeat ) )
			{
				std::cout << "/Ron ";
				passStraightAway = false;
			}

			if ( passStraightAway )
			{
				std::cout << "Passing on rob-a-kan chance" << std::endl;
				ronAKanChance.Pass();
				break;
			}
			else
			{
				std::cout << "/Pass ";
				std::cout << std::endl;
			}

			while ( true )
			{
				std::string input;
				std::cin >> input;
				if ( input.starts_with( "ron" ) && ronAKanChance.CanRon().Contains( playerSeat ) )
				{
					ronAKanChance.Ron( Riichi::SeatSet{ playerSeat } );
					break;
				}
				else
				{
					ronAKanChance.Pass();
					break;
				}
			}

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
			std::cout << "Round started with dealer drawing tile " << draw.TileDrawn().m_tile << std::endl;
			break;
		}

		case Draw:
		{
			Riichi::TableEvents::Draw const& draw = event.Get<Draw>();
			std::cout << Riichi::ToString( draw.Player() ) << " drew tile " << draw.TileDrawn().m_tile << std::endl;
			break;
		}

		case Discard:
		{
			Riichi::TableEvents::Discard const& discard = event.Get<Discard>();
			std::cout << Riichi::ToString( discard.Player() ) << " discarded tile " << discard.TileDiscarded() << std::endl;
			break;
		}

		case Tsumo:
		{
			Riichi::TableEvents::Tsumo const& tsumo = event.Get<Tsumo>();
			auto const& score = table.GetRound().WinnerScore( tsumo.Winner() ).value();
			std::cout << Riichi::ToString( tsumo.Winner() ) << " won with tsumo! Score:\n";
			Riichi::Han total = 0;
			for ( auto const& yaku : score.second )
			{
				std::cout << yaku.first << ": ";
				if ( yaku.second.IsYakuman() )
				{
					std::cout << "yakuman";
				}
				else
				{
					total += yaku.second.Get();
					std::cout << static_cast< int >( yaku.second.Get() );
				}
				std::cout << "\n";
			}
			std::cout << "Total: " << static_cast< int >( total ) << " (" << score.first << ")\n\n";

			table.PrintStandings( std::cout ) << "\n" << std::endl;
			break;
		}

		case Ron:
		{
			Riichi::TableEvents::Ron const& ron = event.Get<Ron>();
			for ( auto const& winner : ron.Winners() )
			{
				auto const& score = table.GetRound().WinnerScore( winner ).value();
				std::cout << Riichi::ToString( winner ) << " won with ron! Score:\n";
				Riichi::Han total = 0;
				for ( auto const& yaku : score.second )
				{
					std::cout << yaku.first << ": ";
					if ( yaku.second.IsYakuman() )
					{
						std::cout << "yakuman";
					}
					else
					{
						total += yaku.second.Get();
						std::cout << static_cast< int >( yaku.second.Get() );
					}
					std::cout << "\n";
				}
				std::cout << "Total: " << static_cast< int >( total ) << " (" << score.first << ")\n\n";
			}

			table.PrintStandings( std::cout ) << "\n" << std::endl;
			break;
		}

		case WallDepleted:
		{
			std::cout << "Draw\n\n";
			table.PrintStandings( std::cout ) << "\n" << std::endl;
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
	table.PrintStandings( std::cout ) << std::endl;

	return 0;
}
