#include "Rules_Standard.hpp"

#include "HandInterpreter_Standard.hpp"
#include "Yaku_Standard.hpp"

namespace Riichi
{

//------------------------------------------------------------------------------
StandardYonma::StandardYonma
(
)
{
	m_interpreters.emplace_back( std::make_unique<StandardInterpreter>() );

	// Yonma uses all tiles
	// TODO-RULES: red dora
	for ( int tileTypeCount = 0; tileTypeCount < 4; ++tileTypeCount )
	{
		for ( Suit suit : Suits{} )
		{
			for ( SuitTileValue val : SuitTileValue::InclusiveRange( SuitTileValue::Min, SuitTileValue::Max ) )
			{
				m_tileSet.emplace_back( SuitTile{ suit, val } );
			}
		}

		for ( DragonTileType dragon : DragonTileTypes{} )
		{
			m_tileSet.emplace_back( dragon );
		}

		for ( WindTileType wind : WindTileTypes{} )
		{
			m_tileSet.emplace_back( wind );
		}
	}

	Ensure( m_tileSet.size() == 4 * (3 * 9 + 3 + 4), "Tileset filled incorrectly" );
}

//------------------------------------------------------------------------------
Set<Tile> StandardYonma::WaitsWithYaku
(
	RoundData const& i_round,
	Seat const& i_playerSeat,
	Hand const& i_hand,
	TileDraw const& i_lastTile
) const
{
	// Let's  a s s e s s
	HandAssessment const assessment( i_hand, *this );

	Set<Tile> waits;
	for ( HandInterpretation const& interp : assessment.Interpretations() )
	{
		if ( interp.m_waitType == WaitType::None )
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
					waits.insert_range( interp.m_waits );
				}
			}
		}
	}

	return waits;
}

//------------------------------------------------------------------------------
Pair<Points, Vector<Pair<char const*, HanValue>>> StandardYonma::CalculateBasicPoints
(
	RoundData const& i_round,
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

	Ensure( maxInterp, "Had max points more than 0 but no valid interpretation" );

	// TODO-MVP: add dora

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
	auto fnIsHonorOrTerminal = []( Tile const& i_tile )
	{
		return i_tile.Type() != TileType::Suit
			|| i_tile.Get<TileType::Suit>().m_value == SuitTileValue::Min
			|| i_tile.Get<TileType::Suit>().m_value == SuitTileValue::Max;
	};

	for ( HandGroup const& group : maxInterp->m_groups )
	{
		if ( group.Type() == GroupType::Triplet )
		{
			fu += ( fnIsHonorOrTerminal( group[ 0 ] ) ? 4 : 2 ) * ( group.Open() ? 2 : 1 );
		}
		else if ( group.Type() == GroupType::Quad )
		{
			fu += ( fnIsHonorOrTerminal( group[ 0 ] ) ? 16 : 8 ) * ( group.Open() ? 2 : 1 );
		}
	}

	// Waits
	switch ( maxInterp->m_waitType )
	{
	using enum WaitType;
	case None:
	{
		Error( "No wait type on winning interp" );
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
		fu += fnIsHonorOrTerminal( i_lastTile.m_tile ) ? 8 : 4;
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
	bool const wonWithPinfu = std::ranges::any_of( maxScore, []( auto const& yaku ) { return std::strcmp( yaku.first, "Pinfu" ) == 0; } );
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

	bool const wonWithChiitoitsu = std::ranges::any_of( maxScore, []( auto const& yaku ) { return std::strcmp( yaku.first, "Chiitoitsu" ) == 0; } );
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
Pair<Points, Points> StandardYonma::PointsFromEachPlayerTsumo
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
Points StandardYonma::PointsFromPlayerRon
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
/*static*/ Points StandardYonma::RoundTo100
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