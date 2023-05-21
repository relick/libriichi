#pragma once

#include "Yaku_Standard.hpp"

#include "Hand.hpp"
#include "Player.hpp"
#include "RoundData.hpp"
#include "Tile.hpp"
#include "Utils.hpp"

#include <numeric>

namespace Riichi::StandardYaku
{

//-----------------------------------------------------------------------------
template<YakuNameString t_YakuhaiName, DragonTileType t_DragonType>
HanValue DragonYakuhai<t_YakuhaiName, t_DragonType>::CalculateValue
(
	RoundData const& i_round,
	Seat const& i_playerSeat,
	Player const& i_player,
	Hand const& i_hand,
	HandAssessment const& i_assessment,
	HandInterpretation const& i_interp,
	Tile const& i_nextTile,
	TileDrawType i_nextTileType
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

template<YakuNameString t_YakuhaiName, DragonTileType t_DragonType>
/*static*/ bool DragonYakuhai<t_YakuhaiName, t_DragonType>::ValidTile( Tile const& i_tile )
{
	return i_tile.Type() == TileType::Dragon && i_tile.Get<TileType::Dragon>() == t_DragonType;
}

}
