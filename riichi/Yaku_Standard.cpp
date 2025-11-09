#include "Yaku_Standard.hpp"

#include "Containers.hpp"
#include "Hand.hpp"
#include "Round.hpp"
#include "Tile.hpp"
#include "Utils.hpp"

#include <numeric>
#include "range/v3/algorithm.hpp"
#include "range/v3/view.hpp"

namespace Riichi::StandardYaku
{

// TODO-DEBT: Lots of the yaku need to assess the 'final group' separately from the rest of the groups. This is to avoid creating a container and doing a bunch of copies where unnecessary
// It would be nice to clean this up somehow. It would also be nice if all the yaku could be made to avoid allocating entirely when assessing!

//------------------------------------------------------------------------------
HanValue MenzenchinTsumohou::CalculateValue
(
	YAKU_CALCULATEVALUE_PARAMS()
)	const
{
	if ( !i_assessment.m_open && i_lastTile.m_type != TileDrawType::DiscardDraw )
	{
		return 1;
	}
	return NoYaku;
}

//------------------------------------------------------------------------------
HanValue Riichi::CalculateValue
(
	YAKU_CALCULATEVALUE_PARAMS()
)	const
{
	if ( i_round.CalledRiichi( i_playerSeat ) )
	{
		return 1;
	}
	return NoYaku;
}

//------------------------------------------------------------------------------
HanValue Ippatsu::CalculateValue
(
	YAKU_CALCULATEVALUE_PARAMS()
)	const
{
	// Oh yeah, we're in 'hardcode the yaku' town
	// it's just easier this way, for now
	if ( i_round.RiichiIppatsuValid( i_playerSeat ) )
	{
		return 1;
	}
	return NoYaku;
}

//------------------------------------------------------------------------------
HanValue Pinfu::CalculateValue
(
	YAKU_CALCULATEVALUE_PARAMS()
)	const
{
	if ( i_assessment.m_open || i_assessment.m_containsTileType[ TileType::Dragon ] )
	{
		// Invalid hand for pinfu
		return NoYaku;
	}

	if ( i_interp.m_waitType != WaitType::Ryanmen )
	{
		// Didn't win on open wait
		return NoYaku;
	}

	for ( HandGroup const& group : i_interp.m_groups )
	{
		switch ( group.Type() )
		{
			using enum GroupType;
		case Triplet:
		case Quad:
		case UpgradedQuad:
		{
			return NoYaku;
		}
		case Pair:
		{
			if ( group.TilesType() == TileType::Wind )
			{
				WindTileType const wind = group[ 0 ].Get<TileType::Wind>();
				if ( wind == i_playerSeat || wind == i_round.Wind() )
				{
					// No Fu in my pinfu :(
					return NoYaku;
				}
			}
			break;
		}
		case Sequence:
		{
			// We love sequences in this house
			break;
		}
		}
	}

	// Passed all checks
	return 1;
}

//------------------------------------------------------------------------------
HanValue Iipeikou::CalculateValue
(
	YAKU_CALCULATEVALUE_PARAMS()
)	const
{
	if ( i_assessment.m_open )
	{
		return NoYaku;
	}

	// TODO-OPT: I wonder if there's a more efficient way of assessing this than just comparing all groups

	if ( SequenceWait( i_interp.m_waitType ) )
	{
		// Need to consider the final group, as it completed a sequence
		HandGroup const finalGroup( i_interp, i_lastTile.m_tile );

		for ( HandGroup const& group : i_interp.m_groups )
		{
			if ( IsMatchingSequence( finalGroup, group ) )
			{
				return 1;
			}
		}
	}

	for ( size_t groupI = 0; groupI < i_interp.m_groups.size(); ++groupI )
	{
		for ( size_t secondGroupI = groupI + 1; secondGroupI < i_interp.m_groups.size(); ++secondGroupI )
		{
			if ( IsMatchingSequence( i_interp.m_groups[ groupI ], i_interp.m_groups[ secondGroupI ] ) )
			{
				return 1;
			}
		}
	}

	// Failed to find matches
	return NoYaku;
}

/*static*/ bool Iipeikou::IsMatchingSequence( HandGroup const& i_a, HandGroup const& i_b )
{
	if ( i_a.Type() == GroupType::Sequence && i_b.Type() == GroupType::Sequence )
	{
		return i_a.Tiles() == i_b.Tiles();
	}
	return false;
}

//------------------------------------------------------------------------------
HanValue HaiteiRaoyue::CalculateValue
(
	YAKU_CALCULATEVALUE_PARAMS()
)	const
{
	if ( i_lastTile.m_type == TileDrawType::SelfDraw && i_round.WallTilesRemaining() == 0u )
	{
		return 1;
	}

	return NoYaku;
}

//------------------------------------------------------------------------------
HanValue HouteiRaoyui::CalculateValue
(
	YAKU_CALCULATEVALUE_PARAMS()
)	const
{
	if ( i_lastTile.m_type == TileDrawType::DiscardDraw && i_round.WallTilesRemaining() == 0u )
	{
		return 1;
	}

	return NoYaku;
}

//------------------------------------------------------------------------------
HanValue RinshanKaihou::CalculateValue
(
	YAKU_CALCULATEVALUE_PARAMS()
)	const
{
	if ( i_lastTile.m_type == TileDrawType::DeadWallDraw )
	{
		return 1;
	}
	return NoYaku;
}

//------------------------------------------------------------------------------
HanValue Chankan::CalculateValue
(
	YAKU_CALCULATEVALUE_PARAMS()
)	const
{
	// TODO-RULES: In almost all cases, players are not allowed to call ron on an ankan (closed kan). The notable exception involves a kokushi tenpai hand, where the last tile needed for the yakuman is called for an ankan. However, the kokushi exception varies on the rules. In some rules, it is disallowed outright. 
	// TODO-RULES: If a player is tenpai for suukantsu and a fifth kan is invoked, chankan may not be applied if that fifth kan is an added kan. 
	if ( i_lastTile.m_type == TileDrawType::KanTheft )
	{
		return 1;
	}
	return NoYaku;
}

//------------------------------------------------------------------------------
HanValue Bakaze::CalculateValue
(
	YAKU_CALCULATEVALUE_PARAMS()
)	const
{
	for ( HandGroup const& group : i_interp.m_groups )
	{
		if ( TripletCompatible( group.Type() ) )
		{
			if ( ValidTile( group[ 0 ], i_round.Wind() ) )
			{
				return 1;
			}
		}
	}

	if ( i_interp.m_waitType == WaitType::Shanpon && ValidTile( i_lastTile.m_tile, i_round.Wind() ) )
	{
		return 1;
	}

	return NoYaku;
}

/*static*/ bool Bakaze::ValidTile( Tile const& i_tile, Seat const& i_roundWind )
{
	return i_tile.Type() == TileType::Wind && i_tile.Get<TileType::Wind>() == i_roundWind;
}

//------------------------------------------------------------------------------
HanValue Jikaze::CalculateValue
(
	YAKU_CALCULATEVALUE_PARAMS()
)	const
{
	for ( HandGroup const& group : i_interp.m_groups )
	{
		if ( TripletCompatible( group.Type() ) )
		{
			if ( ValidTile( group[ 0 ], i_playerSeat ) )
			{
				return 1;
			}
		}
	}

	if ( i_interp.m_waitType == WaitType::Shanpon && ValidTile( i_lastTile.m_tile, i_playerSeat ) )
	{
		return 1;
	}

	return NoYaku;
}

/*static*/ bool Jikaze::ValidTile( Tile const& i_tile, Seat const& i_playerSeat )
{
	return i_tile.Type() == TileType::Wind && i_tile.Get<TileType::Wind>() == i_playerSeat;
}

//------------------------------------------------------------------------------
HanValue DoubleRiichi::CalculateValue
(
	YAKU_CALCULATEVALUE_PARAMS()
)	const
{
	if ( i_round.CalledDoubleRiichi( i_playerSeat ) )
	{
		return 2;
	}
	return NoYaku;
}

//------------------------------------------------------------------------------
HanValue Chantaiyao::CalculateValue
(
	YAKU_CALCULATEVALUE_PARAMS()
)	const
{
	for ( HandGroup const& group : i_interp.m_groups )
	{
		if ( !ranges::any_of( group.Tiles(), RequiredTile ) )
		{
			return NoYaku;
		}
	}

	// Check final group too
	if ( !RequiredTile( i_lastTile.m_tile )
		&& !ranges::any_of( i_interp.m_ungrouped, RequiredTile ) )
	{
		return NoYaku;
	}

	return i_assessment.m_open ? 1 : 2;
}

/*static*/ bool Chantaiyao::RequiredTile( Tile const& i_tile )
{
	return i_tile.IsHonourOrTerminal();
}

//------------------------------------------------------------------------------
HanValue SanshokuDoujun::CalculateValue
(
	YAKU_CALCULATEVALUE_PARAMS()
)	const
{
	// Note 1: Unfortunately, the fourth group and pair are totally unrelated, so we can't make quick rule-outs without checking all combos
	// Note 2: We can also stop when we find a match or when we are sure we don't have a match, so it's not too bad of a search at least.

	if ( !ranges::all_of( i_assessment.m_containsSuit, std::identity{} ) )
	{
		return NoYaku;
	}

	// TODO-OPT: I wonder if there's a more efficient way of assessing this than just comparing all triples of groups

	if ( SequenceWait( i_interp.m_waitType ) )
	{
		// Need to consider the final group, as it completed a sequence
		HandGroup const finalGroup( i_interp, i_lastTile.m_tile );

		for ( size_t groupI = 0; groupI < i_interp.m_groups.size(); ++groupI )
		{
			for ( size_t secondGroupI = groupI + 1; secondGroupI < i_interp.m_groups.size(); ++secondGroupI )
			{
				if ( Sanshoku( finalGroup, i_interp.m_groups[ groupI ], i_interp.m_groups[ secondGroupI ] ) )
				{
					return i_assessment.m_open ? 1 : 2;
				}
			}
		}
	}

	// Didn't find one with final group, so check the existing groups
	for ( size_t groupI = 0; groupI < i_interp.m_groups.size(); ++groupI )
	{
		for ( size_t secondGroupI = groupI + 1; secondGroupI < i_interp.m_groups.size(); ++secondGroupI )
		{
			for ( size_t thirdGroupI = secondGroupI + 1; thirdGroupI < i_interp.m_groups.size(); ++thirdGroupI )
			{
				if ( Sanshoku( i_interp.m_groups[ groupI ], i_interp.m_groups[ secondGroupI ], i_interp.m_groups[ thirdGroupI ] ) )
				{
					return i_assessment.m_open ? 1 : 2;
				}
			}
		}
	}

	// Failed to find matches
	return NoYaku;
}

/*static*/ bool SanshokuDoujun::Sanshoku( HandGroup const& i_a, HandGroup const& i_b, HandGroup const& i_c )
{
	if ( i_a.Type() != GroupType::Sequence || i_b.Type() != GroupType::Sequence || i_c.Type() != GroupType::Sequence )
	{
		return false;
	}

	if ( i_a.CommonSuit() == i_b.CommonSuit() || i_b.CommonSuit() == i_c.CommonSuit() || i_a.CommonSuit() == i_c.CommonSuit() )
	{
		return false;
	}

	return ranges::all_of(
		ranges::views::zip( i_a.Tiles(), i_b.Tiles(), i_c.Tiles() ),
		[]( auto const& i_tileSet )
		{
			auto const& [tileA, tileB, tileC] = i_tileSet;
			return tileA.template Get<TileType::Suit>().m_number == tileB.template Get<TileType::Suit>().m_number
				&& tileB.template Get<TileType::Suit>().m_number == tileC.template Get<TileType::Suit>().m_number;
		}
	);
}

//------------------------------------------------------------------------------
HanValue Ikkitsuukan::CalculateValue
(
	YAKU_CALCULATEVALUE_PARAMS()
)	const
{
	// This one isn't so bad. We'll make a matrix of suit x sequence and just search for all the groups we need

	static constexpr Array<Number, 3> c_requiredSequenceStarts
	{
		Number::One,
		Number::Four,
		Number::Seven,
	};

	Utils::EnumArray<Array<bool, 3>, Suits> groupsPerSuit{};

	auto fnEvalGroup = [ &groupsPerSuit ]( HandGroup const& i_group )
	{
		if ( i_group.Type() != GroupType::Sequence )
		{
			return;
		}

		for ( size_t i = 0; i < 3; ++i )
		{
			if ( i_group[ 0 ].Get<TileType::Suit>().m_number == c_requiredSequenceStarts[ i ] )
			{
				riEnsure( i_group[ 1 ].Get<TileType::Suit>().m_number == c_requiredSequenceStarts[ i ] + 1, "Invalid sequence provided" );
				riEnsure( i_group[ 2 ].Get<TileType::Suit>().m_number == c_requiredSequenceStarts[ i ] + 2, "Invalid sequence provided" );
				groupsPerSuit[ i_group.CommonSuit() ][ i ] = true;
				break;
			}
		}
	};

	if ( SequenceWait( i_interp.m_waitType ) )
	{
		// Need to consider the final group, as it completed a sequence
		HandGroup const finalGroup( i_interp, i_lastTile.m_tile );

		fnEvalGroup( finalGroup );
	}

	for ( HandGroup const& group : i_interp.m_groups )
	{
		fnEvalGroup( group );
	}

	if ( ranges::any_of( groupsPerSuit, []( auto const& arr ) { return ranges::all_of( arr, std::identity{} ); } ) )
	{
		return i_assessment.m_open ? 1 : 2;
	}

	// Failed to find ittsuu
	return NoYaku;
}

//------------------------------------------------------------------------------
HanValue Toitoi::CalculateValue
(
	YAKU_CALCULATEVALUE_PARAMS()
)	const
{
	for ( HandGroup const& group : i_interp.m_groups )
	{
		if ( group.Type() == GroupType::Sequence )
		{
			return NoYaku;
		}
	}

	// Check if we're completing a sequence too
	if ( SequenceWait( i_interp.m_waitType ) )
	{
		// Sequence wait
		return NoYaku;
	}

	return 2;
}

//------------------------------------------------------------------------------
HanValue Sanankou::CalculateValue
(
	YAKU_CALCULATEVALUE_PARAMS()
)	const
{
	int concealedTripleCount = 0;
	for ( HandGroup const& group : i_interp.m_groups )
	{
		if ( !group.Open() && TripletCompatible( group.Type() ) )
		{
			++concealedTripleCount;
		}
	}

	// Check if we're completing a closed triplet too
	if ( i_lastTile.m_type != TileDrawType::DiscardDraw && i_interp.m_waitType == WaitType::Shanpon )
	{
		++concealedTripleCount;
	}

	if ( concealedTripleCount >= 3 )
	{
		return 2;
	}

	return NoYaku;
}

//------------------------------------------------------------------------------
HanValue SanshokuDoukou::CalculateValue
(
	YAKU_CALCULATEVALUE_PARAMS()
)	const
{
	// Note 1: Unfortunately, the fourth group and pair are totally unrelated, so we can't make quick rule-outs without checking all combos
	// Note 2: We can also stop when we find a match or when we are sure we don't have a match, so it's not too bad of a search at least.

	if ( !ranges::all_of( i_assessment.m_containsSuit, std::identity{} ) )
	{
		return NoYaku;
	}

	// TODO-OPT: I wonder if there's a more efficient way of assessing this than just comparing all triples of groups

	if ( i_interp.m_waitType == WaitType::Shanpon )
	{
		// Need to consider the final group, as it completed a triplet
		HandGroup const finalGroup( i_interp, i_lastTile.m_tile );

		for ( size_t groupI = 0; groupI < i_interp.m_groups.size(); ++groupI )
		{
			for ( size_t secondGroupI = groupI + 1; secondGroupI < i_interp.m_groups.size(); ++secondGroupI )
			{
				if ( Sanshoku( finalGroup, i_interp.m_groups[ groupI ], i_interp.m_groups[ secondGroupI ] ) )
				{
					return 2;
				}
			}
		}
	}

	// Didn't find one with final group, so check the existing groups
	for ( size_t groupI = 0; groupI < i_interp.m_groups.size(); ++groupI )
	{
		for ( size_t secondGroupI = groupI + 1; secondGroupI < i_interp.m_groups.size(); ++secondGroupI )
		{
			for ( size_t thirdGroupI = secondGroupI + 1; thirdGroupI < i_interp.m_groups.size(); ++thirdGroupI )
			{
				if ( Sanshoku( i_interp.m_groups[ groupI ], i_interp.m_groups[ secondGroupI ], i_interp.m_groups[ thirdGroupI ] ) )
				{
					return 2;
				}
			}
		}
	}

	// Failed to find matches
	return NoYaku;
}

/*static*/ bool SanshokuDoukou::Sanshoku( HandGroup const& i_a, HandGroup const& i_b, HandGroup const& i_c )
{
	if ( !TripletCompatible( i_a.Type() ) || !TripletCompatible( i_b.Type() ) || !TripletCompatible( i_c.Type() ) )
	{
		return false;
	}

	// Check suits
	if ( i_a.CommonSuit() == i_b.CommonSuit() || i_b.CommonSuit() == i_c.CommonSuit() || i_a.CommonSuit() == i_c.CommonSuit() )
	{
		return false;
	}

	// Check values match
	return i_a.CommonNumber() == i_b.CommonNumber() && i_b.CommonNumber() == i_c.CommonNumber();
}

//------------------------------------------------------------------------------
HanValue Sankantsu::CalculateValue
(
	YAKU_CALCULATEVALUE_PARAMS()
)	const
{
	// NB don't need to check winning group as it can't be a quad
	int quadCount = 0;
	for ( HandGroup const& group : i_interp.m_groups )
	{
		if ( group.Type() == GroupType::Quad || group.Type() == GroupType::UpgradedQuad )
		{
			++quadCount;
		}
	}

	if ( quadCount >= 3 )
	{
		return 2;
	}

	return NoYaku;
}

//------------------------------------------------------------------------------
HanValue Chiitoitsu::CalculateValue
(
	YAKU_CALCULATEVALUE_PARAMS()
)	const
{
	if ( i_interp.m_waitType != WaitType::Tanki )
	{
		return NoYaku;
	}

	// TODO-OPT: Can we do this without filling a container?

	Set<Tile> uniqueTiles;
	uniqueTiles.insert( i_lastTile.m_tile );

	for ( HandGroup const& group : i_interp.m_groups )
	{
		if ( group.Type() != GroupType::Pair )
		{
			return NoYaku;
		}
		uniqueTiles.insert( group[ 0 ] );
	}

	if ( uniqueTiles.size() == 7 )
	{
		return 2;
	}
	return NoYaku;
}

//------------------------------------------------------------------------------
HanValue Honroutou::CalculateValue
(
	YAKU_CALCULATEVALUE_PARAMS()
)	const
{
	for ( HandGroup const& group : i_interp.m_groups )
	{
		if ( !ranges::all_of( group.Tiles(), ValidTile ) )
		{
			return NoYaku;
		}
	}

	// Check final group too
	if ( !ValidTile( i_lastTile.m_tile )
		|| !ranges::all_of( i_interp.m_ungrouped, ValidTile ) )
	{
		return NoYaku;
	}

	return 2;
}

/*static*/ bool Honroutou::ValidTile( Tile const& i_tile )
{
	return i_tile.IsHonourOrTerminal();
}

//------------------------------------------------------------------------------
HanValue Shousangen::CalculateValue
(
	YAKU_CALCULATEVALUE_PARAMS()
)	const
{
	if ( !i_assessment.m_containsTileType[ TileType::Dragon ] )
	{
		return NoYaku;
	}

	int dragonTripletCount = 0; // can't have more than one triplet of the same dragon so 2 triplets = 2 different dragons

	for ( HandGroup const& group : i_interp.m_groups )
	{
		if ( TripletCompatible( group.Type() ) )
		{
			if ( group.TilesType() == TileType::Dragon )
			{
				++dragonTripletCount;
			}
		}
		else if ( group.Type() == GroupType::Pair )
		{
			if ( group.TilesType() != TileType::Dragon )
			{
				return NoYaku;
			}
		}
	}

	if ( i_interp.m_waitType == WaitType::Tanki && i_lastTile.m_tile.Type() != TileType::Dragon )
	{
		return NoYaku;
	}
	else if ( i_interp.m_waitType == WaitType::Shanpon && i_lastTile.m_tile.Type() == TileType::Dragon )
	{
		++dragonTripletCount;
	}

	// Require exactly 2, as shousangen is exclusive from daisangen
	if ( dragonTripletCount == 2 )
	{
		return Yakuman;
	}

	return NoYaku;
}

//------------------------------------------------------------------------------
HanValue Honitsu::CalculateValue
(
	YAKU_CALCULATEVALUE_PARAMS()
)	const
{
	int suitTypeCount = 0;
	for ( Suit suit : Suits{} )
	{
		if ( i_assessment.m_containsSuit[ suit ]
			|| ( i_lastTile.m_tile.Type() == TileType::Suit && i_lastTile.m_tile.Get<TileType::Suit>().m_suit == suit ) )
		{
			++suitTypeCount;
		}
	}

	if ( suitTypeCount == 1 )
	{
		return i_assessment.m_open ? 2 : 3;
	}

	return NoYaku;
}

//------------------------------------------------------------------------------
HanValue JunchanTaiyao::CalculateValue
(
	YAKU_CALCULATEVALUE_PARAMS()
)	const
{
	// Only suit groups possible in a junchan hand
	if ( i_assessment.m_containsHonours || i_lastTile.m_tile.IsHonour() )
	{
		return NoYaku;
	}

	for ( HandGroup const& group : i_interp.m_groups )
	{
		if ( !ranges::any_of( group.Tiles(), RequiredTile ) )
		{
			return NoYaku;
		}
	}

	// Check final group too
	if ( !RequiredTile( i_lastTile.m_tile )
		&& !ranges::any_of( i_interp.m_ungrouped, RequiredTile ) )
	{
		return NoYaku;
	}

	return i_assessment.m_open ? 2 : 3;
}

/*static*/ bool JunchanTaiyao::RequiredTile( Tile const& i_tile )
{
	return i_tile.IsTerminal();
}

//------------------------------------------------------------------------------
HanValue Ryanpeikou::CalculateValue
(
	YAKU_CALCULATEVALUE_PARAMS()
)	const
{
	// Note 1: It's impossible to 'pick' a wrong pairing if we find an iipeikou. So just find one then check the remaining 2 groups
	// Note 2: Ryanpeikou needs 4 sequences, therefore it must involve the winning group unless it's a tanki wait

	if ( i_assessment.m_open || i_interp.m_waitType == WaitType::Shanpon )
	{
		return NoYaku;
	}

	// TODO-OPT: I wonder if there's a more efficient way of assessing this than just comparing all groups

	if ( i_interp.m_waitType == WaitType::Tanki )
	{
		for ( size_t groupI = 0; groupI < i_interp.m_groups.size(); ++groupI )
		{
			for ( size_t secondGroupI = groupI + 1; secondGroupI < i_interp.m_groups.size(); ++secondGroupI )
			{
				if ( IsMatchingSequence( i_interp.m_groups[ groupI ], i_interp.m_groups[ secondGroupI ] ) )
				{
					// We got one! Check the other!
					auto fnNextFree = [ = ]( size_t i ) { return i != groupI && i != secondGroupI; };
					size_t const thirdGroupI = Utils::NextFree( size_t( 0 ), fnNextFree );
					size_t const fourthGroupI = Utils::NextFree( thirdGroupI + 1, fnNextFree );
					if ( IsMatchingSequence( i_interp.m_groups[ thirdGroupI ], i_interp.m_groups[ fourthGroupI ] ) )
					{
						return 3;
					}
					return NoYaku;
				}
			}
		}
	}
	else
	{
		// Need to consider the final group, as it completed a sequence
		HandGroup const finalGroup( i_interp, i_lastTile.m_tile );

		for ( size_t groupI = 0; groupI < i_interp.m_groups.size(); ++groupI )
		{
			if ( IsMatchingSequence( finalGroup, i_interp.m_groups[ groupI ] ) )
			{
				// We got one! Check the other!
				auto fnNextFree = [ & ]( size_t i ) { return i != groupI && i_interp.m_groups[ i ].Type() != GroupType::Pair; };
				size_t const thirdGroupI = Utils::NextFree( size_t( 0 ), fnNextFree );
				size_t const fourthGroupI = Utils::NextFree( thirdGroupI + 1, fnNextFree );
				if ( IsMatchingSequence( i_interp.m_groups[ thirdGroupI ], i_interp.m_groups[ fourthGroupI ] ) )
				{
					return 3;
				}
				return NoYaku;
			}
		}
	}

	// Failed to find ryanpeikou
	return NoYaku;
}

/*static*/ bool Ryanpeikou::IsMatchingSequence( HandGroup const& i_a, HandGroup const& i_b )
{
	if ( i_a.Type() == GroupType::Sequence && i_b.Type() == GroupType::Sequence )
	{
		return i_a.Tiles() == i_b.Tiles();
	}
	return false;
}

//------------------------------------------------------------------------------
HanValue Chinitsu::CalculateValue
(
	YAKU_CALCULATEVALUE_PARAMS()
)	const
{
	if ( i_assessment.m_containsHonours || i_lastTile.m_tile.IsHonour() )
	{
		return NoYaku;
	}

	int suitTypeCount = 0;
	for ( Suit suit : Suits{} )
	{
		if ( i_assessment.m_containsSuit[ suit ] || i_lastTile.m_tile.Get<TileType::Suit>().m_suit == suit )
		{
			++suitTypeCount;
		}
	}

	if ( suitTypeCount == 1 )
	{
		return i_assessment.m_open ? 5 : 6;
	}

	return NoYaku;
}

//------------------------------------------------------------------------------
HanValue KokushiMusou::CalculateValue
(
	YAKU_CALCULATEVALUE_PARAMS()
)	const
{
	if ( i_assessment.m_open
		|| !ranges::all_of( i_assessment.m_containsTileType, std::identity{} )
		|| !ranges::all_of( i_assessment.m_containsSuit, std::identity{} )
		)
	{
		return NoYaku;
	}

	// TODO-OPT: Can we do this without filling a container?

	// Sufficient to check that all tiles are terminals/honours and that the distinct tile count >= 13
	Set<Tile> uniqueTiles;
	uniqueTiles.reserve( 14 );

	for ( HandGroup const& group : i_interp.m_groups )
	{
		for ( Tile const& tile : group.Tiles() )
		{
			if ( !tile.IsHonourOrTerminal() )
			{
				return NoYaku;
			}
			uniqueTiles.insert( tile );
		}
	}

	for ( Tile const& tile : i_interp.m_ungrouped )
	{
		if ( !tile.IsHonourOrTerminal() )
		{
			return NoYaku;
		}
		uniqueTiles.insert( tile );
	}

	// And finally the big tile itself
	if ( !i_lastTile.m_tile.IsHonourOrTerminal() )
	{
		return NoYaku;
	}
	uniqueTiles.insert( i_lastTile.m_tile );

	if ( uniqueTiles.size() >= 13 )
	{
		return Yakuman;
	}

	return NoYaku;
}

//------------------------------------------------------------------------------
HanValue Suuankou::CalculateValue
(
	YAKU_CALCULATEVALUE_PARAMS()
)	const
{
	int concealedTripleCount = 0;
	for ( HandGroup const& group : i_interp.m_groups )
	{
		if ( !group.Open() && TripletCompatible( group.Type() ) )
		{
			++concealedTripleCount;
		}
	}

	// Check if we're completing a closed triplet too
	if ( i_lastTile.m_type != TileDrawType::DiscardDraw && i_interp.m_waitType == WaitType::Shanpon )
	{
		++concealedTripleCount;
	}

	if ( concealedTripleCount >= 4 )
	{
		return Yakuman;
	}

	return NoYaku;
}

//------------------------------------------------------------------------------
HanValue Daisangen::CalculateValue
(
	YAKU_CALCULATEVALUE_PARAMS()
)	const
{
	if ( !i_assessment.m_containsTileType[ TileType::Dragon ] )
	{
		return NoYaku;
	}

	int dragonTripletCount = 0; // can't have more than one triplet of the same dragon so 3 triplets = 3 dragon types

	for ( HandGroup const& group : i_interp.m_groups )
	{
		if ( TripletCompatible( group.Type() ) )
		{
			if ( group.TilesType() == TileType::Dragon )
			{
				++dragonTripletCount;
			}
		}
	}

	if ( i_interp.m_waitType == WaitType::Shanpon && i_lastTile.m_tile.Type() == TileType::Dragon )
	{
		++dragonTripletCount;
	}

	if ( dragonTripletCount >= 3 )
	{
		return Yakuman;
	}

	return NoYaku;
}

//------------------------------------------------------------------------------
HanValue Shousuushii::CalculateValue
(
	YAKU_CALCULATEVALUE_PARAMS()
)	const
{
	if ( !i_assessment.m_containsTileType[ TileType::Wind ] )
	{
		return NoYaku;
	}

	int windTripletCount = 0; // can't have more than one triplet of the same wind so 3 triplets = 3 different winds

	for ( HandGroup const& group : i_interp.m_groups )
	{
		if ( TripletCompatible( group.Type() ) )
		{
			if ( group.TilesType() == TileType::Wind )
			{
				++windTripletCount;
			}
		}
		else if ( group.Type() == GroupType::Pair )
		{
			if ( group.TilesType() != TileType::Wind )
			{
				return NoYaku;
			}
		}
	}

	if ( i_interp.m_waitType == WaitType::Tanki && i_lastTile.m_tile.Type() != TileType::Wind )
	{
		return NoYaku;
	}
	else if ( i_interp.m_waitType == WaitType::Shanpon && i_lastTile.m_tile.Type() == TileType::Wind )
	{
		++windTripletCount;
	}

	// Require exactly 3, as shousuushii is exclusive from daisuushii
	if ( windTripletCount == 3 )
	{
		return Yakuman;
	}

	return NoYaku;
}

//------------------------------------------------------------------------------
HanValue Daisuushii::CalculateValue
(
	YAKU_CALCULATEVALUE_PARAMS()
)	const
{
	if ( !i_assessment.m_containsTileType[ TileType::Wind ] )
	{
		return NoYaku;
	}

	int windTripletCount = 0; // can't have more than one triplet of the same wind so 4 triplets = 4 big winds

	for ( HandGroup const& group : i_interp.m_groups )
	{
		if ( TripletCompatible( group.Type() ) )
		{
			if ( group.TilesType() == TileType::Wind )
			{
				++windTripletCount;
			}
		}
	}

	if ( i_interp.m_waitType == WaitType::Shanpon && i_lastTile.m_tile.Type() == TileType::Wind )
	{
		++windTripletCount;
	}

	if ( windTripletCount >= 4 )
	{
		return Yakuman;
	}

	return NoYaku;
}

//------------------------------------------------------------------------------
HanValue Tsuuiisou::CalculateValue
(
	YAKU_CALCULATEVALUE_PARAMS()
)	const
{
	if ( i_assessment.m_containsTileType[ TileType::Suit ] || i_lastTile.m_tile.Type() == TileType::Suit )
	{
		return NoYaku;
	}

	return Yakuman;
}

//------------------------------------------------------------------------------
HanValue Chinroutou::CalculateValue
(
	YAKU_CALCULATEVALUE_PARAMS()
)	const
{
	if ( i_assessment.m_containsHonours || !i_assessment.m_containsTerminals || i_lastTile.m_tile.IsHonour() || !i_lastTile.m_tile.IsTerminal() )
	{
		return NoYaku;
	}

	for ( HandGroup const& group : i_interp.m_groups )
	{
		if ( !ranges::all_of( group.Tiles(), RequiredTile ) )
		{
			return NoYaku;
		}
	}

	if ( !RequiredTile( i_lastTile.m_tile )
		|| !ranges::all_of( i_interp.m_ungrouped, RequiredTile ) )
	{
		return NoYaku;
	}

	return Yakuman;
}

/*static*/ bool Chinroutou::RequiredTile( Tile const& i_tile )
{
	return i_tile.IsTerminal();
}

//------------------------------------------------------------------------------
HanValue Ryuuiisou::CalculateValue
(
	YAKU_CALCULATEVALUE_PARAMS()
)	const
{
	// Early-out on some broad strokes, for optimisation
	if ( i_assessment.m_containsTileType[ TileType::Wind ]
		|| i_assessment.m_containsSuit[ Suit::Manzu ]
		|| i_assessment.m_containsSuit[ Suit::Pinzu ]
		|| !i_assessment.m_containsSuit[ Suit::Souzu ]
		|| i_lastTile.m_tile.Type() == TileType::Wind
		|| ( i_lastTile.m_tile.Type() == TileType::Suit && i_lastTile.m_tile.Get<TileType::Suit>().m_suit != Suit::Souzu ) )
	{
		return NoYaku;
	}

	for ( HandGroup const& group : i_interp.m_groups )
	{
		if ( !ranges::all_of( group.Tiles(), RequiredTile ) )
		{
			return NoYaku;
		}
	}

	if ( !RequiredTile( i_lastTile.m_tile )
		|| !ranges::all_of( i_interp.m_ungrouped, RequiredTile ) )
	{
		return NoYaku;
	}

	return Yakuman;
}

/*static*/ bool Ryuuiisou::RequiredTile( Tile const& i_tile )
{
	if ( i_tile.Type() == TileType::Suit )
	{
		// Already checked tile is souzu
		SuitTile const& suitTile = i_tile.Get<TileType::Suit>();
		return suitTile.m_number == Number::Two
			|| suitTile.m_number == Number::Three
			|| suitTile.m_number == Number::Four
			|| suitTile.m_number == Number::Six
			|| suitTile.m_number == Number::Eight
			;
	}

	// Already checked tile, if not suit, is a dragon
	return i_tile.Get<TileType::Dragon>() == DragonTileType::Green;
}

//------------------------------------------------------------------------------
HanValue ChuurenPoutou::CalculateValue
(
	YAKU_CALCULATEVALUE_PARAMS()
)	const
{
	if ( i_assessment.m_open || i_assessment.m_containsHonours || i_lastTile.m_tile.IsHonour() )
	{
		return NoYaku;
	}

	Suit const requiredSuit = i_lastTile.m_tile.Get<TileType::Suit>().m_suit;

	Utils::EnumArray<int, Numbers> requiredOfEachValue = {
		3, 1, 1,
		1, 1, 1,
		1, 1, 3,
	};

	auto fnEvalTile = [ requiredSuit, &requiredOfEachValue ]( Tile const& i_tile )
	{
		if ( i_tile.Get<TileType::Suit>().m_suit != requiredSuit )
		{
			return false;
		}
		--requiredOfEachValue[ i_tile.Get<TileType::Suit>().m_number ];
		return true;
	};

	for ( HandGroup const& group : i_interp.m_groups )
	{
		for ( Tile const& tile : group.Tiles() )
		{
			if ( !fnEvalTile( tile ) )
			{
				return NoYaku;
			}
		}
	}

	for ( Tile const& tile : i_interp.m_ungrouped )
	{
		if ( !fnEvalTile( tile ) )
		{
			return NoYaku;
		}
	}

	// And finally the big tile itself
	fnEvalTile( i_lastTile.m_tile );

	// If every value is 0 or less (technically, there should be exactly one value with -1, the others all 0)
	// then we have the yakuman
	if ( ranges::all_of( requiredOfEachValue, []( int n ) { return n <= 0; } ) )
	{
		return Yakuman;
	}

	return NoYaku;
}

//------------------------------------------------------------------------------
HanValue Suukantsu::CalculateValue
(
	YAKU_CALCULATEVALUE_PARAMS()
)	const
{
	int kanCount = 0;
	for ( HandGroup const& group : i_interp.m_groups )
	{
		if ( group.Type() == GroupType::Quad || group.Type() == GroupType::UpgradedQuad )
		{
			++kanCount;
		}
	}

	// N.B. winning tile is necessarily a pair, so no need to check the wait tiles

	if ( kanCount >= 4 )
	{
		return Yakuman;
	}

	return NoYaku;
}

//------------------------------------------------------------------------------
HanValue Tenhou::CalculateValue
(
	YAKU_CALCULATEVALUE_PARAMS()
)	const
{
	if ( i_round.IsDealer( i_playerSeat ) && i_round.Discards( i_playerSeat ).empty() )
	{
		return Yakuman;
	}

	return NoYaku;
}

//------------------------------------------------------------------------------
HanValue Chihou::CalculateValue
(
	YAKU_CALCULATEVALUE_PARAMS()
)	const
{
	if ( i_lastTile.m_type == TileDrawType::SelfDraw
		&& !i_round.CallsMade()
		&& !i_round.IsDealer( i_playerSeat )
		&& i_round.Discards( i_playerSeat ).empty()
		)
	{
		return Yakuman;
	}

	return NoYaku;
}

}
