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

//------------------------------------------------------------------------------
/*static*/ Vector<HandInterpretation> HandAssessment::GenerateInterpretations
(
	HandInterpretation const& i_fixedPart,
	Vector<Tile> const& i_freeTiles
)
{
	Vector<HandInterpretation> interpretations;

	auto fnGenerate = [ & ]()
	{

	};

	auto fnEliminate = [ & ]()
	{

	};

	fnGenerate();
	fnEliminate();

	return interpretations;
}

}