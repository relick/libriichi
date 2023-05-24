#include "Tile.hpp"

namespace Riichi
{

//------------------------------------------------------------------------------
// Include specialness of the tile
//------------------------------------------------------------------------------
bool StrictEqualTo
(
	SuitTile const& i_a,
	SuitTile const& i_b
)
{
	return i_a.m_suit == i_b.m_suit && i_a.m_value == i_b.m_value;
}

//------------------------------------------------------------------------------
bool StrictEqualTo
(
	Tile const& i_a,
	Tile const& i_b
)
{
	if ( i_a.Type() != i_b.Type() )
	{
		return false;
	}

	switch ( i_a.Type() )
	{
		using enum TileType;
	case Suit: return StrictEqualTo( i_a.Get<Suit>(), i_b.Get<Suit>() );
	case Dragon: return i_a.Get<Dragon>() == i_b.Get<Dragon>();
	case Wind: return i_a.Get<Wind>() == i_b.Get<Wind>();
	}

	return false;
}

//------------------------------------------------------------------------------
std::ostream& operator<<( std::ostream& io_out, Tile const& i_tile )
{
	switch ( i_tile.Type() )
	{
		using enum TileType;
	case Suit:
	{
		SuitTile const& tile = i_tile.Get<Suit>();
		switch ( tile.m_suit )
		{
			using enum Suit;
		case Manzu: io_out << static_cast< int >( tile.m_value.m_val ) << "m"; return io_out;
		case Pinzu: io_out << static_cast< int >( tile.m_value.m_val ) << "p"; return io_out;
		case Souzu: io_out << static_cast< int >( tile.m_value.m_val ) << "s"; return io_out;
		}
		break;
	}
	case Dragon:
	{
		DragonTileType const& tile = i_tile.Get<Dragon>();
		switch ( tile )
		{
			using enum DragonTileType;
		case White: io_out << "白"; return io_out;
		case Green: io_out << "発"; return io_out;
		case Red: io_out << "中"; return io_out;
		}
		break;
	}
	case Wind:
	{
		WindTileType const& tile = i_tile.Get<Wind>();
		switch ( tile )
		{
			using enum WindTileType;
		case East: io_out << "東"; return io_out;
		case South: io_out << "南"; return io_out;
		case West: io_out << "西"; return io_out;
		case North: io_out << "北"; return io_out;
		}
		break;
	}
	}
	return io_out;
}

}