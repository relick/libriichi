#include "Tile.hpp"

namespace Riichi
{

//------------------------------------------------------------------------------
std::ostream& operator<<( std::ostream& io_out, Tile const& i_tile )
{
	switch ( i_tile.Face() )
	{
	case Face::One:
	case Face::Two:
	case Face::Three:
	case Face::Four:
	case Face::Five:
	case Face::Six:
	case Face::Seven:
	case Face::Eight:
	case Face::Nine:
	{
		io_out << static_cast< int >( i_tile.Face() );
		break;
	}

	case Face::Haku: io_out << "白"; break;
	case Face::Hatsu: io_out << "発"; break;
	case Face::Chun: io_out << "中"; break;

	case Face::East: io_out << "東"; break;
	case Face::South: io_out << "南"; break;
	case Face::West: io_out << "西"; break;
	case Face::North: io_out << "北"; break;
	}

	if ( i_tile.HasProperty<Akadora>() )
	{
		io_out << "*";
	}

	switch ( i_tile.Suit() )
	{
	case Suit::Manzu: io_out << "m"; break;
	case Suit::Pinzu: io_out << "p"; break;
	case Suit::Souzu: io_out << "s"; break;

	case Suit::None: break;
	}

	return io_out;
}

}