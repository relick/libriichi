#pragma once

#include "Hand.hpp"

#include <ranges>

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
		for ( Tile const& tile : std::views::elements<0>( meld.m_tiles ) )
		{
			i_visitor( tile );
		}
	}

	for ( Tile const& tile : m_freeTiles )
	{
		i_visitor( tile );
	}
}

}