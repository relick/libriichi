#include "HandInterpreter.hpp"

#include "Hand.hpp"

namespace Riichi
{

//------------------------------------------------------------------------------
Vector<HandInterpretation> HandInterpreter::GenerateInterpretations
(
	HandInterpretation const& i_fixedPart,
	Vector<Tile> const& i_sortedFreeTiles
)	const
{
	Vector<HandInterpretation> interps;
	AddInterpretations( interps, i_fixedPart, i_sortedFreeTiles );
	return interps;
}

}