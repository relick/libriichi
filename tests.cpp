#include "Riichi.hpp"

#include "riichi/Random.hpp"
#include "riichi/Round.hpp"
#include "riichi/Rules_Standard.hpp"
#include "riichi/Yaku_Standard.hpp"

void TestYaku()
{
	using namespace Riichi;

	// Set up mock
	Seat mockSeat{ Seat::East };
	Vector<PlayerID> mockPlayers{ PlayerID{}, PlayerID{}, PlayerID{}, PlayerID{} };
	StandardYonma mockRules;
	ShuffleRNG mockRNG( 0 );
	Round mockRound( mockSeat, mockPlayers, mockRules, mockRNG );

	auto fnTestForYaku = [ & ]( Yaku const& i_yaku, Hand const& i_hand, TileDraw const& i_lastTile, Round const* i_mockRound = nullptr )
	{
		HandAssessment assessment( i_hand, mockRules );
		for ( auto const& interp : assessment.Interpretations() )
		{
			if ( interp.m_waitType != WaitType::None )
			{
				if ( i_yaku.CalculateValue( i_mockRound ? *i_mockRound : mockRound, mockSeat, i_hand, assessment, interp, i_lastTile ).IsValid() )
				{
					return true;
				}
			}
		}

		return false;
	};

	// MenzenchinTsumohou
	{
		Hand failHand;
		failHand.AddFreeTiles( {
			SuitTile::Make<Suit::Manzu, 1>(),
			SuitTile::Make<Suit::Manzu, 2>(),
			SuitTile::Make<Suit::Manzu, 3>(),
			SuitTile::Make<Suit::Manzu, 9>(),
			SuitTile::Make<Suit::Manzu, 9>(),
			SuitTile::Make<Suit::Pinzu, 1>(),
			SuitTile::Make<Suit::Pinzu, 2>(),
			SuitTile::Make<Suit::Pinzu, 3>(),
			SuitTile::Make<Suit::Pinzu, 3>(),
			SuitTile::Make<Suit::Pinzu, 5>(),
			SuitTile::Make<Suit::Pinzu, 7>(),
			SuitTile::Make<Suit::Pinzu, 8>(),
			SuitTile::Make<Suit::Pinzu, 9>(),
			} );

		bool const fail = fnTestForYaku(
			StandardYaku::MenzenchinTsumohou(),
			failHand,
			{ SuitTile::Make<Suit::Pinzu, 4>(), TileDrawType::DiscardDraw }
		);
		Ensure( !fail, "MenzenchinTsumohou failed!" );

		Hand successHand;
		successHand.AddFreeTiles( {
			SuitTile::Make<Suit::Manzu, 1>(),
			SuitTile::Make<Suit::Manzu, 2>(),
			SuitTile::Make<Suit::Manzu, 3>(),
			SuitTile::Make<Suit::Manzu, 9>(),
			SuitTile::Make<Suit::Manzu, 9>(),
			SuitTile::Make<Suit::Pinzu, 1>(),
			SuitTile::Make<Suit::Pinzu, 2>(),
			SuitTile::Make<Suit::Pinzu, 3>(),
			SuitTile::Make<Suit::Pinzu, 4>(),
			SuitTile::Make<Suit::Pinzu, 6>(),
			SuitTile::Make<Suit::Pinzu, 7>(),
			SuitTile::Make<Suit::Pinzu, 8>(),
			SuitTile::Make<Suit::Pinzu, 9>(),
			} );

		bool const success = fnTestForYaku(
			StandardYaku::MenzenchinTsumohou(),
			successHand,
			{ SuitTile::Make<Suit::Pinzu, 5>(), TileDrawType::SelfDraw }
		);
		Ensure( success, "MenzenchinTsumohou failed!" );
	}

	// Riichi
	{
		Hand failHand;
		failHand.AddFreeTiles( {
			SuitTile::Make<Suit::Manzu, 1>(),
			SuitTile::Make<Suit::Manzu, 2>(),
			SuitTile::Make<Suit::Manzu, 3>(),
			SuitTile::Make<Suit::Manzu, 9>(),
			SuitTile::Make<Suit::Manzu, 9>(),
			SuitTile::Make<Suit::Pinzu, 1>(),
			SuitTile::Make<Suit::Pinzu, 2>(),
			SuitTile::Make<Suit::Pinzu, 3>(),
			SuitTile::Make<Suit::Pinzu, 3>(),
			SuitTile::Make<Suit::Pinzu, 5>(),
			SuitTile::Make<Suit::Pinzu, 7>(),
			SuitTile::Make<Suit::Pinzu, 8>(),
			SuitTile::Make<Suit::Pinzu, 9>(),
			} );

		bool const fail = fnTestForYaku(
			StandardYaku::Riichi(),
			failHand,
			{ SuitTile::Make<Suit::Pinzu, 4>(), TileDrawType::SelfDraw }
		);
		Ensure( !fail, "Riichi failed!" );

		Round riichiMockRound( mockSeat, mockPlayers, mockRules, mockRNG );
		riichiMockRound.DealHands();
		riichiMockRound.Riichi( std::nullopt );

		Hand successHand;
		successHand.AddFreeTiles( {
			SuitTile::Make<Suit::Manzu, 1>(),
			SuitTile::Make<Suit::Manzu, 2>(),
			SuitTile::Make<Suit::Manzu, 3>(),
			SuitTile::Make<Suit::Manzu, 9>(),
			SuitTile::Make<Suit::Manzu, 9>(),
			SuitTile::Make<Suit::Pinzu, 1>(),
			SuitTile::Make<Suit::Pinzu, 2>(),
			SuitTile::Make<Suit::Pinzu, 3>(),
			SuitTile::Make<Suit::Pinzu, 4>(),
			SuitTile::Make<Suit::Pinzu, 6>(),
			SuitTile::Make<Suit::Pinzu, 7>(),
			SuitTile::Make<Suit::Pinzu, 8>(),
			SuitTile::Make<Suit::Pinzu, 9>(),
			} );

		bool const success = fnTestForYaku(
			StandardYaku::Riichi(),
			successHand,
			{ SuitTile::Make<Suit::Pinzu, 5>(), TileDrawType::SelfDraw },
			&riichiMockRound
		);
		Ensure( success, "Riichi failed!" );
	}

	// Ippatsu
	{
		// Checking we are in riichi but not ippatsu
		{
			Round riichiMockRound( mockSeat, mockPlayers, mockRules, mockRNG );
			riichiMockRound.DealHands();
			riichiMockRound.Riichi( std::nullopt );
			
			// Then advance turns back past the player
			for ( size_t i = 0; i < 8; ++i )
			{
				riichiMockRound.PassCalls( SeatSet{} );
				riichiMockRound.Discard( std::nullopt );
			}

			Hand failHand;
			failHand.AddFreeTiles( {
				SuitTile::Make<Suit::Manzu, 1>(),
				SuitTile::Make<Suit::Manzu, 2>(),
				SuitTile::Make<Suit::Manzu, 3>(),
				SuitTile::Make<Suit::Manzu, 9>(),
				SuitTile::Make<Suit::Manzu, 9>(),
				SuitTile::Make<Suit::Pinzu, 1>(),
				SuitTile::Make<Suit::Pinzu, 2>(),
				SuitTile::Make<Suit::Pinzu, 3>(),
				SuitTile::Make<Suit::Pinzu, 3>(),
				SuitTile::Make<Suit::Pinzu, 5>(),
				SuitTile::Make<Suit::Pinzu, 7>(),
				SuitTile::Make<Suit::Pinzu, 8>(),
				SuitTile::Make<Suit::Pinzu, 9>(),
				} );

			bool const fail = fnTestForYaku(
				StandardYaku::Ippatsu(),
				failHand,
				{ SuitTile::Make<Suit::Pinzu, 4>(), TileDrawType::SelfDraw },
				&riichiMockRound
			);
			Ensure( !fail, "Ippatsu failed!" );
		}

		{
			Round riichiMockRound( mockSeat, mockPlayers, mockRules, mockRNG );
			riichiMockRound.DealHands();
			riichiMockRound.Riichi( std::nullopt );

			Hand successHand;
			successHand.AddFreeTiles( {
				SuitTile::Make<Suit::Manzu, 1>(),
				SuitTile::Make<Suit::Manzu, 2>(),
				SuitTile::Make<Suit::Manzu, 3>(),
				SuitTile::Make<Suit::Manzu, 9>(),
				SuitTile::Make<Suit::Manzu, 9>(),
				SuitTile::Make<Suit::Pinzu, 1>(),
				SuitTile::Make<Suit::Pinzu, 2>(),
				SuitTile::Make<Suit::Pinzu, 3>(),
				SuitTile::Make<Suit::Pinzu, 4>(),
				SuitTile::Make<Suit::Pinzu, 6>(),
				SuitTile::Make<Suit::Pinzu, 7>(),
				SuitTile::Make<Suit::Pinzu, 8>(),
				SuitTile::Make<Suit::Pinzu, 9>(),
				} );

			bool const success = fnTestForYaku(
				StandardYaku::Ippatsu(),
				successHand,
				{ SuitTile::Make<Suit::Pinzu, 5>(), TileDrawType::SelfDraw },
				&riichiMockRound
			);
			Ensure( success, "Ippatsu failed!" );
		}
	}

	// Pinfu
	{
		// Identical hands except different wait types

		Hand failHand;
		failHand.AddFreeTiles( {
			SuitTile::Make<Suit::Manzu, 1>(),
			SuitTile::Make<Suit::Manzu, 2>(),
			SuitTile::Make<Suit::Manzu, 3>(),
			SuitTile::Make<Suit::Manzu, 4>(),
			SuitTile::Make<Suit::Manzu, 5>(),
			SuitTile::Make<Suit::Manzu, 6>(),
			SuitTile::Make<Suit::Manzu, 4>(),
			SuitTile::Make<Suit::Manzu, 5>(),
			SuitTile::Make<Suit::Manzu, 6>(),
			SuitTile::Make<Suit::Manzu, 7>(),
			SuitTile::Make<Suit::Manzu, 7>(),
			SuitTile::Make<Suit::Pinzu, 3>(),
			SuitTile::Make<Suit::Pinzu, 6>(),
			} );

		bool const fail = fnTestForYaku(
			StandardYaku::Pinfu(),
			failHand,
			{ SuitTile::Make<Suit::Pinzu, 5>(), TileDrawType::SelfDraw }
		);
		Ensure( !fail, "Pinfu failed!" );

		Hand successHand;
		successHand.AddFreeTiles( {
			SuitTile::Make<Suit::Manzu, 1>(),
			SuitTile::Make<Suit::Manzu, 2>(),
			SuitTile::Make<Suit::Manzu, 3>(),
			SuitTile::Make<Suit::Manzu, 4>(),
			SuitTile::Make<Suit::Manzu, 5>(),
			SuitTile::Make<Suit::Manzu, 6>(),
			SuitTile::Make<Suit::Manzu, 4>(),
			SuitTile::Make<Suit::Manzu, 5>(),
			SuitTile::Make<Suit::Manzu, 6>(),
			SuitTile::Make<Suit::Manzu, 7>(),
			SuitTile::Make<Suit::Manzu, 7>(),
			SuitTile::Make<Suit::Pinzu, 3>(),
			SuitTile::Make<Suit::Pinzu, 4>(),
			} );

		bool const success = fnTestForYaku(
			StandardYaku::Pinfu(),
			successHand,
			{ SuitTile::Make<Suit::Pinzu, 5>(), TileDrawType::SelfDraw }
		);
		Ensure( success, "Pinfu failed!" );
	}

	// Iipeikou
	{
		Hand failHand;
		failHand.AddFreeTiles( {
			SuitTile::Make<Suit::Manzu, 1>(),
			SuitTile::Make<Suit::Manzu, 2>(),
			SuitTile::Make<Suit::Manzu, 3>(),
			SuitTile::Make<Suit::Manzu, 2>(),
			SuitTile::Make<Suit::Manzu, 3>(),
			SuitTile::Make<Suit::Pinzu, 3>(),
			SuitTile::Make<Suit::Pinzu, 3>(),
			SuitTile::Make<Suit::Pinzu, 3>(),
			SuitTile::Make<Suit::Pinzu, 4>(),
			SuitTile::Make<Suit::Pinzu, 4>(),
			SuitTile::Make<Suit::Pinzu, 4>(),
			SuitTile::Make<Suit::Pinzu, 6>(),
			SuitTile::Make<Suit::Pinzu, 6>(),
			} );

		bool const fail = fnTestForYaku(
			StandardYaku::Iipeikou(),
			failHand,
			{ SuitTile::Make<Suit::Manzu, 4>(), TileDrawType::DiscardDraw }
		);
		Ensure( !fail, "Iipeikou failed!" );

		Hand successHand;
		successHand.AddFreeTiles( {
			SuitTile::Make<Suit::Manzu, 1>(),
			SuitTile::Make<Suit::Manzu, 2>(),
			SuitTile::Make<Suit::Manzu, 3>(),
			SuitTile::Make<Suit::Manzu, 2>(),
			SuitTile::Make<Suit::Manzu, 3>(),
			SuitTile::Make<Suit::Pinzu, 3>(),
			SuitTile::Make<Suit::Pinzu, 3>(),
			SuitTile::Make<Suit::Pinzu, 3>(),
			SuitTile::Make<Suit::Pinzu, 4>(),
			SuitTile::Make<Suit::Pinzu, 4>(),
			SuitTile::Make<Suit::Pinzu, 4>(),
			SuitTile::Make<Suit::Pinzu, 6>(),
			SuitTile::Make<Suit::Pinzu, 6>(),
			} );

		bool const success = fnTestForYaku(
			StandardYaku::Iipeikou(),
			successHand,
			{ SuitTile::Make<Suit::Manzu, 1>(), TileDrawType::DiscardDraw }
		);
		Ensure( success, "Iipeikou failed!" );
	}

	// TODO-TEST: HaiteiRaoyue
	// TODO-TEST: HouteiRaoyui
	// TODO-TEST: RinshanKaihou
	// TODO-TEST: Chankan
	// TODO-TEST: Tanyao<>
	// TODO-TEST: Yakuhai_Haku
	// TODO-TEST: Yakuhai_Hatsu
	// TODO-TEST: Yakuhai_Chun
	// TODO-TEST: Bakaze
	// TODO-TEST: Jikaze
	// TODO-TEST: DoubleRiichi
	// TODO-TEST: Chantaiyao
	// TODO-TEST: SanshokuDoujun

	// Ikkitsuukan
	{
		Hand failHand;
		failHand.AddFreeTiles( {
			SuitTile::Make<Suit::Manzu, 1>(),
			SuitTile::Make<Suit::Manzu, 2>(),
			SuitTile::Make<Suit::Manzu, 3>(),
			SuitTile::Make<Suit::Manzu, 9>(),
			SuitTile::Make<Suit::Manzu, 9>(),
			SuitTile::Make<Suit::Pinzu, 1>(),
			SuitTile::Make<Suit::Pinzu, 2>(),
			SuitTile::Make<Suit::Pinzu, 3>(),
			SuitTile::Make<Suit::Pinzu, 3>(),
			SuitTile::Make<Suit::Pinzu, 5>(),
			SuitTile::Make<Suit::Pinzu, 7>(),
			SuitTile::Make<Suit::Pinzu, 8>(),
			SuitTile::Make<Suit::Pinzu, 9>(),
			} );

		bool const fail = fnTestForYaku(
			StandardYaku::Ikkitsuukan(),
			failHand,
			{ SuitTile::Make<Suit::Pinzu, 4>(), TileDrawType::SelfDraw }
		);
		Ensure( !fail, "Ikkitsuukan failed!" );

		Hand successHand;
		successHand.AddFreeTiles( {
			SuitTile::Make<Suit::Manzu, 1>(),
			SuitTile::Make<Suit::Manzu, 2>(),
			SuitTile::Make<Suit::Manzu, 3>(),
			SuitTile::Make<Suit::Manzu, 9>(),
			SuitTile::Make<Suit::Manzu, 9>(),
			SuitTile::Make<Suit::Pinzu, 1>(),
			SuitTile::Make<Suit::Pinzu, 2>(),
			SuitTile::Make<Suit::Pinzu, 3>(),
			SuitTile::Make<Suit::Pinzu, 4>(),
			SuitTile::Make<Suit::Pinzu, 6>(),
			SuitTile::Make<Suit::Pinzu, 7>(),
			SuitTile::Make<Suit::Pinzu, 8>(),
			SuitTile::Make<Suit::Pinzu, 9>(),
			} );

		bool const success = fnTestForYaku(
			StandardYaku::Ikkitsuukan(),
			successHand,
			{ SuitTile::Make<Suit::Pinzu, 5>(), TileDrawType::SelfDraw }
		);
		Ensure( success, "Ikkitsuukan failed!" );
	}

	// TODO-TEST: Toitoi
	// TODO-TEST: Sanankou
	// TODO-TEST: SanshokuDoukou
	// TODO-TEST: Sankantsu
	// TODO-TEST: Chiitoitsu
	// TODO-TEST: Honroutou
	// TODO-TEST: Shousangen
	// TODO-TEST: Honitsu
	// TODO-TEST: JunchanTaiyao
	// TODO-TEST: Ryanpeikou
	// TODO-TEST: Chinitsu
	// TODO-TEST: KokushiMusou
	// TODO-TEST: Suuankou
	// TODO-TEST: Daisangen
	// TODO-TEST: Shousuushii
	// TODO-TEST: Daisuushii
	// TODO-TEST: Tsuuiisou
	// TODO-TEST: Chinroutou
	// TODO-TEST: Ryuuiisou
	// TODO-TEST: ChuurenPoutou
	// TODO-TEST: Suukantsu
	// TODO-TEST: Tenhou
	// TODO-TEST: Chihou
}

int main()
{
	TestYaku();

	return 0;
}
