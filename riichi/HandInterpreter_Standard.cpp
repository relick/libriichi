#include "HandInterpreter_Standard.hpp"

#include "Hand.hpp"

#include <algorithm>
#include "range/v3/action.hpp"
#include "range/v3/algorithm.hpp"

namespace Riichi
{

//------------------------------------------------------------------------------
/*static*/ uint32_t StandardInterpreter::Rank
(
	HandInterpretation const& i_interp
)
{
	// Bits 0-3 for four triplets
	// Bits 4-7 for four sequences
	// Bit 8 for a pair
	uint32_t rank = 0;
	int tripleCount = 0;
	int sequenceCount = 4;
	for ( HandGroup const& group : i_interp.m_groups )
	{
		switch ( group.Type() )
		{
		case GroupType::Pair:
		{
			rank |= 1 << 8;
			break;
		}
		case GroupType::Triplet:
		case GroupType::Quad:
		case GroupType::UpgradedQuad:
		{
			rank |= 1 << tripleCount++;
			break;
		}
		case GroupType::Sequence:
		{
			rank |= 1 << sequenceCount++;
			break;
		}
		}
	}

	return rank;
}

//------------------------------------------------------------------------------
/*static*/ void StandardInterpreter::SetWait
(
	HandInterpretation& io_interpToSet
)
{
	// Go by order of simplicity
	if ( io_interpToSet.m_ungrouped.size() > 2 )
	{
		// Can't have waits if there's more than 2 tiles remaining
		return;
	}

	// Check for tanki first
	if ( io_interpToSet.m_ungrouped.size() == 1 )
	{
		io_interpToSet.m_waitType = WaitType::Tanki;
		io_interpToSet.m_waits.insert( io_interpToSet.m_ungrouped.front() );
		return;
	}

	riEnsure( io_interpToSet.m_ungrouped.size() == 2, "Wasn't a tanki wait but didn't have 2 tiles remaining" );

	// Check for shanpon second
	if ( io_interpToSet.m_ungrouped.front() == io_interpToSet.m_ungrouped.back() )
	{
		io_interpToSet.m_waitType = WaitType::Shanpon;
		io_interpToSet.m_waits.insert( io_interpToSet.m_ungrouped.front() );
		return;
	}

	// If not shanpon then work out the type of sequence wait, if there is one
	if ( io_interpToSet.m_ungrouped.front().Type() != TileType::Suit || io_interpToSet.m_ungrouped.back().Type() != TileType::Suit )
	{
		// Different tile types, no wait
		return;
	}

	SuitTile const& tile1 = io_interpToSet.m_ungrouped.front().Get<TileType::Suit>();
	SuitTile const& tile2 = io_interpToSet.m_ungrouped.back().Get<TileType::Suit>();

	if ( tile1.m_suit != tile2.m_suit )
	{
		// Different suits, no wait
		return;
	}

	// Tiles are sorted so this should always be a valid thing to ask - tile2 is either 1 or 2 above tile1
	if ( tile2.m_value == tile1.m_value + SuitTileValue::Set<1>() )
	{
		if ( tile1.m_value == SuitTileValue::Min )
		{
			// Bottom edge
			io_interpToSet.m_waitType = WaitType::Penchan;
			io_interpToSet.m_waits.insert( SuitTile{ tile2.m_suit, tile2.m_value + SuitTileValue::Set<1>() } );
			return;
		}
		else if ( tile2.m_value == SuitTileValue::Max )
		{
			// Top edge
			io_interpToSet.m_waitType = WaitType::Penchan;
			io_interpToSet.m_waits.insert( SuitTile{ tile1.m_suit, tile1.m_value - SuitTileValue::Set<1>() } );
			return;
		}

		// Open
		io_interpToSet.m_waitType = WaitType::Ryanmen;
		io_interpToSet.m_waits.insert( SuitTile{ tile1.m_suit, tile1.m_value - SuitTileValue::Set<1>() } );
		io_interpToSet.m_waits.insert( SuitTile{ tile2.m_suit, tile2.m_value + SuitTileValue::Set<1>() } );
		return;
	}
	else if ( tile2.m_value == tile1.m_value + SuitTileValue::Set<2>() )
	{
		// Middle wait
		io_interpToSet.m_waitType = WaitType::Kanchan;
		io_interpToSet.m_waits.insert( SuitTile{ tile1.m_suit, tile1.m_value + SuitTileValue::Set<1>() } );
		return;
	}

	// Remaining pair of suit tiles has no wait on them
}

//------------------------------------------------------------------------------
/*static*/ void StandardInterpreter::PushInterp
(
	Vector<HandInterpretation>& io_interps,
	HandInterpretation& io_interpToPush
)
{
	SetWait( io_interpToPush );

	// TODO-AI: It's possible we actually want to keep the lesser ranking interpretations in case AI wants to pursue them. Not sure what situations we'd eliminate a useful hand though
	uint32_t const newRank = Rank( io_interpToPush );
	for ( auto interpI = io_interps.begin(); interpI != io_interps.end(); /*++interpI*/ )
	{
		uint32_t const existingRank = Rank( *interpI );
		if ( existingRank != newRank )
		{
			if ( ( newRank & existingRank ) == newRank )
			{
				// New interp superseded by this one, so we won't add it
				return;
			}
			else if ( ( newRank & existingRank ) == existingRank )
			{
				// New interp supersedes this one so erase this one
				interpI = io_interps.erase( interpI );
				continue;
			}
		}
		++interpI;
	}

	io_interps.push_back( std::move( io_interpToPush ) );
}

//------------------------------------------------------------------------------
/*static*/ void StandardInterpreter::RecursivelyGenerate
(
	Vector<HandInterpretation>& io_interps,
	HandInterpretation i_soFar,
	Vector<Tile> i_sortedRemaining,
	size_t i_nextTileI
)
{
	// Make groups from current tile, when we've run out, work out the wait from the remaining tiles
	if ( i_sortedRemaining.size() < 3 )
	{
		i_soFar.m_ungrouped = i_sortedRemaining;
		PushInterp( io_interps, i_soFar );
		return;
	}

	bool const needPair = !ranges::any_of( i_soFar.m_groups, []( HandGroup const& group ) { return group.Type() == GroupType::Pair; } );

	bool madeGroups = false;
	// TODO-DEBT: This code needs tidying up, and in general the recursive nature could be optimised to avoid copying vectors
	for ( size_t tileI = i_nextTileI; tileI < i_sortedRemaining.size() - 1; ++tileI )
	{
		Tile const& tile = i_sortedRemaining[ tileI ];
		Tile const& nextTile = i_sortedRemaining[ tileI + 1 ];

		// Try to make pair
		if ( needPair && nextTile == tile )
		{
			HandInterpretation withPair = i_soFar;
			withPair.m_groups.push_back( HandGroup( { tile, nextTile }, GroupType::Pair, false ) );
			Vector<Tile> remTiles = i_sortedRemaining;
			remTiles.erase( remTiles.begin() + tileI, remTiles.begin() + tileI + 2 );
			RecursivelyGenerate( io_interps, withPair, remTiles, tileI );
			madeGroups = true;
		}

		if ( tileI >= i_sortedRemaining.size() - 2 )
		{
			// Ran out of tiles to check
			break;
		}

		// Try to make triplet
		Tile const& nextNextTile = i_sortedRemaining[ tileI + 2 ];
		if ( nextTile == tile && nextNextTile == tile )
		{
			HandInterpretation withTriplet = i_soFar;
			withTriplet.m_groups.push_back( HandGroup( { tile, nextTile, nextNextTile }, GroupType::Triplet, false ) );
			Vector<Tile> remTiles = i_sortedRemaining;
			remTiles.erase( remTiles.begin() + tileI, remTiles.begin() + tileI + 3 );
			RecursivelyGenerate( io_interps, withTriplet, remTiles, tileI );
			madeGroups = true;
		}

		// Try to make sequence
		if ( tile.Type() == TileType::Suit )
		{
			SuitTile const& suitTile = tile.Get<TileType::Suit>();
			if ( suitTile.m_value >= SuitTileValue::Max - SuitTileValue::Set<1>() )
			{
				// Can't find two tiles of higher value as this one is already too high
				continue;
			}
			bool gotSequence = false;
			for ( size_t tile2I = tileI + 1; tile2I < i_sortedRemaining.size(); ++tile2I )
			{
				Tile const& tile2 = i_sortedRemaining[ tile2I ];
				if ( tile2.Type() == TileType::Suit )
				{
					SuitTile const& suitTile2 = tile2.Get<TileType::Suit>();
					if ( suitTile.m_suit == suitTile2.m_suit && suitTile2.m_value == suitTile.m_value + SuitTileValue::Set<1>() )
					{
						// Found second tile, try for third
						for ( size_t tile3I = tile2I + 1; tile3I < i_sortedRemaining.size(); ++tile3I )
						{
							Tile const& tile3 = i_sortedRemaining[ tile3I ];
							if ( tile3.Type() == TileType::Suit )
							{
								SuitTile const& suitTile3 = tile3.Get<TileType::Suit>();
								if ( suitTile.m_suit == suitTile3.m_suit && suitTile3.m_value == suitTile2.m_value + SuitTileValue::Set<1>() )
								{
									// Found three tiles
									HandInterpretation withSeq = i_soFar;
									withSeq.m_groups.push_back( HandGroup( { tile, tile2, tile3 }, GroupType::Sequence, false ) );
									Vector<Tile> remTiles = i_sortedRemaining;
									remTiles.erase( remTiles.begin() + tile3I );
									remTiles.erase( remTiles.begin() + tile2I );
									remTiles.erase( remTiles.begin() + tileI );
									RecursivelyGenerate( io_interps, withSeq, remTiles, tileI );
									madeGroups = true;
									gotSequence = true;

									// TODO-DEBT: Look at this nesting:
								}
							}
							if ( gotSequence )
							{
								break;
							}
						}
					}
				}
				if ( gotSequence )
				{
					break;
				}
			}
		}
	}

	// Failed to make any more groups with this hand so it's an interp on its own
	if ( !madeGroups )
	{
		i_soFar.m_ungrouped = i_sortedRemaining;
		PushInterp( io_interps, i_soFar );
	}
}

//------------------------------------------------------------------------------
void StandardInterpreter::AddInterpretations
(
	Vector<HandInterpretation>& io_interps,
	HandInterpretation const& i_fixedPart,
	Vector<Tile> const& i_sortedFreeTiles
)	const
{
	// Standard hand interpretations. This is for the typical "4 groups + 1 pair" type of hand.
	// We'll recursively generate this in the following way (knowing the free tiles are sorted):
	// - Iterate the tiles
	// - For each tile, check if a pair, triplet, or ascending sequence can be formed using tiles to the right
	// - If there is any of these, add the group to our running interpretation and recurse, starting a search from the next tile
	// - If we run out of groups to generate from the remaining tiles, then push the interpretation after assessing any waits
	// - We also eliminate any interpretations that are "superseded" by another interpretation.
	//  IMPORTANT: The idea being that between "1 pair" and "1 pair + 1 sequence", the latter is always better and the former doesn't need considering
	//             This does two things: covers the greedy algorithm's ass by restricting to the output to something that it will always successfully generate
	//             and means we don't bother considering 'weaker' interpretations later.
	//             But it comes with the caveat that there *may* be a novel interpretation worth considering that happened to be ranked weaker. TODO-DEBT: test whether these actually exist

	RecursivelyGenerate( io_interps, i_fixedPart, i_sortedFreeTiles, 0 );
}

//------------------------------------------------------------------------------
void SevenPairsInterpreter::AddInterpretations
(
	Vector<HandInterpretation>& io_interps,
	HandInterpretation const& i_fixedPart,
	Vector<Tile> const& i_sortedFreeTiles
)	const
{
	// Unlike standard interpretations, there's only one way this can go.
	// We'll pretty much implement the full yaku here (the yaku calculation can then just confirm things are as expected)

	// Requires hand with no melds
	if ( !i_fixedPart.m_groups.empty() )
	{
		return;
	}

	// Fill as many pairs as we can make - there's only ever 1 interpretation with chiitoitsu
	HandInterpretation interp = i_fixedPart;

	for ( size_t tileI = 0; tileI < i_sortedFreeTiles.size(); ++tileI )
	{
		Tile const& tile1 = i_sortedFreeTiles[ tileI ];

		if ( tileI == i_sortedFreeTiles.size() - 1 )
		{
			interp.m_ungrouped.push_back( tile1 );
			break;
		}

		Tile const& tile2 = i_sortedFreeTiles[ tileI + 1 ];

		// Matching and unique
		if ( tile1 == tile2 && !ranges::any_of( interp.m_groups, [ & ]( HandGroup const& group ) { return group[ 0 ] == tile1; } ) )
		{
			interp.m_groups.push_back( HandGroup{ {tile1, tile2}, GroupType::Pair, false } );
			tileI++;
		}
		else
		{
			interp.m_ungrouped.push_back( tile1 );
		}
	}
	
	if ( interp.m_ungrouped.size() == 1 && !ranges::any_of( interp.m_groups, [ & ]( HandGroup const& group ) { return group[ 0 ] == interp.m_ungrouped[ 0 ]; } ) )
	{
		interp.m_waits.insert( interp.m_ungrouped[ 0 ] );
		interp.m_waitType = WaitType::Tanki;
	}

	io_interps.push_back( std::move( interp ) );

}

//------------------------------------------------------------------------------
void ThirteenOrphansInterpreter::AddInterpretations
(
	Vector<HandInterpretation>& io_interps,
	HandInterpretation const& i_fixedPart,
	Vector<Tile> const& i_sortedFreeTiles
)	const
{
	// Unlike standard interpretations, there's only one way this can go.
	// We'll pretty much implement the full yaku here (the yaku calculation can then just confirm things are as expected)

	// Requires hand with no melds
	if ( !i_fixedPart.m_groups.empty() )
	{
		return;
	}

	auto fnRequiredTile = []( Tile const& i_tile )
	{
		if ( i_tile.Type() != TileType::Suit )
		{
			return true;
		}
		SuitTile const& suitTile = i_tile.Get<TileType::Suit>();
		return suitTile.m_value == 1 || suitTile.m_value == 9;
	};

	// We're going to add all free tiles to the ungrouped list
	// But then assess the waits based on if we actually have kokushi musou
	HandInterpretation interp = i_fixedPart;
	interp.m_ungrouped = i_sortedFreeTiles;

	// TODO-OPT: Can we do this without filling a container?

	Vector<Tile> requiredTiles{
		SuitTile{ Suit::Manzu, SuitTileValue::Min },
		SuitTile{ Suit::Manzu, SuitTileValue::Max },
		SuitTile{ Suit::Pinzu, SuitTileValue::Min },
		SuitTile{ Suit::Pinzu, SuitTileValue::Max },
		SuitTile{ Suit::Souzu, SuitTileValue::Min },
		SuitTile{ Suit::Souzu, SuitTileValue::Max },
		DragonTileType::White,
		DragonTileType::Green,
		DragonTileType::Red,
		WindTileType::East,
		WindTileType::South,
		WindTileType::West,
		WindTileType::North,
	};

	for ( Tile const& tile : interp.m_ungrouped )
	{
		if ( !fnRequiredTile( tile ) )
		{
			// We have an invalid tile for 13 orphans, so definitely no waits here
			io_interps.push_back( std::move( interp ) );
			return;
		}
		std::erase( requiredTiles, tile );
	}
	
	if ( requiredTiles.size() > 1 )
	{
		// Don't have enough unique tiles to have a wait for 13 orphans
		io_interps.push_back( std::move( interp ) );
		return;
	}

	// Must have a wait if we reached this far
	interp.m_waitType = WaitType::Tanki;

	if ( requiredTiles.size() == 1 )
	{
		// Have a pair inside the hand already, so there's exactly 1 tile left
		interp.m_waits.insert( requiredTiles[ 0 ] );
	}
	else
	{
		riEnsure( requiredTiles.size() == 0, "Did not have a valid number of unique tiles when assessing 13 orphans" );
		// We have 1 of every tile, so this is a 13 tile wait. Luckily we have a full set of 13 tiles in our ungrouped list! :)
		ranges::actions::insert( interp.m_waits, interp.m_ungrouped );
	}

	io_interps.push_back( std::move( interp ) );
	return;
}

}
