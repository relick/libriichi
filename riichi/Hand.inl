#pragma once

#include "Hand.hpp"

#include "range/v3/view.hpp"

namespace Riichi
{

//------------------------------------------------------------------------------
template<typename T_Visitor>
void Hand::VisitTiles
(
	T_Visitor&& i_visitor
)	 const
{
	for ( Meld const& meld : m_melds )
	{
		for ( TileInstance const& tile : ranges::views::keys( meld.m_tiles ) )
		{
			i_visitor( tile.Tile() );
		}
	}

	for ( TileInstance const& tile : m_freeTiles )
	{
		i_visitor( tile.Tile() );
	}
}

}