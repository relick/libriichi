#include "Rules_Standard.hpp"

#include "Round.hpp"
#include "Table.hpp"

#include "HandInterpreter_Standard.hpp"
#include "Yaku_Standard.hpp"

namespace Riichi
{

//------------------------------------------------------------------------------
StandardYonmaCore::StandardYonmaCore
(
)
{
	AddInterpreter<StandardInterpreter>();

	{
		using namespace StandardYaku;
		AddYakuEvaluator<MenzenchinTsumohou>();
		AddYakuEvaluator<Riichi>();
		AddYakuEvaluator<Ippatsu>();
		AddYakuEvaluator<Pinfu>();
		AddYakuEvaluator<Iipeikou>();
		AddYakuEvaluator<HaiteiRaoyue>();
		AddYakuEvaluator<HouteiRaoyui>();
		AddYakuEvaluator<RinshanKaihou>();
		AddYakuEvaluator<Chankan>();
		AddYakuEvaluator<Tanyao<>>();
		AddYakuEvaluator<Yakuhai_Haku>();
		AddYakuEvaluator<Yakuhai_Hatsu>();
		AddYakuEvaluator<Yakuhai_Chun>();
		AddYakuEvaluator<Bakaze>();
		AddYakuEvaluator<Jikaze>();
		AddYakuEvaluator<DoubleRiichi>();
		AddYakuEvaluator<Chantaiyao>();
		AddYakuEvaluator<SanshokuDoujun>();
		AddYakuEvaluator<Ikkitsuukan>();
		AddYakuEvaluator<Toitoi>();
		AddYakuEvaluator<Sanankou>();
		AddYakuEvaluator<SanshokuDoukou>();
		AddYakuEvaluator<Sankantsu>();
		AddYakuEvaluator<Chiitoitsu>();
		AddYakuEvaluator<Honroutou>();
		AddYakuEvaluator<Shousangen>();
		AddYakuEvaluator<Honitsu>();
		AddYakuEvaluator<JunchanTaiyao>();
		AddYakuEvaluator<Ryanpeikou>();
		AddYakuEvaluator<Chinitsu>();
		AddYakuEvaluator<KokushiMusou>();
		AddYakuEvaluator<Suuankou>();
		AddYakuEvaluator<Daisangen>();
		AddYakuEvaluator<Shousuushii>();
		AddYakuEvaluator<Daisuushii>();
		AddYakuEvaluator<Tsuuiisou>();
		AddYakuEvaluator<Chinroutou>();
		AddYakuEvaluator<Ryuuiisou>();
		AddYakuEvaluator<ChuurenPoutou>();
		AddYakuEvaluator<Suukantsu>();
		AddYakuEvaluator<Tenhou>();
		AddYakuEvaluator<Chihou>();
	}

	// Set up properties, namely akadora
	RegisterTileProperties(
		Akadora{}
	);

	// Yonma uses all tiles

	TileInstanceIDGenerator generateTileID;
	for ( int tileTypeCount = 0; tileTypeCount < 4; ++tileTypeCount )
	{
		for ( Suit suit : Suits{} )
		{
			for ( Face num : Numbers{} )
			{
				TileProperties props;
				if ( num == Face::Five && tileTypeCount == 0 )
				{
					props = TileProperties{ Akadora{} };
				}
				m_tileSet.emplace_back( Tile{ suit, num, props }, generateTileID() );
			}
		}

		for ( Face dragon : Dragons{} )
		{
			m_tileSet.emplace_back( dragon, generateTileID() );
		}

		for ( Face wind : Winds{} )
		{
			m_tileSet.emplace_back( wind, generateTileID() );
		}
	}

	riEnsure( m_tileSet.size() == 4 * (3 * 9 + 3 + 4), "Tileset filled incorrectly" );
}

//------------------------------------------------------------------------------
Pair<Set<TileKind>, Vector<TileInstance>> StandardYonmaCore::WaitsWithYaku
(
	Round const& i_round,
	Seat const& i_playerSeat,
	Hand const& i_hand,
	TileDraw const& i_lastTile,
	bool i_considerForRiichi
) const
{
	// Closed kan theft is not allowed for a win
	// TODO-RULES: some rulesets allow closed kan theft for thirteen orphans
	if ( i_lastTile.m_type == TileDrawType::ClosedKanTheft )
	{
		return {};
	}

	bool riichiAddsYaku = false;
	Vector<TileInstance> validDiscardsForRiichi;

	auto fnAddDiscardsForRiichi = [ & ]( HandAssessment const& i_assessment, HandInterpretation const& i_interp, TileInstance const& i_discardedTile )
	{
		if ( !i_considerForRiichi || i_interp.m_waitType == WaitType::None )
		{
			return;
		}
		if ( riichiAddsYaku )
		{
			validDiscardsForRiichi.push_back( i_discardedTile );
			return;
		}

		for ( YakuEvaluator const& yaku : YakuEvaluators() )
		{
			if ( yaku.UsesInterpreter( i_interp.m_interpreter ) )
			{
				if ( yaku.CalculateValue(
					i_round,
					i_playerSeat,
					i_hand,
					i_assessment,
					i_interp,
					*i_interp.m_waits.begin(),
					TileDrawType::DiscardDraw
				).IsValid() )
				{
					validDiscardsForRiichi.push_back( i_discardedTile );
				}
			}
		}
	};

	if ( i_considerForRiichi )
	{
		for ( YakuEvaluator const& yaku : YakuEvaluators() )
		{
			if ( yaku.AddsYakuToRiichi() )
			{
				riichiAddsYaku = true;
				break;
			}
		}

		// TODO-DEBT TODO-OPT: I've decided the easiest code to write here is to assess all discard options to see if there are any waits
		// but this is probably the worst way to do it

		for ( TileInstance const& tile : i_hand.FreeTiles() )
		{
			Hand alteredHand = i_hand;
			alteredHand.Discard( tile, i_lastTile );
			HandAssessment const assessment( alteredHand, *this );

			for ( HandInterpretation const& interp : assessment.Interpretations() )
			{
				fnAddDiscardsForRiichi( assessment, interp, tile );
			}
		}

	}

	// Let's  a s s e s s
	HandAssessment const assessment( i_hand, *this );

	Set<TileKind> waits;
	for ( HandInterpretation const& interp : assessment.Interpretations() )
	{
		fnAddDiscardsForRiichi( assessment, interp, i_lastTile.m_tile );

		if ( interp.m_waitType == WaitType::None || !interp.m_waits.contains( i_lastTile.m_tile.Tile() ) )
		{
			continue;
		}

		for ( YakuEvaluator const& yaku : YakuEvaluators() )
		{
			if ( yaku.UsesInterpreter( interp.m_interpreter ) )
			{
				if ( yaku.CalculateValue(
					i_round,
					i_playerSeat,
					i_hand,
					assessment,
					interp,
					i_lastTile.m_tile.Tile(),
					i_lastTile.m_type
				).IsValid() )
				{
					std::ranges::for_each( interp.m_waits, [ & ]( TileKind kind ) { waits.insert( kind ); } );
				}
			}
		}
	}

	return { std::move( waits ), std::move( validDiscardsForRiichi ) };
}

//------------------------------------------------------------------------------
HandScore StandardYonmaCore::CalculateBasicPoints
(
	Round const& i_round,
	Seat const& i_playerSeat,
	Hand const& i_hand,
	TileDraw const& i_lastTile
) const
{
	// Let's  a s s e s s
	HandAssessment const assessment( i_hand, *this );

	Han max = 0;
	Vector<Pair<char const*, HanValue>> maxScore;
	HandInterpretation const* maxInterp{ nullptr };
	for ( HandInterpretation const& interp : assessment.Interpretations() )
	{
		if ( !interp.m_waits.contains( i_lastTile.m_tile.Tile() ) )
		{
			continue;
		}

		Han total = 0;
		Vector<Pair<char const*, HanValue>> interpScore;
		for ( YakuEvaluator const& yaku : YakuEvaluators() )
		{
			if ( yaku.UsesInterpreter( interp.m_interpreter ) )
			{
				HanValue const value = yaku.CalculateValue(
					i_round,
					i_playerSeat,
					i_hand,
					assessment,
					interp,
					i_lastTile.m_tile.Tile(),
					i_lastTile.m_type
				);

				if ( value.IsValid() )
				{
					interpScore.push_back( { yaku.Name(), value } );
					total += value.Get();
				}
			}
		}

		// TODO-RULES: Specify other possibilities for superseding
		if ( max <= total )
		{
			max = total;
			maxScore = std::move( interpScore );
			maxInterp = &interp;
		}
	}

	if ( max == 0 )
	{
		return { 0, 0, {} };
	}

	riEnsure( maxInterp, "Had max points more than 0 but no valid interpretation" );

	auto fnHandHasYaku = [ & ]( char const* i_yakuName )
	{
		return std::ranges::any_of( maxScore, [ & ]( auto const& yaku ) { return std::strcmp( yaku.first, i_yakuName ) == 0; } );
	};

	// Handle Dora
	{
		bool constexpr c_indicatedValue = true;
		bool const includeUradora = fnHandHasYaku( "Riichi" ) || fnHandHasYaku( "DoubleRiichi" );
		Vector<TileKind> const doraTiles = i_round.GetDoraTiles( includeUradora );

		Han doraValue{ 0 };
		Han uradoraValue{ 0 };
		Han akadoraValue{ 0 };

		size_t const doraBegin = 0;
		size_t const doraEnd = doraTiles.size() / ( includeUradora ? 2 : 1 );
		size_t const uradoraBegin = doraEnd;
		size_t const uradoraEnd = doraTiles.size();

		std::ranges::for_each(
			MaybeAppendTileInstance( i_hand.AllTiles(), i_lastTile.m_tile ),
			[ & ]( TileInstance const& i_tile )
			{
				EqualsTileKind const sharesTileKind{ i_tile };
				for ( size_t i = doraBegin; i < doraEnd; ++i )
				{
					if ( sharesTileKind( doraTiles[ i ] ) )
					{
						++doraValue;
					}
				}
				for ( size_t i = uradoraBegin; i < uradoraEnd; ++i )
				{
					if ( sharesTileKind( doraTiles[ i ] ) )
					{
						++uradoraValue;
					}
				}
				if ( i_tile.Tile().HasProperty<Akadora>() )
				{
					++akadoraValue;
				}
			}
		);

		if ( doraValue > 0 )
		{
			maxScore.push_back( { "Dora", doraValue } );
			max += doraValue;
		}
		if ( uradoraValue > 0 )
		{
			maxScore.push_back( { "Uradora", uradoraValue } );
			max += uradoraValue;
		}
		if ( akadoraValue > 0 )
		{
			maxScore.push_back( { "Akadora", akadoraValue } );
			max += akadoraValue;
		}
	}

	// Calculate fu
	Points fu = 20;

	// Koutsu/kantsu
	for ( HandGroup const& group : maxInterp->m_groups )
	{
		if ( ConsiderLikeTriplet( group.Type() ) )
		{
			fu += 2 // all triplet/quad groups get at least 2 fu
				* ( group.Open() ? 1 : 2 ) // doubled if closed
				* ( group.Type() == GroupType::Triplet ? 1 : 4 ) // quadrupled if a quad
				* ( group[ 0 ].IsHonourOrTerminal() ? 2 : 1 ) // doubled if honour or terminal
			;
			// i.e.
			//                 simples  terminals/honours
			// open triplet          2                  4
			// closed triplet        4                  8
			// open quad             8                 16
			// closed quad          16                 32
		}
	}

	// Waits
	switch ( maxInterp->m_waitType )
	{
	using enum WaitType;
	case None:
	{
		riError( "No wait type on winning interp" );
		break;
	}

	case Tanki:
	case Kanchan:
	case Penchan:
	{
		fu += 2;
		break;
	}
	case Ryanmen:
	{
		fu += 0;
		break;
	}
	case Shanpon:
	{
		fu += i_lastTile.m_tile.Tile().IsHonourOrTerminal() ? 8 : 4;
		break;
	}
	}

	// Yakuhai
	if ( maxInterp->m_waitType == WaitType::Tanki && i_lastTile.m_tile.Tile().IsWind() )
	{
		Face const wind = i_lastTile.m_tile.Tile().Face();
		if ( i_playerSeat == wind || i_round.Wind() == wind )
		{
			// TODO-RULES: double wind may score 4 instead of 2
			fu += 2;
		}
	}
	else
	{
		for ( HandGroup const& group : maxInterp->m_groups )
		{
			if ( group.Type() == GroupType::Pair && group[ 0 ].IsWind() )
			{
				Face const wind = group[ 0 ].Face();
				if ( i_playerSeat == wind || i_round.Wind() == wind )
				{
					// TODO-RULES: double wind may score 4 instead of 2
					fu += 2;
				}
				break;
			}
		}
	}

	// Win condition
	bool const wonWithPinfu = fnHandHasYaku( "Pinfu" );
	switch ( i_lastTile.m_type )
	{
	using enum TileDrawType;
	case SelfDraw:
	case DeadWallDraw:
	{
		// Tsumo
		// TODO-RULES: dead wall draw may not get fu
		if ( !wonWithPinfu )
		{
			fu += 2;
		}
		break;
	}
	case DiscardDraw:
	case ClosedKanTheft:
	case UpgradedKanTheft:
	{
		// Ron
		if ( !assessment.m_open )
		{
			fu += 10;
		}
		break;
	}
	}

	bool const wonWithChiitoitsu = fnHandHasYaku( "Chiitoitsu" );
	if ( wonWithChiitoitsu )
	{
		fu = 25;
	}
	else
	{
		// Round up to nearest 10
		Points const rem = fu % 10;
		if ( rem > 0 )
		{
			fu = fu - rem + 10;
		}
	}

	// "open pinfu" set to 30
	if ( assessment.m_open && fu == 20 )
	{
		fu = 30;
	}

	Points const basePoints = [ & ]()
	{
		if ( max >= 5 )
		{
			// Counted mangan
			// TODO-RULES: double/triple yakuman
			static Points constexpr c_hanTable[] = {
				2000, // 5 han mangan
				3000, // 6 han haneman
				3000, // 7 han haneman
				4000, // 8 han baiman
				4000, // 9 han baiman
				4000, // 10 han baiman
				6000, // 11 han sanbaiman
				6000, // 12 han sanbaiman
				8000, // 13 han yakuman
			};

			return c_hanTable[ std::min<Han>( max, 13 ) - 5 ];
		}

		Points base = fu * 4;
		Han han = max;
		while ( han-- )
		{
			base *= 2;
		}

		// TODO-RULES: kirage mangan (round up to 2000 for 1920 points)
		return base >= 2000 ? 2000 : base;
	}();

	return { basePoints, fu, std::move( maxScore ) };
}

//------------------------------------------------------------------------------
bool StandardYonmaCore::NoMoreRoundsCore
(
	Table const& i_table,
	Round const& i_previousRound,
	Seat i_gameLength
)	const
{
	// TODO-RULES: allow for negative points play
	for ( Pair<Player, Points> const& player : i_table.AllPlayers() )
	{
		if ( player.second < 0 )
		{
			return true;
		}
	}

	// TODO-RULES: extension rounds if not enough points earned

	bool const roundWindWillIncrement = !RepeatRound( i_previousRound ) && i_previousRound.NextPlayerIsInitial();
	if ( roundWindWillIncrement && i_previousRound.Wind() == i_gameLength )
	{
		return true;
	}

	return false;
}

//------------------------------------------------------------------------------
bool StandardYonmaCore::RepeatRound
(
	Round const& i_previousRound
)	const
{
	// We repeat if dealer won, or there was a draw and dealer was in tenpai whilst others in tenpai
	return i_previousRound.IsWinner( Seat::East ) || i_previousRound.FinishedInTenpai( Seat::East );
}

//------------------------------------------------------------------------------
bool StandardYonmaCore::ShouldAddHonba
(
	Round const& i_previousRound
)	const
{
	// Dealer won, or there was a draw
	return i_previousRound.IsWinner( Seat::East ) || !i_previousRound.AnyWinners();
}

//------------------------------------------------------------------------------
TablePayments StandardYonmaCore::HonbaPotPayments
(
	size_t i_honbaSticks,
	SeatSet const& i_winners,
	Option<Seat> i_ronLoser
)	const
{
	if ( i_winners.Size() == 0 )
	{
		return {};
	}

	bool const tsumoWin = !i_ronLoser.has_value();

	riEnsure( !tsumoWin || i_winners.Size() == 1, "Max 1 tsumo allowed" );

	// Honba paid evenly by all non-winners for tsumo, but just the loser when ron
	Points const singleHonbaPayment = static_cast< Points >( i_honbaSticks * 100 );
	Points const winnerPayment = singleHonbaPayment * 3;

	TablePayments payments;

	if ( i_ronLoser.has_value() )
	{
		// Single loser pays the honba in full to each winner
		Points const loserPayment = -( winnerPayment * i_winners.Size() );

		for ( Seat winner : i_winners )
		{
			payments.m_pointsPerSeat[ winner ] = winnerPayment;
		}
		payments.m_pointsPerSeat[ i_ronLoser.value() ] = loserPayment;
	}
	else
	{
		riEnsure( i_winners.Size() == 1, "Max 1 tsumo allowed" );

		// Non-winners pay evenly to the tsumo winner
		Points const loserPayment = -singleHonbaPayment;

		for ( Seat loser : ~i_winners )
		{
			payments.m_pointsPerSeat[ loser ] = loserPayment;
		}
		payments.m_pointsPerSeat[ *i_winners.begin() ] = winnerPayment;
	}

	return payments;
}

//------------------------------------------------------------------------------
TablePayments StandardYonmaCore::RiichiBetPotPayments
(
	size_t i_riichiSticks,
	SeatSet const& i_winners,
	Option<Seat> i_ronLoser
)	const
{
	if ( i_winners.Size() == 0 )
	{
		return {};
	}

	// The riichi bet is already paid, so this is adding points to the winners only, by dividing what's in the pot evenly
	Points const riichiBetPayment = RoundTo100( static_cast< Points >( ( i_riichiSticks * RiichiBetPoints() ) / i_winners.Size() ) );

	TablePayments payments;

	for ( Seat winner : i_winners )
	{
		payments.m_pointsPerSeat[ winner ] = riichiBetPayment;
	}

	return payments;
}

//------------------------------------------------------------------------------
TablePayments StandardYonmaCore::TsumoPayments
(
	HandScore const& i_handScore,
	Seat i_winner
)	const
{
	TablePayments payments;
	bool const winnerIsDealer = ( i_winner == Seat::East );

	Points const doubleBasic = RoundTo100( 2 * i_handScore.m_basicPoints );
	Points const singleBasic = RoundTo100( i_handScore.m_basicPoints );

	for ( Seat seat : Seats{} )
	{
		if ( seat == i_winner )
		{
			payments.m_pointsPerSeat[ seat ] = doubleBasic + ( 2 * ( winnerIsDealer ? doubleBasic : singleBasic ) );
		}
		else
		{
			payments.m_pointsPerSeat[ seat ] = ( seat == Seat::East || winnerIsDealer ) ? -doubleBasic : -singleBasic;
		}
	}

	return payments;
}

//------------------------------------------------------------------------------
TablePayments StandardYonmaCore::RonPayments
(
	HandScore const& i_handScore,
	Seat i_winner,
	Seat i_loser
)	const
{
	TablePayments payments;
	bool const winnerIsDealer = ( i_winner == Seat::East );

	payments.m_pointsPerSeat[ i_winner ] = RoundTo100( ( winnerIsDealer ? 6 : 4 ) * i_handScore.m_basicPoints );
	payments.m_pointsPerSeat[ i_loser ] = -payments.m_pointsPerSeat[ i_winner ];

	return payments;
}

//------------------------------------------------------------------------------
TablePayments StandardYonmaCore::ExhaustiveDrawPayments
(
	SeatSet const& i_playersInTenpai
)	const
{
	if ( i_playersInTenpai.Size() == 0 || i_playersInTenpai.Size() == 4 )
	{
		return {};
	}

	size_t constexpr c_pointsAvailable = 3000;
	Points const pointsPaidPerNonTenpaiPlayer = static_cast< Points >( c_pointsAvailable / ( GetPlayerCount() - i_playersInTenpai.Size() ) );
	Points const pointsGainedPerTenpaiPlayer = static_cast< Points >( c_pointsAvailable / i_playersInTenpai.Size() );

	TablePayments payments;
	for ( Seat tenpaiPlayer : i_playersInTenpai )
	{
		payments.m_pointsPerSeat[ tenpaiPlayer ] = pointsGainedPerTenpaiPlayer;
	}
	for ( Seat nonTenpaiPlayer : ~i_playersInTenpai )
	{
		payments.m_pointsPerSeat[ nonTenpaiPlayer ] = -pointsPaidPerNonTenpaiPlayer;
	}

	return payments;
}

//------------------------------------------------------------------------------
/*static*/ Points StandardYonmaCore::RoundTo100
(
	Points i_finalPoints
)
{
	Points const rem = i_finalPoints % 100;
	if ( rem > 0 )
	{
		return i_finalPoints - rem + 100;
	}
	return i_finalPoints;
}

}