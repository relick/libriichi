#include "Riichi.hpp"

#include "riichi/Rules_Standard.hpp"

#include <iostream>
#include <random>
#include "range/v3/algorithm.hpp"

int main()
{
	using namespace Riichi;

	uint32_t tileInstanceID = 0;

	StandardYonma<Seat::East> yonma;

	Hand nineGatesHand;
	for ( Face num : Numbers{} )
	{
		TileInstance const tile{ { Suit::Manzu, num }, tileInstanceID++, };
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
	threeFourHand.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::One }, tileInstanceID++, } } );
	threeFourHand.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::One }, tileInstanceID++, } } );
	threeFourHand.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::Two }, tileInstanceID++, } } );
	threeFourHand.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::Two }, tileInstanceID++, } } );
	threeFourHand.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::Two }, tileInstanceID++, } } );
	threeFourHand.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::Three }, tileInstanceID++, } } );
	threeFourHand.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::Three }, tileInstanceID++, } } );
	threeFourHand.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::Three }, tileInstanceID++, } } );
	threeFourHand.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::Nine }, tileInstanceID++, } } );
	threeFourHand.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::Nine }, tileInstanceID++, } } );
	threeFourHand.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::Five }, tileInstanceID++, } } );
	threeFourHand.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::Five }, tileInstanceID++, } } );
	threeFourHand.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::Five }, tileInstanceID++, } } );

	HandAssessment threeFourAssessment( threeFourHand, yonma );

	Hand allPairs;
	allPairs.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::Two }, tileInstanceID++, } } );
	allPairs.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::Two }, tileInstanceID++, } } );
	allPairs.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::Three }, tileInstanceID++, } } );
	allPairs.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::Three }, tileInstanceID++, } } );
	allPairs.AddFreeTiles( { TileInstance{ { Suit::Pinzu, Face::Two }, tileInstanceID++, } } );
	allPairs.AddFreeTiles( { TileInstance{ { Suit::Pinzu, Face::Two }, tileInstanceID++, } } );
	allPairs.AddFreeTiles( { TileInstance{ { Suit::Pinzu, Face::Three }, tileInstanceID++, } } );
	allPairs.AddFreeTiles( { TileInstance{ { Suit::Pinzu, Face::Three }, tileInstanceID++, } } );
	allPairs.AddFreeTiles( { TileInstance{ { Suit::Souzu, Face::Two }, tileInstanceID++, } } );
	allPairs.AddFreeTiles( { TileInstance{ { Suit::Souzu, Face::Two }, tileInstanceID++, } } );
	allPairs.AddFreeTiles( { TileInstance{ { Suit::Souzu, Face::Three }, tileInstanceID++, } } );
	allPairs.AddFreeTiles( { TileInstance{ { Suit::Souzu, Face::Three }, tileInstanceID++, } } );
	allPairs.AddFreeTiles( { TileInstance{ { Suit::Souzu, Face::Three }, tileInstanceID++, } } );

	HandAssessment allPairsAssessment( allPairs, yonma );

	Hand seqTrip;
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::One }, tileInstanceID++, } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::One }, tileInstanceID++, } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::One }, tileInstanceID++, } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::Two }, tileInstanceID++, } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::Six }, tileInstanceID++, } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::Six }, tileInstanceID++, } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::Six }, tileInstanceID++, } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Pinzu, Face::Six }, tileInstanceID++, } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Pinzu, Face::Six }, tileInstanceID++, } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Pinzu, Face::Six }, tileInstanceID++, } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Souzu, Face::Six }, tileInstanceID++, } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Souzu, Face::Six }, tileInstanceID++, } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Souzu, Face::Six }, tileInstanceID++, } } );

	HandAssessment seqTripAssessment( seqTrip, yonma );

	Hand multiInterpFewGroups;
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::Two }, tileInstanceID++, } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::Three }, tileInstanceID++, } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::Seven }, tileInstanceID++, } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::Eight }, tileInstanceID++, } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::Nine }, tileInstanceID++, } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::Nine }, tileInstanceID++, } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::Nine }, tileInstanceID++, } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Pinzu, Face::Six }, tileInstanceID++, } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Souzu, Face::Two }, tileInstanceID++, } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Souzu, Face::Six }, tileInstanceID++, } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Souzu, Face::Eight }, tileInstanceID++, } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Face::Chun }, tileInstanceID++, } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Face::Chun }, tileInstanceID++, } } );

	Hand multiInterpNoWaits;
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::Three }, tileInstanceID++, } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::Four }, tileInstanceID++, } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::Five }, tileInstanceID++, } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Pinzu, Face::Two }, tileInstanceID++, } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Pinzu, Face::Five }, tileInstanceID++, } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Pinzu, Face::Six }, tileInstanceID++, } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Pinzu, Face::Seven }, tileInstanceID++, } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Souzu, Face::Seven }, tileInstanceID++, } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Souzu, Face::Eight }, tileInstanceID++, } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Souzu, Face::Eight }, tileInstanceID++, } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Souzu, Face::Eight }, tileInstanceID++, } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Souzu, Face::Nine }, tileInstanceID++, } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Face::Chun }, tileInstanceID++, } } );

	Hand multiInterpSomeWaits;
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::Three }, tileInstanceID++, } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::Four }, tileInstanceID++, } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Manzu, Face::Five }, tileInstanceID++, } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Pinzu, Face::Three }, tileInstanceID++, } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Pinzu, Face::Four }, tileInstanceID++, } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Pinzu, Face::Five }, tileInstanceID++, } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Pinzu, Face::Five }, tileInstanceID++, } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Pinzu, Face::Five }, tileInstanceID++, } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Souzu, Face::Three }, tileInstanceID++, } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Souzu, Face::Four }, tileInstanceID++, } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Souzu, Face::Six }, tileInstanceID++, } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Souzu, Face::Seven }, tileInstanceID++, } } );
	seqTrip.AddFreeTiles( { TileInstance{ { Suit::Souzu, Face::Eight }, tileInstanceID++, } } );

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
