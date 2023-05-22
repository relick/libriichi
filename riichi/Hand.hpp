#pragma once

#include "Declare.hpp"
#include "Seat.hpp"
#include "Tile.hpp"
#include "Utils.hpp"

#include <unordered_set>
#include <vector>

namespace Riichi
{

//------------------------------------------------------------------------------
enum class GroupType : EnumValueType
{
	Pair,
	Sequence,
	Triplet,
	Quad, // Kan only - we never assess a quad inside a hand interpretation (other than to offer the option to the player)
};
//------------------------------------------------------------------------------
inline bool TripletCompatible( GroupType i_type ) { return i_type == GroupType::Triplet || i_type == GroupType::Quad; }

//------------------------------------------------------------------------------
// Meld is made up of tiles that may have come from a different seat to the owner of the meld
// A meld can also be closed (e.g. closed kan), in which it is locked in but doesn't affect open-ness of the hand
//------------------------------------------------------------------------------
struct Meld
{
	using MeldTile = std::pair<Tile, Seat>;

	std::vector<MeldTile> m_tiles;
	GroupType m_type{ GroupType::Sequence };
	bool m_open{ true };
};

//------------------------------------------------------------------------------
// A hand is made up of free tiles, and a set of melds
// Melds are ordered in the order they occurred
//------------------------------------------------------------------------------
class Hand
{
	std::vector<Tile> m_freeTiles;
	std::vector<Meld> m_melds;
public:

	std::vector<Tile> const& FreeTiles() const { return m_freeTiles; }
	std::vector<Meld> const& Melds() const { return m_melds; }
	void AddFreeTiles( std::vector<Tile> const& i_newTiles );
};

//------------------------------------------------------------------------------
enum class WaitType : EnumValueType
{
	Tanki, // Pair
	Kanchan, // Middle/closed
	Penchan, // Edge
	Ryanmen, // Open
	Shanpon, // Triplet
};
//------------------------------------------------------------------------------
inline bool SequenceWait( WaitType i_type ) { return i_type != WaitType::Tanki && i_type == WaitType::Shanpon; }

//------------------------------------------------------------------------------
inline GroupType WaitTypeToGroupType( WaitType i_wait )
{
	if ( i_wait == WaitType::Shanpon ) { return GroupType::Triplet; }
	if ( i_wait == WaitType::Tanki ) { return GroupType::Pair; }
	return GroupType::Sequence;
}

//------------------------------------------------------------------------------
class HandGroup
{
	std::vector<Tile> m_tiles; // Sorted, if a sequence
	GroupType m_type{ GroupType::Sequence };
	bool m_open{ true };

public:
	HandGroup( std::vector<Tile> i_tiles, GroupType i_type, bool i_open );
	HandGroup( HandInterpretation const& i_interp, Tile i_winningTile ); // Make a group from the ungrouped + winning tile
	HandGroup( HandGroup const& ) = default;
	HandGroup( HandGroup&& ) = default;

	std::vector<Tile> const& Tiles() const { return m_tiles; }
	Tile const& operator[]( size_t i ) const { return m_tiles[ i ]; }
	GroupType Type() const { return m_type; }
	bool Open() const { return m_open; }
	TileType TilesType() const;
	Suit CommonSuit() const;
	SuitTileValue CommonSuitTileValue() const;
};

//------------------------------------------------------------------------------
struct HandInterpretation
{
	std::vector<HandGroup> m_groups;
	std::vector<Tile> m_ungrouped;
	std::unordered_set<Tile> m_waits;
	WaitType m_waitType;
};

//------------------------------------------------------------------------------
// Useful pre-calculations that saves every yaku checking for the same simple things
//------------------------------------------------------------------------------
struct HandAssessment
{
	Utils::EnumIndexedArray<bool, TileType, c_tileTypeCount> m_containsTileType{};
	Utils::EnumIndexedArray<bool, Suit, c_suitCount> m_containsSuit{};
	bool m_containsTerminals{ false }; // 1 or 9 of suit tiles
	bool m_containsHonours{ false }; // wind or dragon
	bool m_open{ false };

	explicit HandAssessment( Hand const& i_hand );

private:
	std::vector<HandInterpretation> m_interpretations;
	static std::vector<HandInterpretation> GenerateInterpretations
	(
		HandInterpretation const& i_fixedPart,
		std::vector<Tile> const& i_freeTiles
	);
};

}
