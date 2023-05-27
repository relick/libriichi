#include "Hand.hpp"

#include "Utils.hpp"

#include <algorithm>
#include <ranges>

namespace Riichi
{

//------------------------------------------------------------------------------
void Hand::AddFreeTiles
(
	Vector<Tile> const& i_newTiles
)
{
	m_freeTiles.insert_range( m_freeTiles.end(), i_newTiles );
	std::ranges::sort( m_freeTiles ); // TODO-QOL: players may not always want their hand sorted
}

//------------------------------------------------------------------------------
void Hand::Discard
(
	Tile const& i_toDiscard,
	Tile const& i_drawToAdd
)
{
	Utils::EraseOne( m_freeTiles,
		[ & ]( Tile const& i_tile )
		{
			return StrictEqualTo( i_tile, i_toDiscard );
		}
	);
	m_freeTiles.push_back( i_drawToAdd );
	std::ranges::sort( m_freeTiles ); // TODO-QOL: players may not always want their hand sorted
}

//------------------------------------------------------------------------------
void Hand::MakeMeld
(
	Pair<Seat, Tile> const& i_meldTile,
	Pair<Tile, Tile> const& i_otherTiles,
	GroupType i_meldType
)
{
	Ensure( i_meldType != GroupType::Quad, "Must call MakeKan instead of MakeMeld for quads" );

	Utils::EraseOne(
		m_freeTiles,
		[ & ]( Tile const& i_tile )
		{
			return StrictEqualTo( i_tile, i_otherTiles.first );
		}
	);

	Utils::EraseOne(
		m_freeTiles,
		[ & ]( Tile const& i_tile )
		{
			return StrictEqualTo( i_tile, i_otherTiles.second );
		}
	);

	Meld newMeld;
	newMeld.m_tiles.push_back( { i_meldTile.second, i_meldTile.first } );
	newMeld.m_tiles.push_back( { i_otherTiles.first, std::nullopt } );
	newMeld.m_tiles.push_back( { i_otherTiles.second, std::nullopt } );
	newMeld.m_type = i_meldType;
	newMeld.m_open = true;

	m_melds.push_back( std::move( newMeld ) );
}

//------------------------------------------------------------------------------
void Hand::MakeKan
(
	Tile const& i_meldTile,
	Option<Seat> i_calledFrom
)
{
	// First check existing triplet melds
	for ( Meld& meld : m_melds )
	{
		if ( meld.m_type == GroupType::Triplet && meld.m_tiles.front().first == i_meldTile )
		{
			// Meld comes from our own hand
			Ensure( !i_calledFrom.has_value(), "Cannot call kan on already open meld" );

			meld.m_tiles.push_back( { i_meldTile, std::nullopt } );
			meld.m_type = GroupType::Quad;
			return;
		}
	}

	// Then check hand

	Meld newMeld;
	if ( i_calledFrom.has_value() )
	{
		newMeld.m_tiles.push_back( { i_meldTile, i_calledFrom } );
	}
	std::erase_if(
		m_freeTiles,
		[ & ]( Tile const& i_tile )
		{
			if ( i_tile == i_meldTile )
			{
				newMeld.m_tiles.push_back( { i_tile, std::nullopt } );
				return true;
			}
			return false;
		}
	);

	Ensure( newMeld.m_tiles.size() == 4, "Did not find 4 tiles for kan" );

	newMeld.m_type = GroupType::Quad;
	newMeld.m_open = i_calledFrom.has_value();

	m_melds.push_back( std::move( newMeld ) );
}

//------------------------------------------------------------------------------
Vector<Pair<Tile, Tile>> Hand::ChiOptions
(
	Tile const& i_tile
)	const
{
	if ( i_tile.Type() != TileType::Suit )
	{
		// Cannot chi if not suit tile
		return {};
	}

	SuitTile const& suitTile = i_tile.Get<TileType::Suit>();

	// It's a little complicated searching for chi options
	// We have to check for 3 shapes: DHH HDH HHD where D is the discarded tile and H are hand tiles
	// We also want to multiply chi options based on strict tile comparison (so aka dora are treated as separate options)
	// It works quite well then to reuse a pair of sets and search for the tiles we need in each shape, then fill out options from that as a cartesian product

	Vector<Pair<Tile, Tile>> options;
	Set<Tile, StrictEqualToTile> tiles1;
	Set<Tile, StrictEqualToTile> tiles2;

	auto fnSearchForTiles = [ & ]( Tile const& i_search1, Tile const& i_search2 )
	{
		for ( Tile const& tile : m_freeTiles )
		{
			if ( tile == i_search1 )
			{
				tiles1.insert( tile );
			}
			else if ( tile == i_search2 )
			{
				tiles2.insert( tile );
			}
		}

		if ( !tiles1.empty() && !tiles2.empty() )
		{
			// Found shape, cartesian product the options
			for ( Tile const& tile1 : tiles1 )
			{
				for ( Tile const& tile2 : tiles2 )
				{
					options.push_back( { tile1, tile2 } );
				}
			}
		}

		tiles1.clear();
		tiles2.clear();
	};

	if ( suitTile.m_value <= SuitTileValue::Max - SuitTileValue::Set<2>() )
	{
		SuitTile const oneUp{ suitTile.m_suit, suitTile.m_value + SuitTileValue::Set<1>() };
		SuitTile const twoUp{ suitTile.m_suit, suitTile.m_value + SuitTileValue::Set<2>() };
		fnSearchForTiles( oneUp, twoUp );
	}

	if ( suitTile.m_value >= SuitTileValue::Min + SuitTileValue::Set<1>() && suitTile.m_value <= SuitTileValue::Max - SuitTileValue::Set<1>() )
	{
		SuitTile const oneDown{ suitTile.m_suit, suitTile.m_value - SuitTileValue::Set<1>() };
		SuitTile const oneUp{ suitTile.m_suit, suitTile.m_value + SuitTileValue::Set<1>() };
		fnSearchForTiles( oneDown, oneUp );
	}

	if ( suitTile.m_value >= SuitTileValue::Min + SuitTileValue::Set<2>() )
	{
		SuitTile const twoDown{ suitTile.m_suit, suitTile.m_value - SuitTileValue::Set<2>() };
		SuitTile const oneDown{ suitTile.m_suit, suitTile.m_value - SuitTileValue::Set<1>() };
		fnSearchForTiles( twoDown, oneDown );
	}

	return options;
}

//------------------------------------------------------------------------------
bool Hand::CanPon
(
	Tile const& i_tile
)	const
{
	size_t const othersCount = std::ranges::count( m_freeTiles, i_tile );
	return othersCount >= 2;
}

//------------------------------------------------------------------------------
bool Hand::CanCallKan
(
	Tile const& i_tile
)	const
{
	size_t const othersCount = std::ranges::count( m_freeTiles, i_tile );
	return othersCount >= 3;
}

//------------------------------------------------------------------------------
Vector<Hand::DrawKanResult> Hand::DrawKanOptions
(
	Tile const* i_drawnTile
)	const
{
	Vector<DrawKanResult> results;

	if ( i_drawnTile )
	{
		for ( Meld const& meld : m_melds )
		{
			if ( meld.m_type == GroupType::Triplet && meld.m_tiles.front().first == *i_drawnTile )
			{
				results.push_back( { *i_drawnTile, false } );
				break;
			}
		}

		if ( results.empty() )
		{
			size_t const othersCount = std::ranges::count( m_freeTiles, *i_drawnTile );
			results.push_back( { *i_drawnTile, true } );
		}
	}

	for ( Meld const& meld : m_melds )
	{
		if ( meld.m_type == GroupType::Triplet && std::ranges::contains( std::views::elements<0>( meld.m_tiles ), meld.m_tiles.front().first ) )
		{
			results.push_back( { meld.m_tiles.front().first, false } );
			break;
		}
	}

	// Could have more than one set of 4 in hand, so just search for them left to right
	// n^2 but code is simple and n is small
	for ( auto tileI = m_freeTiles.begin(); tileI != m_freeTiles.end(); ++tileI )
	{
		size_t const tilesOfType = std::count( tileI, m_freeTiles.end(), *tileI );
		if ( tilesOfType >= 4 )
		{
			results.push_back( { *tileI, true } );
		}
	}

	return results;
}

//------------------------------------------------------------------------------
std::ostream& operator<<( std::ostream& io_out, Hand const& i_hand )
{
	Vector<Tile> sortedTiles = i_hand.m_freeTiles;
	std::ranges::sort( sortedTiles );

	auto fnPrintSuit = [ &io_out ]( Suit suit )
	{
		switch ( suit )
		{
		case Suit::Manzu: io_out << 'm'; break;
		case Suit::Pinzu: io_out << 'p'; break;
		case Suit::Souzu: io_out << 's'; break;
		}
	};

	Option<Suit> lastSuit;
	for ( Tile const& tile : sortedTiles )
	{
		if ( tile.Type() == TileType::Suit )
		{
			if ( lastSuit.has_value() && lastSuit.value() != tile.Get<TileType::Suit>().m_suit )
			{
				fnPrintSuit( lastSuit.value() );
			}
			lastSuit = tile.Get<TileType::Suit>().m_suit;
			io_out << static_cast< int >( tile.Get<TileType::Suit>().m_value.m_val );
		}
		else
		{
			if ( lastSuit.has_value() )
			{
				fnPrintSuit( lastSuit.value() );
				lastSuit.reset();
			}
			io_out << tile;
		}
	}
	if ( lastSuit.has_value() )
	{
		fnPrintSuit( lastSuit.value() );
	}

	for ( Meld const& meld : i_hand.m_melds )
	{
		io_out << ' ';

		for ( Meld::MeldTile const& tile : meld.m_tiles )
		{
			io_out << tile.first;
		}
	}

	io_out << '\n';

	return io_out;
}

//------------------------------------------------------------------------------
HandGroup::HandGroup
(
	Vector<Tile> i_tiles,
	GroupType i_type,
	bool i_open
)
	: m_tiles( std::move( i_tiles ) )
	, m_type{ i_type }
	, m_open{ i_open }
{
	// Verify tiles are acceptable groups
#ifndef NDEBUG
	switch ( m_type )
	{
		using enum GroupType;
	case Pair:
	{
		Ensure( m_tiles.size() == 2, "Pair group didn't have 2 tiles" );
		Ensure( std::ranges::adjacent_find( m_tiles, std::not_equal_to{} ) == m_tiles.end(), "Pair group didn't have matching tiles" );
		break;
	}
	case Sequence:
	{
		Ensure( m_tiles.size() == 3, "Sequence group didn't have 3 tiles" );
		Ensure( std::ranges::all_of( m_tiles, []( Tile const& i_t ) { return i_t.Type() == TileType::Suit; } ), "Sequence group has a non-suit tile" );
		Ensure( std::ranges::adjacent_find( m_tiles, std::not_equal_to{}, []( Tile const& i_t ) { return i_t.Get<TileType::Suit>().m_suit; } ) == m_tiles.end(), "Sequence group didn't have matching suits" );
		break;
	}
	case Triplet:
	{
		Ensure( m_tiles.size() == 3, "Triplet group didn't have 3 tiles" );
		Ensure( std::ranges::adjacent_find( m_tiles, std::not_equal_to{} ) == m_tiles.end(), "Triplet group didn't have matching tiles" );
		break;
	}
	case Quad:
	{
		Ensure( m_tiles.size() == 4, "Quad group didn't have 4 tiles" );
		Ensure( std::ranges::adjacent_find( m_tiles, std::not_equal_to{} ) == m_tiles.end(), "Quad group didn't have matching tiles" );
		break;
	}
	}
#endif

	// Do a quick sort, though only need to for sequences
	if ( m_type == GroupType::Sequence )
	{
		std::ranges::sort( m_tiles );
	}
}

//------------------------------------------------------------------------------
HandGroup::HandGroup
(
	HandInterpretation const& i_interp,
	Tile i_winningTile
)
	: HandGroup(
		Utils::Append( i_interp.m_ungrouped, i_winningTile ),
		WaitTypeToGroupType( i_interp.m_waitType ),
		false
	)
{}

//------------------------------------------------------------------------------
TileType HandGroup::TilesType
(
)	const
{
	return m_tiles.front().Type();
}

//------------------------------------------------------------------------------
Suit HandGroup::CommonSuit
(
)	const
{
	Ensure( TilesType() == TileType::Suit, "Cannot call CommonSuit when not a suit group" );
	return m_tiles.front().Get<TileType::Suit>().m_suit;
}

//------------------------------------------------------------------------------
SuitTileValue HandGroup::CommonSuitTileValue
(
)	const
{
	Ensure( TilesType() == TileType::Suit, "Cannot call CommonSuitTileValue when not a suit group" );
	return m_tiles.front().Get<TileType::Suit>().m_value;
}

//------------------------------------------------------------------------------
uint32_t HandInterpretation::Rank
(
)	const
{
	// Bits 0-3 for four triplets
	// Bits 4-7 for four sequences
	// Bit 8 for a pair
	uint32_t rank = 0;
	int tripleCount = 0;
	int sequenceCount = 4;
	for ( HandGroup const& group : m_groups )
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
HandAssessment::HandAssessment
(
	Hand const& i_hand
)
{
	// Make a temporary list of all tiles in the hand that we can use for quickly assessing
	Vector<Tile> tilesInHand = i_hand.FreeTiles();

	// At the same time, make any meld-specific assessments
	for ( Meld const& meld : i_hand.Melds() )
	{
		tilesInHand.insert_range( tilesInHand.end(), std::views::elements<0>( meld.m_tiles ) );

		m_open |= meld.m_open;
	}

	// Now assess individual tiles
	for ( Tile const& tile : tilesInHand )
	{
		m_containsTileType[ tile.Type() ] = true;

		if ( tile.Type() == TileType::Suit )
		{
			SuitTile const& suitTile = tile.Get<TileType::Suit>();
			m_containsSuit[ suitTile.m_suit ] = true;
			if ( suitTile.m_value == 1 || suitTile.m_value == 9 )
			{
				m_containsTerminals = true;
			}
			break;
		}
	}

	m_containsHonours = m_containsTileType[ TileType::Dragon ] || m_containsTileType[ TileType::Wind ];

	// Finally, make possible hand interpretations
	// Start by setting up the fixed part determined by the melds
	HandInterpretation fixedPart;
	for ( Meld const& meld : i_hand.Melds() )
	{
		fixedPart.m_groups.emplace_back(
			std::ranges::to<Vector<Tile>>( std::views::elements<0>( meld.m_tiles ) ),
			meld.m_type,
			meld.m_open
		);
	}

	// Aim: calculate all possible arrangements that could form (part of) a winning hand, excluding special arrangements (13 orphans + 7 pairs)
	// Need to do this without repeating any arrangements, nor any permutations of arrangements
	// e.g. if there's 3445, then 345 + 4 and 44 + 35 should be the only two arrangements
	// or if there's 123456, then 123 456, 234 + 156, 345 + 126, should be the only three arrangements.
	// This is quite tricky, as evidently, some tiles will be part of multiple groups so simply recursively enumerating every tile will generate a lot of repeats.
	// Option 1 is the simple bruteforce: generate all arrangements, excluding those with fewer groups than available, then eliminate duplicates
	// Option 2:
	// Recursively enumerate, with the rules:
	/// Arrangements selected greedily i.e. if a group is present, it is always included, and only excluded if that would allow another group to form
	/// sequences are only generated numerically upwards from the given tile
	/// pairs/triplets are only generated to the right from the given tile
	/// duplicates of 'earlier' tiles are skipped if the earlier tile was marked as ungrouped
	// This should generate identical arrangement sets to option 1
	// Proof:
	// Suppose there's an arrangement A that is generated by 1 but not by 2.
	// This means it must contain a group G that is not present in any arrangement generated by 2
	// Such a group would have been generated in contradiction to one of the rules of 2:
	/// if it's a sequence that's below an assessed tile, this couldn't happen because the lower tile would've been assessed at some point
	/// if it's a pair/triplet that's to the left of a given tile, again can't happen because the left tile would've been assessed first
	/// if it's because it's including a tile skipped due to being a duplicate, this can't be right, as such a tile would only be skipped if already marked ungrouped
	// FWIW most hands will collapse to just a couple of interpretations pretty quick
	// and there is a small upper limit to the number of possible arrangements as they are always as greedy as possible:
	// about 4 or so
	// The main tricky part of this is working out which arrangements are 'degenerate' i.e. strictly weaker than another arrangement
	// the principle is that if ungrouping a tile allows another group to be made, then that arrangement is 'good'
	// but this isn't global. A hand with a 4 sequence arrangement and a 3 triplet arrangement has 2 good arrangements
	// Okay I think I just need to make the baseline bruteforce to test against

	m_interpretations = GenerateInterpretations( fixedPart, i_hand.FreeTiles() );
}


static void CalculateWait
(
	HandInterpretation& io_soFar
)
{
	// Go by order of simplicity

	// Check for tanki first
	if ( io_soFar.m_ungrouped.size() == 1 )
	{
		io_soFar.m_waitType = WaitType::Tanki;
		io_soFar.m_waits.insert( io_soFar.m_ungrouped.front() );
		return;
	}

	Ensure( io_soFar.m_ungrouped.size() == 2, "Wasn't a tanki wait but didn't have 2 tiles remaining" );

	// Check for shanpon second
	if ( io_soFar.m_ungrouped.front() == io_soFar.m_ungrouped.back() )
	{
		io_soFar.m_waitType = WaitType::Shanpon;
		io_soFar.m_waits.insert( io_soFar.m_ungrouped.front() );
		return;
	}

	// If not shanpon then work out the type of sequence wait, if there is one
	if ( io_soFar.m_ungrouped.front().Type() != TileType::Suit || io_soFar.m_ungrouped.back().Type() != TileType::Suit )
	{
		// no wait :(
		return;
	}

	SuitTile const& tile1 = io_soFar.m_ungrouped.front().Get<TileType::Suit>();
	SuitTile const& tile2 = io_soFar.m_ungrouped.back().Get<TileType::Suit>();

	if ( tile1.m_suit != tile2.m_suit )
	{
		// no wait :(
		return;
	}

	// Tiles are sorted so this should always be a valid thing to ask - tile2 is either 1 or 2 above tile1
	if ( tile2.m_value == tile1.m_value + SuitTileValue::Set<1>() )
	{
		if ( tile1.m_value == SuitTileValue::Min )
		{
			// Bottom edge
			io_soFar.m_waitType = WaitType::Penchan;
			io_soFar.m_waits.insert( SuitTile{ tile2.m_suit, tile2.m_value + SuitTileValue::Set<1>() } );
			return;
		}
		else if ( tile2.m_value == SuitTileValue::Max )
		{
			// Top edge
			io_soFar.m_waitType = WaitType::Penchan;
			io_soFar.m_waits.insert( SuitTile{ tile1.m_suit, tile1.m_value - SuitTileValue::Set<1>() } );
			return;
		}

		// Open
		io_soFar.m_waitType = WaitType::Ryanmen;
		io_soFar.m_waits.insert( SuitTile{ tile1.m_suit, tile1.m_value - SuitTileValue::Set<1>() } );
		io_soFar.m_waits.insert( SuitTile{ tile2.m_suit, tile2.m_value + SuitTileValue::Set<1>() } );
		return;
	}
	else if ( tile2.m_value == tile1.m_value + SuitTileValue::Set<2>() )
	{
		// Middle wait
		io_soFar.m_waitType = WaitType::Kanchan;
		io_soFar.m_waits.insert( SuitTile{ tile1.m_suit, tile1.m_value + SuitTileValue::Set<1>() } );
	}

	// Nothing :(
}

static void Gen
(
	Vector<HandInterpretation>& io_interps,
	HandInterpretation i_soFar,
	Vector<Tile> i_sortedRemaining,
	size_t i_nextTileI
)
{
	/*
		Vector<HandGroup> m_groups;
		Vector<Tile> m_ungrouped;
		Set<Tile> m_waits;
		WaitType m_waitType;
	*/

	// Make groups from current tile, when we've run out, work out the wait from the remaining tiles
	if ( i_sortedRemaining.size() < 3 )
	{
		i_soFar.m_ungrouped = i_sortedRemaining;
		if ( i_soFar.m_groups.size() >= 4 )
		{
			CalculateWait( i_soFar );
		}

		uint32_t const newRank = i_soFar.Rank();
		for ( auto interpI = io_interps.begin(); interpI != io_interps.end(); /*++interpI*/ )
		{
			uint32_t const existingRank = interpI->Rank();
			if ( existingRank != newRank )
			{
				if ( ( newRank & existingRank ) == newRank )
				{
					// Degen
					return;
				}
				else if ( ( newRank & existingRank ) == existingRank )
				{
					// New interp supersedes it
					interpI = io_interps.erase( interpI );
					continue;
				}
			}
			++interpI;
		}
		io_interps.push_back( i_soFar );
		return;
	}

	bool const needPair = !std::ranges::any_of( i_soFar.m_groups, []( HandGroup const& group ) { return group.Type() == GroupType::Pair; } );

	bool madeGroups = false;
	// TODO-MVP: don't bother with optimisations for now, just repeat
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
			Gen( io_interps, withPair, remTiles, tileI );
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
			Gen( io_interps, withTriplet, remTiles, tileI );
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
									Gen( io_interps, withSeq, remTiles, tileI );
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
		uint32_t const newRank = i_soFar.Rank();
		for ( auto interpI = io_interps.begin(); interpI != io_interps.end(); /*++interpI*/ )
		{
			uint32_t const existingRank = interpI->Rank();
			if ( existingRank != newRank )
			{
				if ( ( newRank & existingRank ) == newRank )
				{
					// Degen
					return;
				}
				else if ( ( newRank & existingRank ) == existingRank )
				{
					// New interp supersedes it
					interpI = io_interps.erase( interpI );
					continue;
				}
			}
			++interpI;
		}
		io_interps.push_back( i_soFar );
	}
}

//------------------------------------------------------------------------------
/*static*/ Vector<HandInterpretation> HandAssessment::GenerateInterpretations
(
	HandInterpretation const& i_fixedPart,
	Vector<Tile> const& i_freeTiles
)
{
	Vector<HandInterpretation> interpretations;

	Vector<Tile> sortedFreeTiles = i_freeTiles;
	std::ranges::sort( sortedFreeTiles );
	Gen( interpretations, i_fixedPart, sortedFreeTiles, 0 );

	// TODO-AI: Likely want this to be optional, so that AI can assess paths that don't lead to immediate victory
	/*if ( std::ranges::any_of(interpretations, [](HandInterpretation const& i_interp) { return !i_interp.m_waits.empty(); }) )
	{
		std::erase_if( interpretations, []( HandInterpretation const& i_interp ) { return i_interp.m_waits.empty(); } );
	}*/

	return interpretations;
}

}