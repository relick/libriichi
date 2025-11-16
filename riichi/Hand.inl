#pragma once

#include "Hand.hpp"

namespace Riichi
{

//------------------------------------------------------------------------------
template<TileInstanceRange TileInstances>
void Hand::AddFreeTiles
(
	TileInstances&& i_newTiles
)
{
	m_freeTiles.insert( m_freeTiles.end(), i_newTiles.begin(), i_newTiles.end() );
	std::ranges::sort( m_freeTiles, CompareTileKindOp{} ); // TODO-QOL: players may not always want their hand sorted
}

//------------------------------------------------------------------------------
template<std::same_as<TileInstance>... TileInstances>
Meld const& Hand::CallMeld
(
	TileInstance i_calledTile,
	Seat i_calledFrom,
	TileInstances... i_freeHandTiles
)
{
	// Determine the type of group from the parameters
	size_t constexpr freeHandTileCount = sizeof...( i_freeHandTiles );
	static_assert( freeHandTileCount >= 2 && freeHandTileCount <= 3, "Invalid number of tiles to make a called meld with" );

	// Remove all the free hand tiles
	[[maybe_unused]] bool const allFreeHandTilesErased = ( Utils::EraseOneIf( m_freeTiles, EqualsTileInstanceID{ i_freeHandTiles } ) && ... );
	riEnsure( allFreeHandTilesErased, "Failed to find expected tiles in hand to form meld with" );

	// Make the new meld
	if constexpr ( freeHandTileCount == 2 )
	{
		// Must be sequence or triplet
		EqualsTileKind const sharesTileKind{ i_calledTile };
		if ( ( sharesTileKind( i_freeHandTiles ) && ... ) )
		{
			// Triplet
			m_melds.push_back( Meld::MakeTriplet( { i_calledTile, i_calledFrom }, { i_freeHandTiles... } ) );
		}
		else
		{
			// Sequence
			m_melds.push_back( Meld::MakeSequence( { i_calledTile, i_calledFrom }, { i_freeHandTiles... } ) );
		}
	}
	else
	{
		// Must be open quad
		EqualsTileKind const sharesTileKind{ i_calledTile };
		riEnsure( ( sharesTileKind( i_freeHandTiles ) && ... ), "All tiles must share kind for quad!" );
		m_melds.push_back( Meld::MakeOpenQuad( { i_calledTile, i_calledFrom }, { i_freeHandTiles... } ) );
	}

	return m_melds.back();
}


//------------------------------------------------------------------------------
Meld const& Hand::CallMeldFromHand
(
	HandKanOption const& i_kanOption
)
{
	// Remove all the free hand tiles
	bool const allFreeHandTilesErased = std::ranges::all_of(
		i_kanOption.m_freeHandTilesInvolved,
		[ this ]( TileInstance const& freeTile ) { return Utils::EraseOneIf( m_freeTiles, EqualsTileInstanceID{ freeTile } ); }
	);
	riEnsure( allFreeHandTilesErased, "Failed to find expected tiles in hand to form meld with" );

	auto kanTiles = i_kanOption.Tiles();
	EqualsTileKind const sharesTileKind{ i_kanOption.m_kanTileKind };
	riEnsure( std::ranges::all_of( kanTiles, sharesTileKind ), "All tiles must share kind for quad!" );

	if ( i_kanOption.m_closed )
	{
		// Closed kan
		riEnsure( kanTiles.size() == 4, "Expected 4 tiles to be specified for a closed quad" );
		
		// Make a new meld
		m_melds.push_back( Meld::MakeClosedQuad( kanTiles ) );
	}
	else
	{
		// Upgraded kan
		riEnsure( kanTiles.size() == 1, "Expected 1 tile to be specified for an upgraded triplet" );

		// Find the existing triplet that matches
		for ( Meld& meld : m_melds )
		{
			if ( meld.Triplet() && sharesTileKind( meld.SharedTileKind() ) )
			{
				meld.UpgradeTripletToQuad( kanTiles.front() );
				return meld;
			}
		}

		riError( "Failed to find existing triplet to upgrade to quad" );
	}

	return m_melds.back();
}

//------------------------------------------------------------------------------
auto Hand::AllTiles
(
) const
{
	auto allMeldTiles =
		m_melds
		| std::views::transform( []( Meld const& meld ) { return meld.Tiles(); } )
		| std::views::join;
	return Utils::ConcatRanges<TileInstance>( std::move( allMeldTiles ), m_freeTiles );
}

}