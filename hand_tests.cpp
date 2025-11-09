#include "Riichi.hpp"

#include "riichi/Rules_Standard.hpp"

#include <iostream>
#include <random>
#include "range/v3/algorithm.hpp"

int main()
{
	Riichi::StandardYonma<Riichi::Seat::East> yonma;

	Riichi::Hand nineGatesHand;
	for ( Riichi::Number num : Riichi::Numbers{} )
	{
		Riichi::SuitTile const tile{Riichi::Suit::Manzu, num};
		if ( num == Riichi::Number::One || num == Riichi::Number::Nine )
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
	threeFourHand.AddFreeTiles( { Riichi::SuitTile::One<Riichi::Suit::Manzu>(), } );
	threeFourHand.AddFreeTiles( { Riichi::SuitTile::One<Riichi::Suit::Manzu>(), } );
	threeFourHand.AddFreeTiles( { Riichi::SuitTile::Two<Riichi::Suit::Manzu>(), } );
	threeFourHand.AddFreeTiles( { Riichi::SuitTile::Two<Riichi::Suit::Manzu>(), } );
	threeFourHand.AddFreeTiles( { Riichi::SuitTile::Two<Riichi::Suit::Manzu>(), } );
	threeFourHand.AddFreeTiles( { Riichi::SuitTile::Three<Riichi::Suit::Manzu>(), } );
	threeFourHand.AddFreeTiles( { Riichi::SuitTile::Three<Riichi::Suit::Manzu>(), } );
	threeFourHand.AddFreeTiles( { Riichi::SuitTile::Three<Riichi::Suit::Manzu>(), } );
	threeFourHand.AddFreeTiles( { Riichi::SuitTile::Nine<Riichi::Suit::Manzu>(), } );
	threeFourHand.AddFreeTiles( { Riichi::SuitTile::Nine<Riichi::Suit::Manzu>(), } );
	threeFourHand.AddFreeTiles( { Riichi::SuitTile::Five<Riichi::Suit::Manzu>(), } );
	threeFourHand.AddFreeTiles( { Riichi::SuitTile::Five<Riichi::Suit::Manzu>(), } );
	threeFourHand.AddFreeTiles( { Riichi::SuitTile::Five<Riichi::Suit::Manzu>(), } );

	Riichi::HandAssessment threeFourAssessment( threeFourHand, yonma );

	Riichi::Hand allPairs;
	allPairs.AddFreeTiles( { Riichi::SuitTile::Two<Riichi::Suit::Manzu>(), } );
	allPairs.AddFreeTiles( { Riichi::SuitTile::Two<Riichi::Suit::Manzu>(), } );
	allPairs.AddFreeTiles( { Riichi::SuitTile::Three<Riichi::Suit::Manzu>(), } );
	allPairs.AddFreeTiles( { Riichi::SuitTile::Three<Riichi::Suit::Manzu>(), } );
	allPairs.AddFreeTiles( { Riichi::SuitTile::Two<Riichi::Suit::Pinzu>(), } );
	allPairs.AddFreeTiles( { Riichi::SuitTile::Two<Riichi::Suit::Pinzu>(), } );
	allPairs.AddFreeTiles( { Riichi::SuitTile::Three<Riichi::Suit::Pinzu>(), } );
	allPairs.AddFreeTiles( { Riichi::SuitTile::Three<Riichi::Suit::Pinzu>(), } );
	allPairs.AddFreeTiles( { Riichi::SuitTile::Two<Riichi::Suit::Souzu>(), } );
	allPairs.AddFreeTiles( { Riichi::SuitTile::Two<Riichi::Suit::Souzu>(), } );
	allPairs.AddFreeTiles( { Riichi::SuitTile::Three<Riichi::Suit::Souzu>(), } );
	allPairs.AddFreeTiles( { Riichi::SuitTile::Three<Riichi::Suit::Souzu>(), } );
	allPairs.AddFreeTiles( { Riichi::SuitTile::Three<Riichi::Suit::Souzu>(), } );

	Riichi::HandAssessment allPairsAssessment( allPairs, yonma );

	Riichi::Hand seqTrip;
	seqTrip.AddFreeTiles( { Riichi::SuitTile::One<Riichi::Suit::Manzu>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::One<Riichi::Suit::Manzu>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::One<Riichi::Suit::Manzu>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Two<Riichi::Suit::Manzu>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Six<Riichi::Suit::Manzu>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Six<Riichi::Suit::Manzu>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Six<Riichi::Suit::Manzu>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Six<Riichi::Suit::Pinzu>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Six<Riichi::Suit::Pinzu>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Six<Riichi::Suit::Pinzu>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Six<Riichi::Suit::Souzu>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Six<Riichi::Suit::Souzu>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Six<Riichi::Suit::Souzu>(), } );

	Riichi::HandAssessment seqTripAssessment( seqTrip, yonma );

	Riichi::Hand multiInterpFewGroups;
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Two<Riichi::Suit::Manzu>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Three<Riichi::Suit::Manzu>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Seven<Riichi::Suit::Manzu>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Eight<Riichi::Suit::Manzu>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Nine<Riichi::Suit::Manzu>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Nine<Riichi::Suit::Manzu>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Nine<Riichi::Suit::Manzu>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Six<Riichi::Suit::Pinzu>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Two<Riichi::Suit::Souzu>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Six<Riichi::Suit::Souzu>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Eight<Riichi::Suit::Souzu>(), } );
	seqTrip.AddFreeTiles( { Riichi::DragonTileType::Red } );
	seqTrip.AddFreeTiles( { Riichi::DragonTileType::Red } );

	Riichi::Hand multiInterpNoWaits;
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Three<Riichi::Suit::Manzu>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Four<Riichi::Suit::Manzu>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Five<Riichi::Suit::Manzu>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Two<Riichi::Suit::Pinzu>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Five<Riichi::Suit::Pinzu>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Six<Riichi::Suit::Pinzu>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Seven<Riichi::Suit::Pinzu>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Seven<Riichi::Suit::Souzu>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Eight<Riichi::Suit::Souzu>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Eight<Riichi::Suit::Souzu>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Eight<Riichi::Suit::Souzu>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Nine<Riichi::Suit::Souzu>(), } );
	seqTrip.AddFreeTiles( { Riichi::DragonTileType::Red } );

	Riichi::Hand multiInterpSomeWaits;
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Three<Riichi::Suit::Manzu>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Four<Riichi::Suit::Manzu>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Five<Riichi::Suit::Manzu>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Three<Riichi::Suit::Pinzu>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Four<Riichi::Suit::Pinzu>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Five<Riichi::Suit::Pinzu>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Five<Riichi::Suit::Pinzu>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Five<Riichi::Suit::Pinzu>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Three<Riichi::Suit::Souzu>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Four<Riichi::Suit::Souzu>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Six<Riichi::Suit::Souzu>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Seven<Riichi::Suit::Souzu>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Eight<Riichi::Suit::Souzu>(), } );

	Riichi::ShuffleRNG shuffleRNG{ std::random_device()( ) };
	while ( true )
	{
		Riichi::Vector<Riichi::Tile> tileSet = yonma.Tileset();
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
