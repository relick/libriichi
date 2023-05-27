#pragma once

#include "Yaku_Standard.hpp"

#include "Hand.hpp"
#include "RoundData.hpp"
#include "Tile.hpp"

#include <numeric>

namespace Riichi::StandardYaku
{

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

	if ( i_interp.m_waitType == WaitType::Shanpon && ValidTile( i_nextTile ) )
	{
		return 1;
	}

	return NoYaku;
}

template<NameString t_YakuhaiName, DragonTileType t_DragonType>
/*static*/ bool DragonYakuhai<t_YakuhaiName, t_DragonType>::ValidTile( Tile const& i_tile )
{
	return i_tile.Type() == TileType::Dragon && i_tile.Get<TileType::Dragon>() == t_DragonType;
}

}
