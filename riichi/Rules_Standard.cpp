#include "Rules_Standard.hpp"

#include "HandInterpreter_Standard.hpp"
#include "Yaku_Standard.hpp"

namespace Riichi
{

StandardYonma::StandardYonma
(
)
{
	m_interpreters.emplace_back( std::make_unique<StandardInterpreter>() );

	// Yonma uses all tiles
	// TODO-AI: red dora
	for ( int tileTypeCount = 0; tileTypeCount < 4; ++tileTypeCount )
	{
		for ( Suit suit : Suits{} )
		{
			for ( SuitTileValue val : SuitTileValue::InclusiveRange( SuitTileValue::Min, SuitTileValue::Max ) )
			{
				m_tileSet.emplace_back( SuitTile{ suit, val } );
			}
		}

		for ( DragonTileType dragon : DragonTileTypes{} )
		{
			m_tileSet.emplace_back( dragon );
		}

		for ( WindTileType wind : WindTileTypes{} )
		{
			m_tileSet.emplace_back( wind );
		}
	}

	Ensure( m_tileSet.size() == 4 * (3 * 9 + 3 + 4), "Tileset filled incorrectly" );
}

}