#include "Rules_Standard.hpp"

#include "Round.hpp"
#include "Table.hpp"

#include "HandInterpreter_Standard.hpp"
#include "Yaku_Standard.hpp"

#include "range/v3/action.hpp"
#include "range/v3/algorithm.hpp"

namespace Riichi
{

//------------------------------------------------------------------------------
StandardYonmaCore::StandardYonmaCore
(
)
{
	m_interpreters.emplace_back( std::make_unique<StandardInterpreter>() );

	{
		using namespace StandardYaku;
		m_yaku.emplace_back( std::make_unique<MenzenchinTsumohou>() );
		m_yaku.emplace_back( std::make_unique<Riichi>() );
		m_yaku.emplace_back( std::make_unique<Ippatsu>() );
		m_yaku.emplace_back( std::make_unique<Pinfu>() );
		m_yaku.emplace_back( std::make_unique<Iipeikou>() );
		m_yaku.emplace_back( std::make_unique<HaiteiRaoyue>() );
		m_yaku.emplace_back( std::make_unique<HouteiRaoyui>() );
		m_yaku.emplace_back( std::make_unique<RinshanKaihou>() );
		m_yaku.emplace_back( std::make_unique<Chankan>() );
		m_yaku.emplace_back( std::make_unique<Tanyao<>>() );
		m_yaku.emplace_back( std::make_unique<Yakuhai_Haku>() );
		m_yaku.emplace_back( std::make_unique<Yakuhai_Hatsu>() );
		m_yaku.emplace_back( std::make_unique<Yakuhai_Chun>() );
		m_yaku.emplace_back( std::make_unique<Bakaze>() );
		m_yaku.emplace_back( std::make_unique<Jikaze>() );
		m_yaku.emplace_back( std::make_unique<DoubleRiichi>() );
		m_yaku.emplace_back( std::make_unique<Chantaiyao>() );
		m_yaku.emplace_back( std::make_unique<SanshokuDoujun>() );
		m_yaku.emplace_back( std::make_unique<Ikkitsuukan>() );
		m_yaku.emplace_back( std::make_unique<Toitoi>() );
		m_yaku.emplace_back( std::make_unique<Sanankou>() );
		m_yaku.emplace_back( std::make_unique<SanshokuDoukou>() );
		m_yaku.emplace_back( std::make_unique<Sankantsu>() );
		m_yaku.emplace_back( std::make_unique<Chiitoitsu>() );
		m_yaku.emplace_back( std::make_unique<Honroutou>() );
		m_yaku.emplace_back( std::make_unique<Shousangen>() );
		m_yaku.emplace_back( std::make_unique<Honitsu>() );
		m_yaku.emplace_back( std::make_unique<JunchanTaiyao>() );
		m_yaku.emplace_back( std::make_unique<Ryanpeikou>() );
		m_yaku.emplace_back( std::make_unique<Chinitsu>() );
		m_yaku.emplace_back( std::make_unique<KokushiMusou>() );
		m_yaku.emplace_back( std::make_unique<Suuankou>() );
		m_yaku.emplace_back( std::make_unique<Daisangen>() );
		m_yaku.emplace_back( std::make_unique<Shousuushii>() );
		m_yaku.emplace_back( std::make_unique<Daisuushii>() );
		m_yaku.emplace_back( std::make_unique<Tsuuiisou>() );
		m_yaku.emplace_back( std::make_unique<Chinroutou>() );
		m_yaku.emplace_back( std::make_unique<Ryuuiisou>() );
		m_yaku.emplace_back( std::make_unique<ChuurenPoutou>() );
		m_yaku.emplace_back( std::make_unique<Suukantsu>() );
		m_yaku.emplace_back( std::make_unique<Tenhou>() );
		m_yaku.emplace_back( std::make_unique<Chihou>() );
	}

	// Yonma uses all tiles
	// TODO-RULES: red dora
	uint32_t tileID = 0;
	for ( int tileTypeCount = 0; tileTypeCount < 4; ++tileTypeCount )
	{
		for ( Suit suit : Suits{} )
		{
			for ( Number num : Numbers{} )
			{
				m_tileSet.emplace_back( SuitTile{ suit, num }, tileID++ );
			}
		}

		for ( DragonTileType dragon : DragonTileTypes{} )
		{
			m_tileSet.emplace_back( dragon, tileID++ );
		}

		for ( WindTileType wind : WindTileTypes{} )
		{
			m_tileSet.emplace_back( wind, tileID++ );
		}
	}

	riEnsure( m_tileSet.size() == 4 * (3 * 9 + 3 + 4), "Tileset filled incorrectly" );
}

//------------------------------------------------------------------------------
Pair<Set<Tile>, Vector<Tile>> StandardYonmaCore::WaitsWithYaku
(
	Round const& i_round,
	Seat const& i_playerSeat,
	Hand const& i_hand,
	TileDraw const& i_lastTile,
	bool i_considerForRiichi
) const
{
	bool riichiAddsYaku = false;
	Vector<Tile> validDiscardsForRiichi;

	auto fnAddDiscardsForRiichi = [ & ]( HandAssessment const& i_assessment, HandInterpretation const& i_interp, Tile const& i_discardedTile )
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

		for ( auto const& yaku : m_yaku )
		{
			if ( yaku->UsesInterpreter( i_interp.m_interpreter ) )
			{
				if ( yaku->CalculateValue(
					i_round,
					i_playerSeat,
					i_hand,
					i_assessment,
					i_interp,
					{ *i_interp.m_waits.begin(), TileDrawType::DiscardDraw }
				).IsValid() )
				{
					validDiscardsForRiichi.push_back( i_discardedTile );
				}
			}
		}
	};

