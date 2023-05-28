#include "Riichi.hpp"

#include "riichi/Rules_Standard.hpp"

#include <iostream>
#include <random>

int main()
{
	Riichi::StandardYonma<Riichi::Seat::East> yonma;

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

	Riichi::HandAssessment nineGatesAssessment( nineGatesHand, yonma );

	Riichi::Hand threeFourHand;
	threeFourHand.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Manzu, 1>(), } );
	threeFourHand.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Manzu, 1>(), } );
	threeFourHand.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Manzu, 2>(), } );
	threeFourHand.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Manzu, 2>(), } );
	threeFourHand.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Manzu, 2>(), } );
	threeFourHand.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Manzu, 3>(), } );
	threeFourHand.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Manzu, 3>(), } );
	threeFourHand.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Manzu, 3>(), } );
	threeFourHand.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Manzu, 9>(), } );
	threeFourHand.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Manzu, 9>(), } );
	threeFourHand.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Manzu, 5>(), } );
	threeFourHand.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Manzu, 5>(), } );
	threeFourHand.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Manzu, 5>(), } );

	Riichi::HandAssessment threeFourAssessment( threeFourHand, yonma );

	Riichi::Hand allPairs;
	allPairs.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Manzu, 2>(), } );
	allPairs.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Manzu, 2>(), } );
	allPairs.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Manzu, 3>(), } );
	allPairs.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Manzu, 3>(), } );
	allPairs.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Pinzu, 2>(), } );
	allPairs.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Pinzu, 2>(), } );
	allPairs.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Pinzu, 3>(), } );
	allPairs.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Pinzu, 3>(), } );
	allPairs.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Souzu, 2>(), } );
	allPairs.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Souzu, 2>(), } );
	allPairs.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Souzu, 3>(), } );
	allPairs.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Souzu, 3>(), } );
	allPairs.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Souzu, 3>(), } );

	Riichi::HandAssessment allPairsAssessment( allPairs, yonma );

	Riichi::Hand seqTrip;
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Manzu, 1>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Manzu, 1>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Manzu, 1>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Manzu, 2>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Manzu, 6>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Manzu, 6>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Manzu, 6>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Pinzu, 6>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Pinzu, 6>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Pinzu, 6>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Souzu, 6>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Souzu, 6>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Souzu, 6>(), } );

	Riichi::HandAssessment seqTripAssessment( seqTrip, yonma );

	Riichi::Hand multiInterpFewGroups;
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Manzu, 2>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Manzu, 3>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Manzu, 7>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Manzu, 8>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Manzu, 9>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Manzu, 9>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Manzu, 9>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Pinzu, 6>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Souzu, 2>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Souzu, 6>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Souzu, 8>(), } );
	seqTrip.AddFreeTiles( { Riichi::DragonTileType::Red } );
	seqTrip.AddFreeTiles( { Riichi::DragonTileType::Red } );

	Riichi::Hand multiInterpNoWaits;
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Manzu, 3>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Manzu, 4>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Manzu, 5>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Pinzu, 2>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Pinzu, 5>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Pinzu, 6>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Pinzu, 7>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Souzu, 7>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Souzu, 8>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Souzu, 8>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Souzu, 8>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Souzu, 9>(), } );
	seqTrip.AddFreeTiles( { Riichi::DragonTileType::Red } );

	Riichi::Hand multiInterpSomeWaits;
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Manzu, 3>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Manzu, 4>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Manzu, 5>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Pinzu, 3>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Pinzu, 4>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Pinzu, 5>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Pinzu, 5>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Pinzu, 5>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Souzu, 3>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Souzu, 4>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Souzu, 6>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Souzu, 7>(), } );
	seqTrip.AddFreeTiles( { Riichi::SuitTile::Make<Riichi::Suit::Souzu, 8>(), } );

	Riichi::ShuffleRNG shuffleRNG{ std::random_device()( ) };
	while ( true )
	{
		Riichi::Vector<Riichi::Tile> tileSet = yonma.Tileset();
		std::ranges::shuffle( tileSet, shuffleRNG );
		tileSet.erase( tileSet.begin() + 13, tileSet.end() );

		Riichi::Hand hand;
		hand.AddFreeTiles( tileSet );

		Riichi::HandAssessment ass( hand, yonma );
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

	return 0;
}
