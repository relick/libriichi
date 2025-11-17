#include "Riichi.hpp"

#include "riichi/Random.hpp"
#include "riichi/Round.hpp"
#include "riichi/Rules_Standard.hpp"
#include "riichi/Yaku_Standard.hpp"

void TestYaku()
{
	using namespace Riichi;

	TileInstanceIDGenerator generateID;

	// Set up mock
	Seat mockSeat{ Seat::East };
	Vector<PlayerID> mockPlayers{ PlayerID{}, PlayerID{}, PlayerID{}, PlayerID{} };
	StandardYonma<Seat::East> mockRules;
	ShuffleRNG mockRNG( 0 );
	Round mockRound( mockSeat, mockPlayers, mockRules, mockRNG );

	auto fnTestForYaku = [ & ]( YakuEvaluator const& i_yaku, Hand const& i_hand, TileDraw const& i_lastTile, Round const* i_mockRound = nullptr )
	{
		HandAssessment assessment( i_hand, mockRules );
		for ( auto const& interp : assessment.Interpretations() )
		{
			if ( interp.m_waitType != WaitType::None )
			{
				if ( i_yaku.CalculateValue(
					i_mockRound ? *i_mockRound : mockRound,
					mockSeat,
					i_hand,
					assessment,
					interp,
					i_lastTile.m_tile.Tile(),
					i_lastTile.m_type
				).IsValid() )
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
			TileInstance{ { Suit::Manzu, Face::One }, generateID(), },
			TileInstance{ { Suit::Manzu, Face::Two }, generateID(), },
			TileInstance{ { Suit::Manzu, Face::Three }, generateID(), },
			TileInstance{ { Suit::Manzu, Face::Nine }, generateID(), },
			TileInstance{ { Suit::Manzu, Face::Nine }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::One }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::Two }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::Three }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::Three }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::Five }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::Seven }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::Eight }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::Nine }, generateID(), },
			} );

		bool const fail = fnTestForYaku(
			StandardYaku::MenzenchinTsumohou(),
			failHand,
			{ TileInstance{ { Suit::Pinzu, Face::Four }, generateID(), }, TileDrawType::DiscardDraw }
		);
		riEnsure( !fail, "MenzenchinTsumohou failed!" );

		Hand successHand;
		successHand.AddFreeTiles( {
			TileInstance{ { Suit::Manzu, Face::One }, generateID(), },
			TileInstance{ { Suit::Manzu, Face::Two }, generateID(), },
			TileInstance{ { Suit::Manzu, Face::Three }, generateID(), },
			TileInstance{ { Suit::Manzu, Face::Nine }, generateID(), },
			TileInstance{ { Suit::Manzu, Face::Nine }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::One }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::Two }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::Three }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::Four }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::Six }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::Seven }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::Eight }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::Nine }, generateID(), },
			} );

		bool const success = fnTestForYaku(
			StandardYaku::MenzenchinTsumohou(),
			successHand,
			{ TileInstance{ { Suit::Pinzu, Face::Five }, generateID(), }, TileDrawType::SelfDraw }
		);
		riEnsure( success, "MenzenchinTsumohou failed!" );
	}

	// Riichi
	{
		Hand failHand;
		failHand.AddFreeTiles( {
			TileInstance{ { Suit::Manzu, Face::One }, generateID(), },
			TileInstance{ { Suit::Manzu, Face::Two }, generateID(), },
			TileInstance{ { Suit::Manzu, Face::Three }, generateID(), },
			TileInstance{ { Suit::Manzu, Face::Nine }, generateID(), },
			TileInstance{ { Suit::Manzu, Face::Nine }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::One }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::Two }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::Three }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::Three }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::Five }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::Seven }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::Eight }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::Nine }, generateID(), },
			} );

		bool const fail = fnTestForYaku(
			StandardYaku::Riichi(),
			failHand,
			{ TileInstance{ { Suit::Pinzu, Face::Four }, generateID(), }, TileDrawType::SelfDraw }
		);
		riEnsure( !fail, "Riichi failed!" );

		Round riichiMockRound( mockSeat, mockPlayers, mockRules, mockRNG );
		riichiMockRound.DealHands();
		riichiMockRound.Riichi( std::nullopt );

		Hand successHand;
		successHand.AddFreeTiles( {
			TileInstance{ { Suit::Manzu, Face::One }, generateID(), },
			TileInstance{ { Suit::Manzu, Face::Two }, generateID(), },
			TileInstance{ { Suit::Manzu, Face::Three }, generateID(), },
			TileInstance{ { Suit::Manzu, Face::Nine }, generateID(), },
			TileInstance{ { Suit::Manzu, Face::Nine }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::One }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::Two }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::Three }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::Four }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::Six }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::Seven }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::Eight }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::Nine }, generateID(), },
			} );

		bool const success = fnTestForYaku(
			StandardYaku::Riichi(),
			successHand,
			{ TileInstance{ { Suit::Pinzu, Face::Five }, generateID(), }, TileDrawType::SelfDraw },
			&riichiMockRound
		);
		riEnsure( success, "Riichi failed!" );
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
				TileInstance{ { Suit::Manzu, Face::One }, generateID(), },
				TileInstance{ { Suit::Manzu, Face::Two }, generateID(), },
				TileInstance{ { Suit::Manzu, Face::Three }, generateID(), },
				TileInstance{ { Suit::Manzu, Face::Nine }, generateID(), },
				TileInstance{ { Suit::Manzu, Face::Nine }, generateID(), },
				TileInstance{ { Suit::Pinzu, Face::One }, generateID(), },
				TileInstance{ { Suit::Pinzu, Face::Two }, generateID(), },
				TileInstance{ { Suit::Pinzu, Face::Three }, generateID(), },
				TileInstance{ { Suit::Pinzu, Face::Three }, generateID(), },
				TileInstance{ { Suit::Pinzu, Face::Five }, generateID(), },
				TileInstance{ { Suit::Pinzu, Face::Seven }, generateID(), },
				TileInstance{ { Suit::Pinzu, Face::Eight }, generateID(), },
				TileInstance{ { Suit::Pinzu, Face::Nine }, generateID(), },
				} );

			bool const fail = fnTestForYaku(
				StandardYaku::Ippatsu(),
				failHand,
				{ TileInstance{ { Suit::Pinzu, Face::Four }, generateID(), }, TileDrawType::SelfDraw },
				&riichiMockRound
			);
			riEnsure( !fail, "Ippatsu failed!" );
		}

		{
			Round riichiMockRound( mockSeat, mockPlayers, mockRules, mockRNG );
			riichiMockRound.DealHands();
			riichiMockRound.Riichi( std::nullopt );

			Hand successHand;
			successHand.AddFreeTiles( {
				TileInstance{ { Suit::Manzu, Face::One }, generateID(), },
				TileInstance{ { Suit::Manzu, Face::Two }, generateID(), },
				TileInstance{ { Suit::Manzu, Face::Three }, generateID(), },
				TileInstance{ { Suit::Manzu, Face::Nine }, generateID(), },
				TileInstance{ { Suit::Manzu, Face::Nine }, generateID(), },
				TileInstance{ { Suit::Pinzu, Face::One }, generateID(), },
				TileInstance{ { Suit::Pinzu, Face::Two }, generateID(), },
				TileInstance{ { Suit::Pinzu, Face::Three }, generateID(), },
				TileInstance{ { Suit::Pinzu, Face::Four }, generateID(), },
				TileInstance{ { Suit::Pinzu, Face::Six }, generateID(), },
				TileInstance{ { Suit::Pinzu, Face::Seven }, generateID(), },
				TileInstance{ { Suit::Pinzu, Face::Eight }, generateID(), },
				TileInstance{ { Suit::Pinzu, Face::Nine }, generateID(), },
				} );

			bool const success = fnTestForYaku(
				StandardYaku::Ippatsu(),
				successHand,
				{ TileInstance{ { Suit::Pinzu, Face::Five }, generateID(), }, TileDrawType::SelfDraw },
				&riichiMockRound
			);
			riEnsure( success, "Ippatsu failed!" );
		}
	}

	// Pinfu
	{
		// Identical hands except different wait types

		Hand failHand;
		failHand.AddFreeTiles( {
			TileInstance{ { Suit::Manzu, Face::One }, generateID(), },
			TileInstance{ { Suit::Manzu, Face::Two }, generateID(), },
			TileInstance{ { Suit::Manzu, Face::Three }, generateID(), },
			TileInstance{ { Suit::Manzu, Face::Four }, generateID(), },
			TileInstance{ { Suit::Manzu, Face::Five }, generateID(), },
			TileInstance{ { Suit::Manzu, Face::Six }, generateID(), },
			TileInstance{ { Suit::Manzu, Face::Four }, generateID(), },
			TileInstance{ { Suit::Manzu, Face::Five }, generateID(), },
			TileInstance{ { Suit::Manzu, Face::Six }, generateID(), },
			TileInstance{ { Suit::Manzu, Face::Seven }, generateID(), },
			TileInstance{ { Suit::Manzu, Face::Seven }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::Three }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::Six }, generateID(), },
			} );

		bool const fail = fnTestForYaku(
			StandardYaku::Pinfu(),
			failHand,
			{ TileInstance{ { Suit::Pinzu, Face::Five }, generateID(), }, TileDrawType::SelfDraw }
		);
		riEnsure( !fail, "Pinfu failed!" );

		Hand successHand;
		successHand.AddFreeTiles( {
			TileInstance{ { Suit::Manzu, Face::One }, generateID(), },
			TileInstance{ { Suit::Manzu, Face::Two }, generateID(), },
			TileInstance{ { Suit::Manzu, Face::Three }, generateID(), },
			TileInstance{ { Suit::Manzu, Face::Four }, generateID(), },
			TileInstance{ { Suit::Manzu, Face::Five }, generateID(), },
			TileInstance{ { Suit::Manzu, Face::Six }, generateID(), },
			TileInstance{ { Suit::Manzu, Face::Four }, generateID(), },
			TileInstance{ { Suit::Manzu, Face::Five }, generateID(), },
			TileInstance{ { Suit::Manzu, Face::Six }, generateID(), },
			TileInstance{ { Suit::Manzu, Face::Seven }, generateID(), },
			TileInstance{ { Suit::Manzu, Face::Seven }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::Three }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::Four }, generateID(), },
			} );

		bool const success = fnTestForYaku(
			StandardYaku::Pinfu(),
			successHand,
			{ TileInstance{ { Suit::Pinzu, Face::Five }, generateID(), }, TileDrawType::SelfDraw }
		);
		riEnsure( success, "Pinfu failed!" );
	}

	// Iipeikou
	{
		Hand failHand;
		failHand.AddFreeTiles( {
			TileInstance{ { Suit::Manzu, Face::One }, generateID(), },
			TileInstance{ { Suit::Manzu, Face::Two }, generateID(), },
			TileInstance{ { Suit::Manzu, Face::Three }, generateID(), },
			TileInstance{ { Suit::Manzu, Face::Two }, generateID(), },
			TileInstance{ { Suit::Manzu, Face::Three }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::Three }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::Three }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::Three }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::Four }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::Four }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::Four }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::Six }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::Six }, generateID(), },
			} );

		bool const fail = fnTestForYaku(
			StandardYaku::Iipeikou(),
			failHand,
			{ TileInstance{ { Suit::Manzu, Face::Four }, generateID(), }, TileDrawType::DiscardDraw }
		);
		riEnsure( !fail, "Iipeikou failed!" );

		Hand successHand;
		successHand.AddFreeTiles( {
			TileInstance{ { Suit::Manzu, Face::One }, generateID(), },
			TileInstance{ { Suit::Manzu, Face::Two }, generateID(), },
			TileInstance{ { Suit::Manzu, Face::Three }, generateID(), },
			TileInstance{ { Suit::Manzu, Face::Two }, generateID(), },
			TileInstance{ { Suit::Manzu, Face::Three }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::Three }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::Three }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::Three }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::Four }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::Four }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::Four }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::Six }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::Six }, generateID(), },
			} );

		bool const success = fnTestForYaku(
			StandardYaku::Iipeikou(),
			successHand,
			{ TileInstance{ { Suit::Manzu, Face::One }, generateID(), }, TileDrawType::DiscardDraw }
		);
		riEnsure( success, "Iipeikou failed!" );
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
			TileInstance{ { Suit::Manzu, Face::One }, generateID(), },
			TileInstance{ { Suit::Manzu, Face::Two }, generateID(), },
			TileInstance{ { Suit::Manzu, Face::Three }, generateID(), },
			TileInstance{ { Suit::Manzu, Face::Nine }, generateID(), },
			TileInstance{ { Suit::Manzu, Face::Nine }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::One }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::Two }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::Three }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::Three }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::Five }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::Seven }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::Eight }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::Nine }, generateID(), },
			} );

		bool const fail = fnTestForYaku(
			StandardYaku::Ikkitsuukan(),
			failHand,
			{ TileInstance{ { Suit::Pinzu, Face::Four }, generateID(), }, TileDrawType::SelfDraw }
		);
		riEnsure( !fail, "Ikkitsuukan failed!" );

		Hand successHand;
		successHand.AddFreeTiles( {
			TileInstance{ { Suit::Manzu, Face::One }, generateID(), },
			TileInstance{ { Suit::Manzu, Face::Two }, generateID(), },
			TileInstance{ { Suit::Manzu, Face::Three }, generateID(), },
			TileInstance{ { Suit::Manzu, Face::Nine }, generateID(), },
			TileInstance{ { Suit::Manzu, Face::Nine }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::One }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::Two }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::Three }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::Four }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::Six }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::Seven }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::Eight }, generateID(), },
			TileInstance{ { Suit::Pinzu, Face::Nine }, generateID(), },
			} );

		bool const success = fnTestForYaku(
			StandardYaku::Ikkitsuukan(),
			successHand,
			{ TileInstance{ { Suit::Pinzu, Face::Five }, generateID(), }, TileDrawType::SelfDraw }
		);
		riEnsure( success, "Ikkitsuukan failed!" );
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
