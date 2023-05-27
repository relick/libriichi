#pragma once

#include "Containers.hpp"
#include "Declare.hpp"
#include "Tile.hpp"

namespace Riichi
{

//------------------------------------------------------------------------------
struct HandInterpreter
{
	virtual ~HandInterpreter() = default;

	virtual char const* Name() const = 0;
	virtual void AddInterpretations
	(
		Vector<HandInterpretation>& io_interps,
		HandInterpretation const& i_fixedPart,
		Vector<Tile> const& i_sortedFreeTiles
	) const = 0;

	Vector<HandInterpretation> GenerateInterpretations
	(
		HandInterpretation const& i_fixedPart,
		Vector<Tile> const& i_sortedFreeTiles
	) const;
};

}