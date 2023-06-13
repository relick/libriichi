#include "Hand.hpp"

#include "HandInterpreter.hpp"
#include "Rules.hpp"
#include "Utils.hpp"

#include <algorithm>
#include "range/v3/action.hpp"
#include "range/v3/algorithm.hpp"
#include "range/v3/view.hpp"

namespace Riichi
{

//------------------------------------------------------------------------------
void Hand::AddFreeTiles
(
	Vector<Tile> const& i_newTiles
)
{
	m_freeTiles.insert( m_freeTiles.end(), i_newTiles.begin(), i_newTiles.end() );
	std::sort( m_freeTiles.begin(), m_freeTiles.end() ); // TODO-QOL: players may not always want their hand sorted
}

//------------------------------------------------------------------------------
void Hand::Discard
(
	Tile const& i_toDiscard,
	Option<TileDraw> const& i_drawToAdd
)
{
	bool const success = Utils::EraseOne( m_freeTiles,
		[ & ]( Tile const& i_tile )
		{
			return i_tile.Is( i_toDiscard );
		}
	);
	riEnsure( success, "Failed to discard tile - invalid?" );

	if ( i_drawToAdd.has_value() )
	{
		m_freeTiles.push_back( i_drawToAdd.value().m_tile );
		std::sort( m_freeTiles.begin(), m_freeTiles.end() ); // TODO-QOL: players may not always want their hand sorted
	}
}

//------------------------------------------------------------------------------
void Hand::MakeMeld
(
	Pair<Seat, Tile> const& i_meldTile,
	Pair<Tile, Tile> const& i_otherTiles,
	GroupType i_meldType
)
{
	riEnsure( i_meldType < GroupType::Quad, "Must call MakeKan instead of MakeMeld for quads" );

	Utils::EraseOne(
		m_freeTiles,
		[ & ]( Tile const& i_tile )
		{
			return i_tile.Is( i_otherTiles.first );
		}
	);

	Utils::EraseOne(
		m_freeTiles,
		[ & ]( Tile const& i_tile )
		{
			return i_tile.Is( i_otherTiles.second );
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
Hand::KanResult Hand::MakeKan
(
	Tile const& i_meldTile,
	bool i_drawnTile,
	Option<Seat> i_calledFrom
)
{
	// First check existing triplet melds
	for ( Meld& meld : m_melds )
	{
		if ( meld.m_type == GroupType::Triplet && meld.m_tiles.front().first == i_meldTile )
		{
			// Meld comes from our own hand
			riEnsure( !i_calledFrom.has_value(), "Cannot call kan on already open meld" );

			meld.m_tiles.push_back( { i_meldTile, std::nullopt } );
			meld.m_type = GroupType::UpgradedQuad;
			return { true, true, };
		}
	}

	// Then check hand

	riEnsure( i_calledFrom.has_value() != i_drawnTile, "Kan tile cannot be drawn tile and called from at the same time" );

	Meld newMeld;
	if ( i_calledFrom.has_value() )
	{
		// Called from another player, add it in now
		newMeld.m_tiles.push_back( { i_meldTile, i_calledFrom } );
	}
	else if ( i_drawnTile )
	{
		// It is our drawn tile, add it in now
		newMeld.m_tiles.push_back( { i_meldTile, std::nullopt } );
	}

	// Find the remaining 3
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

	riEnsure( newMeld.m_tiles.size() == 4, "Did not find 4 tiles for kan" );

	newMeld.m_type = GroupType::Quad;
	newMeld.m_open = i_calledFrom.has_value();

	m_melds.push_back( std::move( newMeld ) );

	return { false, i_calledFrom.has_value(), };
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
	Set<Tile, EquivalentTile> tiles1;
	Set<Tile, EquivalentTile> tiles2;

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
	size_t const othersCount = ranges::count( m_freeTiles, i_tile );
	return othersCount >= 2;
}

//------------------------------------------------------------------------------
bool Hand::CanCallKan
(
	Tile const& i_tile
)	const
{
	size_t const othersCount = ranges::count( m_freeTiles, i_tile );
	return othersCount >= 3;
}

//------------------------------------------------------------------------------
Vector<Hand::DrawKanResult> Hand::DrawKanOptions
(
	Option<Tile> const& i_drawnTile
)	const
{
	Vector<DrawKanResult> results;

	if ( i_drawnTile.has_value() )
	{
		for ( Meld const& meld : m_melds )
		{
			if ( meld.m_type == GroupType::Triplet && meld.m_tiles.front().first == i_drawnTile.value() )
			{
				results.push_back( { i_drawnTile.value(), false } );
				break;
			}
		}

		if ( results.empty() )
		{
			size_t const othersCount = ranges::count( m_freeTiles, i_drawnTile.value() );
			if ( othersCount >= 3 )
			{
				results.push_back( { i_drawnTile.value(), true } );
			}
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
	auto fnPrintSuit = [ &io_out ]( Suit suit )
	{
		switch ( suit )
		{
		case Suit::Manzu: io_out << 'm'; break;
		case Suit::Pinzu: io_out << 'p'; break;
		case Suit::Souzu: io_out << 's'; break;
		}
	};

	auto fnPrintTiles = [ & ]( auto const& i_tiles )
	{
		Option<Suit> lastSuit;
		for ( Tile const& tile : i_tiles )
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
	};

	Vector<Tile> sortedTiles = i_hand.m_freeTiles;
	std::sort( sortedTiles.begin(), sortedTiles.end() );

	fnPrintTiles( sortedTiles );

	for ( Meld const& meld : i_hand.m_melds )
	{
		io_out << ' ';
		fnPrintTiles( ranges::views::keys( meld.m_tiles ) );
	}

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
		riEnsure( m_tiles.size() == 2, "Pair group didn't have 2 tiles" );
		riEnsure( ranges::adjacent_find( m_tiles, std::not_equal_to{} ) == m_tiles.end(), "Pair group didn't have matching tiles" );
		break;
	}
	case Sequence:
	{
		riEnsure( m_tiles.size() == 3, "Sequence group didn't have 3 tiles" );
		riEnsure( ranges::all_of( m_tiles, []( Tile const& i_t ) { return i_t.Type() == TileType::Suit; } ), "Sequence group has a non-suit tile" );
		riEnsure( ranges::adjacent_find( m_tiles, std::not_equal_to{}, []( Tile const& i_t ) { return i_t.Get<TileType::Suit>().m_suit; } ) == m_tiles.end(), "Sequence group didn't have matching suits" );
		break;
	}
	case Triplet:
	{
		riEnsure( m_tiles.size() == 3, "Triplet group didn't have 3 tiles" );
		riEnsure( ranges::adjacent_find( m_tiles, std::not_equal_to{} ) == m_tiles.end(), "Triplet group didn't have matching tiles" );
		break;
	}
	case Quad:
	case UpgradedQuad:
	{
		riEnsure( m_tiles.size() == 4, "Quad group didn't have 4 tiles" );
		riEnsure( ranges::adjacent_find( m_tiles, std::not_equal_to{} ) == m_tiles.end(), "Quad group didn't have matching tiles" );
		break;
	}
	}
#endif

	// Do a quick sort, though only need to for sequences
	if ( m_type == GroupType::Sequence )
	{
		std::sort( m_tiles.begin(), m_tiles.end() );
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
	riEnsure( TilesType() == TileType::Suit, "Cannot call CommonSuit when not a suit group" );
	return m_tiles.front().Get<TileType::Suit>().m_suit;
}

//------------------------------------------------------------------------------
SuitTileValue HandGroup::CommonSuitTileValue
(
)	const
{
	riEnsure( TilesType() == TileType::Suit, "Cannot call CommonSuitTileValue when not a suit group" );
	return m_tiles.front().Get<TileType::Suit>().m_value;
}

//------------------------------------------------------------------------------
HandAssessment::HandAssessment
(
	Hand const& i_hand,
	Rules const& i_rules
)
{
	// Make any meld-specific assessments
	for ( Meld const& meld : i_hand.Melds() )
	{
		m_open |= meld.m_open;
	}

	// Now assess individual tiles
	i_hand.VisitTiles(
		[ this ]( Tile const& tile )
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
			}
		}
	);

	m_containsHonours = m_containsTileType[ TileType::Dragon ] || m_containsTileType[ TileType::Wind ];

	// Finally, make possible hand interpretations
	// Start by setting up the fixed part determined by the melds
	HandInterpretation fixedPart;
	for ( Meld const& meld : i_hand.Melds() )
	{
		fixedPart.m_groups.emplace_back(
			ranges::to<Vector<Tile>>( ranges::views::keys( meld.m_tiles ) ),
			meld.m_type,
			meld.m_open
		);
	}

	// Then sort the remaining free tiles
	Vector<Tile> sortedFreeTiles = i_hand.FreeTiles();
	std::sort( sortedFreeTiles.begin(), sortedFreeTiles.end() );

	// And visit all the interpreters
	i_rules.VisitInterpreters(
		[ this, &fixedPart, &sortedFreeTiles ]( HandInterpreter const& i_interpreter )
		{
			fixedPart.m_interpreter = i_interpreter.Name();
			i_interpreter.AddInterpretations( m_interpretations, fixedPart, sortedFreeTiles );
		}
	);

	for ( HandInterpretation const& interpretation : m_interpretations )
	{
		ranges::actions::insert( m_overallWaits, interpretation.m_waits );
	}
}

}