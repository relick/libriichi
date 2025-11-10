#include "Riichi.hpp"

#include "riichi/Rules_Standard.hpp"

#include <iostream>
#include <random>
#include "range/v3/algorithm.hpp"

int main()
{
	Riichi::StandardYonma<Riichi::Seat::East> yonma;

	Riichi::Hand nineGatesHand;
	for ( Riichi::Face num : Riichi::Numbers{} )
	{
		Riichi::Tile const tile{ Riichi::Suit::Manzu, num };
		if ( tile.IsTerminal() )
		{
			nineGatesHand.AddFreeTiles( { tile, tile, tile } );
		}
		else
		{
			nineGatesHand.AddFreeTiles( { tile } );
		}
	}

	Riichi::HandAssessment nineGatesAssessment( nineGatesHand, yonma );

	Riichi::Hand threeFourHand;
	threeFourHand.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Manzu, Riichi::Face::One }, } );
	threeFourHand.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Manzu, Riichi::Face::One }, } );
	threeFourHand.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Manzu, Riichi::Face::Two }, } );
	threeFourHand.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Manzu, Riichi::Face::Two }, } );
	threeFourHand.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Manzu, Riichi::Face::Two }, } );
	threeFourHand.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Manzu, Riichi::Face::Three }, } );
	threeFourHand.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Manzu, Riichi::Face::Three }, } );
	threeFourHand.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Manzu, Riichi::Face::Three }, } );
	threeFourHand.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Manzu, Riichi::Face::Nine }, } );
	threeFourHand.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Manzu, Riichi::Face::Nine }, } );
	threeFourHand.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Manzu, Riichi::Face::Five }, } );
	threeFourHand.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Manzu, Riichi::Face::Five }, } );
	threeFourHand.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Manzu, Riichi::Face::Five }, } );

	Riichi::HandAssessment threeFourAssessment( threeFourHand, yonma );

	Riichi::Hand allPairs;
	allPairs.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Manzu, Riichi::Face::Two }, } );
	allPairs.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Manzu, Riichi::Face::Two }, } );
	allPairs.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Manzu, Riichi::Face::Three }, } );
	allPairs.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Manzu, Riichi::Face::Three }, } );
	allPairs.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Pinzu, Riichi::Face::Two }, } );
	allPairs.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Pinzu, Riichi::Face::Two }, } );
	allPairs.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Pinzu, Riichi::Face::Three }, } );
	allPairs.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Pinzu, Riichi::Face::Three }, } );
	allPairs.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Souzu, Riichi::Face::Two }, } );
	allPairs.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Souzu, Riichi::Face::Two }, } );
	allPairs.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Souzu, Riichi::Face::Three }, } );
	allPairs.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Souzu, Riichi::Face::Three }, } );
	allPairs.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Souzu, Riichi::Face::Three }, } );

	Riichi::HandAssessment allPairsAssessment( allPairs, yonma );

	Riichi::Hand seqTrip;
	seqTrip.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Manzu, Riichi::Face::One }, } );
	seqTrip.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Manzu, Riichi::Face::One }, } );
	seqTrip.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Manzu, Riichi::Face::One }, } );
	seqTrip.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Manzu, Riichi::Face::Two }, } );
	seqTrip.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Manzu, Riichi::Face::Six }, } );
	seqTrip.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Manzu, Riichi::Face::Six }, } );
	seqTrip.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Manzu, Riichi::Face::Six }, } );
	seqTrip.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Pinzu, Riichi::Face::Six }, } );
	seqTrip.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Pinzu, Riichi::Face::Six }, } );
	seqTrip.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Pinzu, Riichi::Face::Six }, } );
	seqTrip.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Souzu, Riichi::Face::Six }, } );
	seqTrip.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Souzu, Riichi::Face::Six }, } );
	seqTrip.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Souzu, Riichi::Face::Six }, } );

	Riichi::HandAssessment seqTripAssessment( seqTrip, yonma );

	Riichi::Hand multiInterpFewGroups;
	seqTrip.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Manzu, Riichi::Face::Two }, } );
	seqTrip.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Manzu, Riichi::Face::Three }, } );
	seqTrip.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Manzu, Riichi::Face::Seven }, } );
	seqTrip.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Manzu, Riichi::Face::Eight }, } );
	seqTrip.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Manzu, Riichi::Face::Nine }, } );
	seqTrip.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Manzu, Riichi::Face::Nine }, } );
	seqTrip.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Manzu, Riichi::Face::Nine }, } );
	seqTrip.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Pinzu, Riichi::Face::Six }, } );
	seqTrip.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Souzu, Riichi::Face::Two }, } );
	seqTrip.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Souzu, Riichi::Face::Six }, } );
	seqTrip.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Souzu, Riichi::Face::Eight }, } );
	seqTrip.AddFreeTiles( { Riichi::Tile{ Riichi::Face::Chun } } );
	seqTrip.AddFreeTiles( { Riichi::Tile{ Riichi::Face::Chun } } );

	Riichi::Hand multiInterpNoWaits;
	seqTrip.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Manzu, Riichi::Face::Three }, } );
	seqTrip.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Manzu, Riichi::Face::Four }, } );
	seqTrip.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Manzu, Riichi::Face::Five }, } );
	seqTrip.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Pinzu, Riichi::Face::Two }, } );
	seqTrip.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Pinzu, Riichi::Face::Five }, } );
	seqTrip.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Pinzu, Riichi::Face::Six }, } );
	seqTrip.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Pinzu, Riichi::Face::Seven }, } );
	seqTrip.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Souzu, Riichi::Face::Seven }, } );
	seqTrip.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Souzu, Riichi::Face::Eight }, } );
	seqTrip.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Souzu, Riichi::Face::Eight }, } );
	seqTrip.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Souzu, Riichi::Face::Eight }, } );
	seqTrip.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Souzu, Riichi::Face::Nine }, } );
	seqTrip.AddFreeTiles( { Riichi::Tile{ Riichi::Face::Chun } } );

	Riichi::Hand multiInterpSomeWaits;
	seqTrip.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Manzu, Riichi::Face::Three }, } );
	seqTrip.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Manzu, Riichi::Face::Four }, } );
	seqTrip.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Manzu, Riichi::Face::Five }, } );
	seqTrip.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Pinzu, Riichi::Face::Three }, } );
	seqTrip.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Pinzu, Riichi::Face::Four }, } );
	seqTrip.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Pinzu, Riichi::Face::Five }, } );
	seqTrip.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Pinzu, Riichi::Face::Five }, } );
	seqTrip.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Pinzu, Riichi::Face::Five }, } );
	seqTrip.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Souzu, Riichi::Face::Three }, } );
	seqTrip.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Souzu, Riichi::Face::Four }, } );
	seqTrip.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Souzu, Riichi::Face::Six }, } );
	seqTrip.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Souzu, Riichi::Face::Seven }, } );
	seqTrip.AddFreeTiles( { Riichi::Tile{ Riichi::Suit::Souzu, Riichi::Face::Eight }, } );

	Riichi::ShuffleRNG shuffleRNG{ std::random_device()( ) };
	while ( true )
	{
		Riichi::Vector<Riichi::TileInstance> tileSet = yonma.Tileset();
		ranges::shuffle( tileSet, shuffleRNG );
		tileSet.erase( tileSet.begin() + 13, tileSet.end() );

		Riichi::Hand hand;
		hand.AddFreeTiles( tileSet );

		Riichi::HandAssessment ass( hand, yonma );
		bool const unequalGroups = ranges::adjacent_find( ass.Interpretations(),
			[]( Riichi::HandInterpretation const& i_a, Riichi::HandInterpretation const& i_b ) -> bool
			{
				return i_a.m_groups.size() != i_b.m_groups.size() && i_b.m_groups.size() >= 3 && i_a.m_groups.size() >= 3;
			}
		) != ass.Interpretations().end();
		bool const hasWaits = ranges::find_if( ass.Interpretations(),
			[]( Riichi::HandInterpretation const& i_i ) -> bool
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
