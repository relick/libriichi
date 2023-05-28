#include "Riichi.hpp"

#include "riichi/Random.hpp"
#include "riichi/Round.hpp"
#include "riichi/Rules_Standard.hpp"
#include "riichi/Yaku_Standard.hpp"

int main()
{
	using namespace Riichi;

	// Set up mock
	StandardYonma rules;
	ShuffleRNG shuffleRNG( 0 );
	Round mockRound( Seat::East, { PlayerID{} }, rules, shuffleRNG );

	auto fnTestForYaku = [ & ]( Yaku const& i_yaku, Hand const& i_hand, TileDraw const& i_lastTile )
	{
		HandAssessment assessment( i_hand, rules );
		for ( auto const& interp : assessment.Interpretations() )
		{
			if ( interp.m_waitType != WaitType::None )
			{
				if ( i_yaku.CalculateValue( mockRound, Seat::East, i_hand, assessment, interp, i_lastTile ).IsValid() )
				{
					return true;
				}
			}
		}

		return false;
	};


	{
		Hand ikkitsuukanSuccess;
		ikkitsuukanSuccess.AddFreeTiles( {
			SuitTile{ Suit::Manzu, SuitTileValue::Set<1>() },
			SuitTile{ Suit::Manzu, SuitTileValue::Set<2>() },
			SuitTile{ Suit::Manzu, SuitTileValue::Set<3>() },
			SuitTile{ Suit::Manzu, SuitTileValue::Set<9>() },
			SuitTile{ Suit::Manzu, SuitTileValue::Set<9>() },
			SuitTile{ Suit::Pinzu, SuitTileValue::Set<1>() },
			SuitTile{ Suit::Pinzu, SuitTileValue::Set<2>() },
			SuitTile{ Suit::Pinzu, SuitTileValue::Set<3>() },
			SuitTile{ Suit::Pinzu, SuitTileValue::Set<4>() },
			SuitTile{ Suit::Pinzu, SuitTileValue::Set<6>() },
			SuitTile{ Suit::Pinzu, SuitTileValue::Set<7>() },
			SuitTile{ Suit::Pinzu, SuitTileValue::Set<8>() },
			SuitTile{ Suit::Pinzu, SuitTileValue::Set<9>() },
			} );

		Ensure(
			fnTestForYaku(
				StandardYaku::Ikkitsuukan(),
				ikkitsuukanSuccess,
				{ SuitTile{ Suit::Pinzu, SuitTileValue::Set<5>() }, TileDrawType::SelfDraw }
			),
			"Ikkitsuukan failed!" );
	}

	return 0;
}
