#include "Riichi.hpp"

#include "riichi/Rules_Standard.hpp"

#include <iostream>
#include <random>
#include "range/v3/algorithm.hpp"

int main()
{
	using namespace Riichi;

	TileInstanceIDGenerator generateID;

	StandardYonma<Seat::East> yonma;

	Hand nineGatesHand;
	for ( Face num : Numbers{} )
	{
		TileInstance const tile{ { Suit::Manzu, num }, generateID(), };
		if ( tile.Tile().IsTerminal() )
		{
			nineGatesHand.AddFreeTiles( { tile, tile, tile } );
		}
		else
		{
			nineGatesHand.AddFreeTiles( { tile } );
		}
	}

	HandAssessment nineGatesAssessment( nineGatesHand, yonma );

	Hand threeFourHand;
	threeFourHand.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::One }, generateID(), } } );
	threeFourHand.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::One }, generateID(), } } );
	threeFourHand.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::Two }, generateID(), } } );
	threeFourHand.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::Two }, generateID(), } } );
	threeFourHand.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::Two }, generateID(), } } );
	threeFourHand.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::Three }, generateID(), } } );
	threeFourHand.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::Three }, generateID(), } } );
	threeFourHand.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::Three }, generateID(), } } );
	threeFourHand.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::Nine }, generateID(), } } );
	threeFourHand.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::Nine }, generateID(), } } );
	threeFourHand.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::Five }, generateID(), } } );
	threeFourHand.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::Five }, generateID(), } } );
	threeFourHand.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::Five }, generateID(), } } );

	HandAssessment threeFourAssessment( threeFourHand, yonma );

	Hand allPairs;
	allPairs.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::Two }, generateID(), } } );
	allPairs.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::Two }, generateID(), } } );
	allPairs.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::Three }, generateID(), } } );
	allPairs.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::Three }, generateID(), } } );
	allPairs.AddFreeTiles( { TileInstance{ { Suit::Pinzu, Face::Two }, generateID(), } } );
	allPairs.AddFreeTiles( { TileInstance{ { Suit::Pinzu, Face::Two }, generateID(), } } );
	allPairs.AddFreeTiles( { TileInstance{ { Suit::Pinzu, Face::Three }, generateID(), } } );
	allPairs.AddFreeTiles( { TileInstance{ { Suit::Pinzu, Face::Three }, generateID(), } } );
	allPairs.AddFreeTiles( { TileInstance{ { Suit::Souzu, Face::Two }, generateID(), } } );
	allPairs.AddFreeTiles( { TileInstance{ { Suit::Souzu, Face::Two }, generateID(), } } );
	allPairs.AddFreeTiles( { TileInstance{ { Suit::Souzu, Face::Three }, generateID(), } } );
	allPairs.AddFreeTiles( { TileInstance{ { Suit::Souzu, Face::Three }, generateID(), } } );
	allPairs.AddFreeTiles( { TileInstance{ { Suit::Souzu, Face::Three }, generateID(), } } );

	HandAssessment allPairsAssessment( allPairs, yonma );

	Hand seqTrip;
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::One }, generateID(), } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::One }, generateID(), } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::One }, generateID(), } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::Two }, generateID(), } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::Six }, generateID(), } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::Six }, generateID(), } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::Six }, generateID(), } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Pinzu, Face::Six }, generateID(), } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Pinzu, Face::Six }, generateID(), } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Pinzu, Face::Six }, generateID(), } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Souzu, Face::Six }, generateID(), } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Souzu, Face::Six }, generateID(), } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Souzu, Face::Six }, generateID(), } } );

	HandAssessment seqTripAssessment( seqTrip, yonma );

	Hand multiInterpFewGroups;
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::Two }, generateID(), } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::Three }, generateID(), } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::Seven }, generateID(), } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::Eight }, generateID(), } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::Nine }, generateID(), } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::Nine }, generateID(), } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::Nine }, generateID(), } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Pinzu, Face::Six }, generateID(), } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Souzu, Face::Two }, generateID(), } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Souzu, Face::Six }, generateID(), } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Souzu, Face::Eight }, generateID(), } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Face::Chun }, generateID(), } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Face::Chun }, generateID(), } } );

	Hand multiInterpNoWaits;
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::Three }, generateID(), } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::Four }, generateID(), } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::Five }, generateID(), } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Pinzu, Face::Two }, generateID(), } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Pinzu, Face::Five }, generateID(), } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Pinzu, Face::Six }, generateID(), } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Pinzu, Face::Seven }, generateID(), } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Souzu, Face::Seven }, generateID(), } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Souzu, Face::Eight }, generateID(), } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Souzu, Face::Eight }, generateID(), } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Souzu, Face::Eight }, generateID(), } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Souzu, Face::Nine }, generateID(), } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Face::Chun }, generateID(), } } );

	Hand multiInterpSomeWaits;
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::Three }, generateID(), } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::Four }, generateID(), } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::Five }, generateID(), } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Pinzu, Face::Three }, generateID(), } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Pinzu, Face::Four }, generateID(), } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Pinzu, Face::Five }, generateID(), } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Pinzu, Face::Five }, generateID(), } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Pinzu, Face::Five }, generateID(), } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Souzu, Face::Three }, generateID(), } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Souzu, Face::Four }, generateID(), } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Souzu, Face::Six }, generateID(), } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Souzu, Face::Seven }, generateID(), } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Souzu, Face::Eight }, generateID(), } } );

	ShuffleRNG shuffleRNG{ std::random_device()( ) };
	while ( true )
	{
		Vector<TileInstance> tileSet = yonma.Tileset();
		ranges::shuffle( tileSet, shuffleRNG );
		tileSet.erase( tileSet.begin() + 13, tileSet.end() );

		Hand hand;
		hand.AddFreeTiles( tileSet );

		HandAssessment ass( hand, yonma );
		bool const unequalGroups = ranges::adjacent_find( ass.Interpretations(),
			[]( HandInterpretation const& i_a, HandInterpretation const& i_b ) -> bool
			{
				return i_a.m_groups.size() != i_b.m_groups.size() && i_b.m_groups.size() >= 3 && i_a.m_groups.size() >= 3;
			}
		) != ass.Interpretations().end();
		bool const hasWaits = ranges::find_if( ass.Interpretations(),
			[]( HandInterpretation const& i_i ) -> bool
			{
				return !i_i.m_waits.empty();
			}
		) != ass.Interpretations().end();
		if ( unequalGroups && hasWaits )
		{
			riError( "found one" );
		}
	}

	return 0;
}
