#pragma once

#include "HandInterpreter.hpp"

namespace Riichi
{

//------------------------------------------------------------------------------
class StandardInterpreter final
	: public HandInterpreter
{
	char const* Name() const { return "Standard"; }

	void AddInterpretations
	(
		Vector<HandInterpretation>& io_interps,
		HandInterpretation const& i_fixedPart,
		Vector<Tile> const& i_sortedFreeTiles
	) const;

private:
	// A special bitmask that allows us to determine if an interpretation is superseded by another
	// i.e. holds the same kinds of groupings but fewer of them
	static uint32_t Rank( HandInterpretation const& i_interp );

	static void SetWait( HandInterpretation& io_interpToSet );

	static void PushInterp
	(
		Vector<HandInterpretation>& io_interps,
		HandInterpretation& io_interpToPush
	);

	static void RecursivelyGenerate
	(
		Vector<HandInterpretation>& io_interps,
		HandInterpretation i_soFar,
		Vector<Tile> i_sortedRemaining,
		size_t i_nextTileI
	);
};

//------------------------------------------------------------------------------
class SevenPairsInterpreter final
	: public HandInterpreter
{
	char const* Name() const { return "SevenPairs"; }

	void AddInterpretations
	(
		Vector<HandInterpretation>& io_interps,
		HandInterpretation const& i_fixedPart,
		Vector<Tile> const& i_sortedFreeTiles
	) const;
};

//------------------------------------------------------------------------------
class ThirteenOrphansInterpreter final
	: public HandInterpreter
{
	char const* Name() const { return "ThirteenOrphans"; }

	void AddInterpretations
	(
		Vector<HandInterpretation>& io_interps,
		HandInterpretation const& i_fixedPart,
		Vector<Tile> const& i_sortedFreeTiles
	) const;
};

}