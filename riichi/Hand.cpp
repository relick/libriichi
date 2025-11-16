#include "Hand.hpp"

#include "HandInterpreter.hpp"
#include "Rules.hpp"
#include "Utils.hpp"

#include <algorithm>

namespace Riichi
{

//------------------------------------------------------------------------------
void Hand::Discard
(
	TileInstance const& i_toDiscard,
	Option<TileDraw> const& i_drawToAdd
)
{
	bool const success = Utils::EraseOneIf( m_freeTiles, EqualsTileInstanceID{ i_toDiscard } );
	riEnsure( success, "Failed to discard tile - invalid?" );

	if ( i_drawToAdd.has_value() )
	{
		m_freeTiles.push_back( i_drawToAdd.value().m_tile );
		std::sort( m_freeTiles.begin(), m_freeTiles.end(), CompareTileKindOp{} ); // TODO-QOL: players may not always want their hand sorted
	}
}

//------------------------------------------------------------------------------
Vector<ChiOption> Hand::ChiOptions
(
	TileKind i_tile
)	const
{
	if ( i_tile.IsHonour() )
	{
		// Cannot chi if not suit tile
		return {};
	}

	// It's a little complicated searching for chi options
	// We have to check for 3 shapes: DHH HDH HHD where D is the discarded tile and H are hand tiles
	// We also want to multiply chi options based on differing Tile values (NOT just TileKinds!)
	// It works quite well then to reuse a pair of sets and search for the tiles we need in each shape, then fill out options from that as a cartesian product

	Vector<ChiOption> options;
	Set<TileInstance, EqualsTileInstanceIDOp> tiles1;
	Set<TileInstance, EqualsTileInstanceIDOp> tiles2;

	auto fnSearchForTiles = [ & ]( TileKind const& i_search1, TileKind const& i_search2 )
	{
		EqualsTileKind const sharesSearch1Kind{ i_search1 };
		EqualsTileKind const sharesSearch2Kind{ i_search2 };
		for ( TileInstance const& tile : m_freeTiles )
		{
			if ( sharesSearch1Kind( tile ) )
			{
				tiles1.insert( tile );
			}
			else if ( sharesSearch2Kind( tile ) )
			{
				tiles2.insert( tile );
			}
		}

		if ( !tiles1.empty() && !tiles2.empty() )
		{
			// Found shape, cartesian product the options
			for ( TileInstance const& tile1 : tiles1 )
			{
				for ( TileInstance const& tile2 : tiles2 )
				{
					options.push_back( { i_tile, false, {}, { tile1, tile2 } } );
				}
			}
		}

		tiles1.clear();
		tiles2.clear();
	};

	if ( i_tile.Face() <= Face::Seven )
	{
		fnSearchForTiles( i_tile.Next(), i_tile.Next().Next() );
	}

	if ( i_tile.Face() >= Face::Two && i_tile.Face() <= Face::Eight )
	{
		fnSearchForTiles( i_tile.Prev(), i_tile.Next() );
	}

	if ( i_tile.Face() >= Face::Three )
	{
		fnSearchForTiles( i_tile.Prev().Prev(), i_tile.Prev() );
	}

	return options;
}

//------------------------------------------------------------------------------
Vector<PonOption> Hand::PonOptions
(
	TileKind i_tile
)	const
{
	EqualsTileKind const sharesTileKind{ i_tile };
	Vector<TileInstance> usableTiles = std::ranges::to<Vector<TileInstance>>(
		m_freeTiles
		| std::views::filter( [ & ]( TileInstance const& tile ) { return sharesTileKind( tile ); } )
	);

	// Need to have at least 2 tiles matching the given kind.
	Vector<PonOption> options;
	if ( usableTiles.size() >= 2 )
	{
		for ( size_t i = 0; i < usableTiles.size(); ++i )
		{
			for ( size_t j = i + 1; j < usableTiles.size(); ++j )
			{
				options.push_back( { i_tile, false, {}, { usableTiles[ i ], usableTiles[ j ] } } );
			}
		}
	}

	return options;
}

//------------------------------------------------------------------------------
Vector<KanOption> Hand::KanOptions
(
	TileKind i_tile
)	const
{
	EqualsTileKind const sharesTileKind{ i_tile };
	Vector<TileInstance> usableTiles = std::ranges::to<Vector<TileInstance>>(
		m_freeTiles
		| std::views::filter( [ & ]( TileInstance const& tile ) { return sharesTileKind( tile ); } )
	);

	// Need to have at least 3 tiles matching the given kind.
	Vector<KanOption> options;
	if ( usableTiles.size() >= 3 )
	{
		for ( size_t i = 0; i < usableTiles.size(); ++i )
		{
			for ( size_t j = i + 1; j < usableTiles.size(); ++j )
			{
				for ( size_t k = j + 1; k < usableTiles.size(); ++k )
				{
					options.push_back( { i_tile, false, {}, { usableTiles[ i ], usableTiles[ j ], usableTiles[ k ] } } );
				}
			}
		}
	}

	return options;
}

//------------------------------------------------------------------------------
Vector<HandKanOption> Hand::HandKanOptions
(
	Option<TileInstance> const& i_drawnTile
)	const
{
	Vector<HandKanOption> results;

	bool constexpr c_closedKan = true;
	bool constexpr c_isDrawnTile = true;

	if ( i_drawnTile )
	{
		EqualsTileKind const sharesDrawnTileKind{ *i_drawnTile };

		// Find possible closed kan with the drawn tile
		{
			HandKanOption closedKan{ sharesDrawnTileKind.kind, c_closedKan, i_drawnTile };
			for ( TileInstance const& freeTile : m_freeTiles )
			{
				if ( sharesDrawnTileKind( freeTile ) )
				{
					closedKan.m_freeHandTilesInvolved.push_back( freeTile );
				}
			}
			if ( closedKan.m_freeHandTilesInvolved.size() == 3 )
			{
				results.push_back( std::move( closedKan ) );
			}
		}

		// Find possible upgraded kan with the drawn tile
		for ( Meld const& meld : m_melds )
		{
			if ( meld.Triplet() && sharesDrawnTileKind( meld.SharedTileKind() ) )
			{
				results.push_back( { sharesDrawnTileKind.kind, !c_closedKan, i_drawnTile } );
				break;
			}
		}
	}

	// Could have more than one set of 4 in hand, so just search for them left to right
	// n^2 but code is simple and n is small
	for ( auto tileI = m_freeTiles.begin(); tileI != m_freeTiles.end(); ++tileI )
	{
		EqualsTileKind const sharesFreeTileKind{ *tileI };

		// Find possible closed kan with this tile
		{
			HandKanOption closedKan{ sharesFreeTileKind.kind, c_closedKan, std::nullopt };
			for ( auto otherTileI = tileI; otherTileI != m_freeTiles.end(); ++otherTileI )
			{
				if ( sharesFreeTileKind( *otherTileI ) )
				{
					closedKan.m_freeHandTilesInvolved.push_back( *otherTileI );
				}
			}
			if ( closedKan.m_freeHandTilesInvolved.size() == 4 )
			{
				results.push_back( std::move( closedKan ) );
			}
		}

		// Find possible upgraded kan with this tile
		for ( Meld const& meld : m_melds )
		{
			if ( meld.Triplet() && sharesFreeTileKind( meld.SharedTileKind() ) )
			{
				results.push_back( { sharesFreeTileKind.kind, !c_closedKan, std::nullopt, { *tileI } } );
				break;
			}
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

	auto fnPrintTiles = [ & ]( auto&& i_tiles )
	{
		Option<Suit> lastSuit;
		for ( TileInstance const& tileInstance : i_tiles )
		{
			Tile const& tile = tileInstance.Tile();
			if ( tile.IsNumber() )
			{
				if ( lastSuit.has_value() && lastSuit.value() != tile.Suit() )
				{
					fnPrintSuit( lastSuit.value() );
				}
				lastSuit = tile.Suit();
				io_out << static_cast< int >( tile.Face() );
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

	Vector<TileInstance> sortedTiles = i_hand.m_freeTiles;
	std::sort( sortedTiles.begin(), sortedTiles.end(), CompareTileKindOp{} );

	fnPrintTiles( sortedTiles );

	for ( Meld const& meld : i_hand.m_melds )
	{
		io_out << ' ';
		fnPrintTiles( meld.Tiles() );
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
		riEnsure( std::ranges::all_of( m_tiles, EqualsTileKind{ m_tiles.front() } ), "Pair group didn't have matching tiles" );
		break;
	}
	case Sequence:
	{
		riEnsure( m_tiles.size() == 3, "Sequence group didn't have 3 tiles" );
		riEnsure( std::ranges::all_of( m_tiles, []( Tile const& i_t ) { return i_t.IsNumber(); } ), "Sequence group has a non-suit tile" );
		riEnsure( std::ranges::all_of( m_tiles, [ & ]( Tile const& i_t ) { return m_tiles.front().Suit() == i_t.Suit(); } ), "Sequence group didn't have matching suits" );
		break;
	}
	case Triplet:
	{
		riEnsure( m_tiles.size() == 3, "Triplet group didn't have 3 tiles" );
		riEnsure( std::ranges::all_of( m_tiles, EqualsTileKind{ m_tiles.front() } ), "Triplet group didn't have matching tiles" );
		break;
	}
	case Quad:
	{
		riEnsure( m_tiles.size() == 4, "Quad group didn't have 4 tiles" );
		riEnsure( std::ranges::all_of( m_tiles, EqualsTileKind{ m_tiles.front() } ), "Quad group didn't have matching tiles" );
		break;
	}
	}
#endif

	// Do a quick sort, though only need to for sequences
	if ( m_type == GroupType::Sequence )
	{
		std::sort( m_tiles.begin(), m_tiles.end(), CompareTileKindOp{} );
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
bool HandGroup::IsNumbers
(
)	const
{
	return ( *this )[ 0 ].IsNumber();
}

//------------------------------------------------------------------------------
bool HandGroup::IsDragons
(
)	const
{
	return ( *this )[ 0 ].IsDragon();
}

//------------------------------------------------------------------------------
bool HandGroup::IsWinds
(
)	const
{
	return ( *this )[ 0 ].IsWind();
}

//------------------------------------------------------------------------------
Suit HandGroup::CommonSuit
(
)	const
{
	riEnsure( IsNumbers(), "Cannot call CommonSuit when not a suit group" );
	return ( *this )[ 0 ].Suit();
}

//------------------------------------------------------------------------------
Face HandGroup::CommonNumber
(
)	const
{
	riEnsure( IsNumbers(), "Cannot call CommonNumber when not a suit group" );
	return ( *this )[ 0 ].Face();
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
		m_open |= meld.Open();
	}

	// Now assess individual tiles
	for (TileInstance const& tile : i_hand.AllTiles() )
	{
		if ( tile.Tile().IsNumber() )
		{
			m_containsSuitSimples[ tile.Tile().Suit() ] |= tile.Tile().IsSimple();
			m_containsSuitTerminals[ tile.Tile().Suit() ] |= tile.Tile().IsTerminal();
		}
		else
		{
			m_containsDragons |= tile.Tile().IsDragon();
			m_containsWinds |= tile.Tile().IsWind();
		}
	}


	// Finally, make possible hand interpretations
	// Start by setting up the fixed part determined by the melds
	HandInterpretation fixedPart;
	for ( Meld const& meld : i_hand.Melds() )
	{
		fixedPart.m_groups.emplace_back(
			std::ranges::to<Vector<Tile>>( std::views::transform( meld.Tiles(), &TileInstance::GetTile ) ),
			meld.AssessmentType(),
			meld.Open()
		);
	}

	// Then sort the remaining free tiles
	Vector<Tile> sortedFreeTiles = std::ranges::to<Vector<Tile>>( std::views::transform( i_hand.FreeTiles(), &TileInstance::GetTile ) );
	std::ranges::sort( sortedFreeTiles, CompareTileKindOp{} );

	// And visit all the interpreters
	for ( HandInterpreter const& interpreter : i_rules.Interpreters() )
	{
		fixedPart.m_interpreter = interpreter.Name();
		interpreter.AddInterpretations( m_interpretations, fixedPart, sortedFreeTiles );
	}

	for ( HandInterpretation const& interpretation : m_interpretations )
	{
		std::ranges::for_each( interpretation.m_waits, [ this ]( TileKind kind ) { m_overallWaits.insert( kind ); } );
	}
}

}