	if ( i_considerForRiichi )
	{
		for ( auto const& yaku : m_yaku )
		{
			if ( yaku->AddsYakuToRiichi() )
			{
				riichiAddsYaku = true;
				break;
			}
		}

		// TODO-DEBT TODO-OPT: I've decided the easiest code to write here is to assess all discard options to see if there are any waits
		// but this is probably the worst way to do it

		for ( size_t tileI = 0; tileI < i_hand.FreeTiles().size(); ++tileI )
		{
			Hand alteredHand = i_hand;
			alteredHand.Discard( i_hand.FreeTiles()[ tileI ], i_lastTile );
			HandAssessment const assessment( alteredHand, *this );

			for ( HandInterpretation const& interp : assessment.Interpretations() )
			{
				fnAddDiscardsForRiichi( assessment, interp, i_hand.FreeTiles()[ tileI ] );
			}
		}

	}

	// Let's  a s s e s s
	HandAssessment const assessment( i_hand, *this );

	Set<Tile> waits;
	for ( HandInterpretation const& interp : assessment.Interpretations() )
	{
		fnAddDiscardsForRiichi( assessment, interp, i_lastTile.m_tile );

		if ( interp.m_waitType == WaitType::None || !interp.m_waits.contains( i_lastTile.m_tile ) )
		{
			continue;
		}

		for ( auto const& yaku : m_yaku )
		{
			if ( yaku->UsesInterpreter( interp.m_interpreter ) )
			{
				if ( yaku->CalculateValue(
					i_round,
					i_playerSeat,
					i_hand,
					assessment,
					interp,
					i_lastTile
				).IsValid() )
				{
					ranges::actions::insert( waits, interp.m_waits );
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
		if ( !interp.m_waits.contains( i_lastTile.m_tile ) )
		{
			continue;
		}

		Han total = 0;
		Vector<Pair<char const*, HanValue>> interpScore;
		for ( auto const& yaku : m_yaku )
		{
			if ( yaku->UsesInterpreter( interp.m_interpreter ) )
			{
				HanValue const value = yaku->CalculateValue(
					i_round,
					i_playerSeat,
					i_hand,
					assessment,
					interp,
					i_lastTile
				);

				if ( value.IsValid() )
				{
					interpScore.push_back( { yaku->Name(), value } );
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
		return { 0, {} };
	}

	riEnsure( maxInterp, "Had max points more than 0 but no valid interpretation" );

	auto fnHandHasYaku = [ & ]( char const* i_yakuName )
	{
		return ranges::any_of( maxScore, [ & ]( auto const& yaku ) { return std::strcmp( yaku.first, i_yakuName ) == 0; } );
	};

	// Handle Dora
	{
		bool constexpr c_indicatedValue = true;
		bool const includeUradora = fnHandHasYaku( "Riichi" ) || fnHandHasYaku( "DoubleRiichi" );
		Vector<Tile> const doraTiles = i_round.GatherDoraTiles( c_indicatedValue );
		Vector<Tile> const uraDoraTiles = includeUradora ? i_round.GatherUradoraTiles( c_indicatedValue ) : Vector<Tile>{};

		Han doraValue{ 0 };
		Han uradoraValue{ 0 };

		auto fnAssessTile = [ & ]( Tile const& i_tile )
		{
			for ( Tile const& doraTile : doraTiles )
			{
				if ( doraTile == i_tile )
				{
					++doraValue;
				}
			}
			for ( Tile const& uradoraTile : uraDoraTiles )
			{
				if ( uradoraTile == i_tile )
				{
					++uradoraValue;
				}
			}
		};

		i_hand.VisitTiles( fnAssessTile	);
		fnAssessTile( i_lastTile.m_tile );

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
	}

	if ( max >= 5 )
	{
		// Counted mangan
		// TODO-RULES: double/triple yakuman
		static Points constexpr c_hanTable[] = {
			0,
			0,
			0,
			0,
			0,
			2000,
			3000,
			4000,
			6000,
			8000,
		};

		return { c_hanTable[ std::min<Han>( max, 13 ) ], std::move( maxScore ) };
	}

	// Calculate fu
	Points fu = 20;

	// Koutsu/kantsu
	for ( HandGroup const& group : maxInterp->m_groups )
	{
		if ( TripletCompatible( group.Type() ) )
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
		fu += i_lastTile.m_tile.IsHonourOrTerminal() ? 8 : 4;
		break;
	}
	}

	// Yakuhai
	if ( maxInterp->m_waitType == WaitType::Tanki && i_lastTile.m_tile.Type() == TileType::Wind )
	{
		WindTileType const wind = i_lastTile.m_tile.Get<TileType::Wind>();
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
			if ( group.Type() == GroupType::Pair && group[ 0 ].Type() == TileType::Wind )
			{
				WindTileType const wind = group[ 0 ].Get<TileType::Wind>();
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
	case KanTheft:
	{
		// Tsumo
		// TODO-RULES: kan theft may not get fu
		if ( assessment.m_open || !wonWithPinfu )
		{
			fu += 2;
		}
		break;
	}
	case DiscardDraw:
	{
		// Ron
		if ( !assessment.m_open || fu == 20 )
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

	Points const basePoints = [ & ]()
	{
		Points base = fu * 4;
		Han han = max;
		while ( han-- )
		{
			base *= 2;
		}

		// TODO-RULES: kirage mangan (round up to 2000 for 1920 points)
		return base >= 2000 ? 2000 : base;
	}();

	return { basePoints, std::move( maxScore ) };
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

	bool anyPlayersOutOfPoints = false;
	i_table.VisitPlayers( [ &anyPlayersOutOfPoints ]( Pair<Player, Points> const& i_player ) { anyPlayersOutOfPoints |= i_player.second < 0; } );

	if ( anyPlayersOutOfPoints )
	{
		return true;
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
Pair<Points, Points> StandardYonmaCore::PotPoints
(
	size_t i_honbaSticks,
	size_t i_riichiSticks,
	bool i_isTsumo,
	size_t i_winners
)	const
{
	if ( i_isTsumo )
	{
		// Honba paid by all non-winners
		riEnsure( i_winners == 1, "Only 1 tsumo" );
		return { static_cast< Points >( i_honbaSticks * 100 ), static_cast< Points >( i_riichiSticks * RiichiBet() ) };
	}

	// Honba paid by ron'd player
	return { static_cast< Points >( i_honbaSticks * 300 ), RoundTo100( static_cast< Points >( ( i_riichiSticks * RiichiBet() ) / i_winners ) ) };
}

//------------------------------------------------------------------------------
Pair<Points, Points> StandardYonmaCore::PointsFromEachPlayerTsumo
(
	Points i_basicPoints,
	bool i_isDealer
)	const
{
	if ( i_isDealer )
	{
		return { RoundTo100( 2 * i_basicPoints ), RoundTo100( 2 * i_basicPoints ) };
	}
	
	return { RoundTo100( 2 * i_basicPoints ), RoundTo100( i_basicPoints ) };
}

//------------------------------------------------------------------------------
Points StandardYonmaCore::PointsFromPlayerRon
(
	Points i_basicPoints,
	bool i_isDealer
)	const
{
	if ( i_isDealer )
	{
		return RoundTo100( 6 * i_basicPoints );
	}

	return RoundTo100( 4 * i_basicPoints );
}

//------------------------------------------------------------------------------
Pair<Points, Points> StandardYonmaCore::PointsEachPlayerInTenpaiDraw
(
	size_t i_playersInTenpai
)	const
{
	if ( i_playersInTenpai == 0 )
	{
		return { 0, 0 };
	}

	size_t const playerCount = GetPlayerCount();
	size_t const playersNotInTenpai = playerCount - i_playersInTenpai;
	size_t const pointsAvailable = playersNotInTenpai * 1000;
	Points const pointsGainedPerTenpaiPlayer = static_cast< Points >( pointsAvailable / i_playersInTenpai );
	Points const pointsPaidPerNonTenpaiPlayer = static_cast< Points >( pointsAvailable / playersNotInTenpai );

	return { pointsGainedPerTenpaiPlayer, pointsPaidPerNonTenpaiPlayer };
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