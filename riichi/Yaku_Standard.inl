#pragma once

#include "Yaku_Standard.hpp"

#include "Hand.hpp"
#include "Round.hpp"
#include "Tile.hpp"

#include <numeric>
#include "range/v3/algorithm.hpp"

namespace Riichi::StandardYaku
{

//------------------------------------------------------------------------------
template<bool t_KuitanEnabled>
HanValue Tanyao<t_KuitanEnabled>::CalculateValue
(
	YAKU_CALCULATEVALUE_PARAMS()
)	const
{
	// Kuitan rule required for open tanyao hands
	if constexpr ( !t_KuitanEnabled )
	{
		if ( i_assessment.m_open )
		{
			return NoYaku;
		}
	}

	if ( InvalidTile( i_lastTile.m_tile ) || ranges::any_of( i_interp.m_ungrouped, InvalidTile ) )
	{
		return NoYaku;
	}

	for ( HandGroup const& group : i_interp.m_groups )
	{
		for ( Tile const& tile : group.Tiles() )
		{
			if ( InvalidTile( tile ) )
			{
				return NoYaku;
			}
		}
	}

	return 1;
}

template<bool t_KuitanEnabled>
/*static*/ bool Tanyao<t_KuitanEnabled>::InvalidTile( Tile const& i_tile )
{
	if ( i_tile.Type() != TileType::Suit )
	{
		return true;
	}

	SuitTile const& suitTile = i_tile.template Get<TileType::Suit>();
	if ( suitTile.m_value == 1 || suitTile.m_value == 9 )
	{
		return true;
	}

	return false;
}

//------------------------------------------------------------------------------
template<NameString t_YakuhaiName, DragonTileType t_DragonType>
HanValue DragonYakuhai<t_YakuhaiName, t_DragonType>::CalculateValue
(
	YAKU_CALCULATEVALUE_PARAMS()
)	const
{
	for ( HandGroup const& group : i_interp.m_groups )
	{
		if ( TripletCompatible( group.Type() ) )
		{
			if ( ValidTile( group[ 0 ] ) )
			{
				return 1;
			}
		}
	}

	if ( i_interp.m_waitType == WaitType::Shanpon && ValidTile( i_lastTile.m_tile ) )
	{
		return 1;
	}

	return NoYaku;
}

template<NameString t_YakuhaiName, DragonTileType t_DragonType>
/*static*/ bool DragonYakuhai<t_YakuhaiName, t_DragonType>::ValidTile( Tile const& i_tile )
{
	return i_tile.Type() == TileType::Dragon && i_tile.template Get<TileType::Dragon>() == t_DragonType;
}

}
