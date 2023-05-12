#pragma once

#include "seat.hpp"
#include "tile.hpp"
#include "utils.hpp"

#include <unordered_set>
#include <vector>

namespace Riichi
{

enum class GroupType : uint8_t
{
	Pair,
	Sequence,
	Triplet,
	Quad, // Kan only - we never assess a quad inside a hand interpretation (other than to offer the option to the player)
};

// Meld is made up of tiles that may have come from a different seat to the owner of the meld
// A meld can also be closed (e.g. closed kan), in which it is locked in but doesn't affect open-ness of the hand
struct Meld
{
	using MeldTile = std::pair<Tile, Seat>;

	std::vector<MeldTile> m_tiles;
	GroupType m_type{ GroupType::Sequence };
	bool m_open{ true };
};

// A hand is made up of free tiles, and a set of melds
// Melds are ordered in the order they occurred
class Hand
{
	std::vector<Tile> m_freeTiles;
	std::vector<Meld> m_melds;
public:

	std::vector<Tile> const& FreeTiles() const { return m_freeTiles; }
	std::vector<Meld> const& Melds() const { return m_melds; }
};

enum class WaitType
{
	Tanki, // Pair
	Kanchan, // Middle/closed
	Penchan, // Edge
	Ryanmen, // Open
	Shanpon, // Triplet
};

struct HandInterpretation
{
	struct Group
	{
		std::vector<Tile> m_tiles;
		GroupType m_type{ GroupType::Sequence };
		bool m_open{ true };
	};

	std::vector<Group> m_groups;
	std::vector<Tile> m_ungrouped;
	std::unordered_set<Tile> m_waits;
	WaitType m_waitType;
};

// Useful pre-calculations that saves every yaku checking for the same simple things
struct HandAssessment
{
	std::array<bool, c_tileTypeCount> m_containsTileType{};
	bool ContainsTileType( TileType i_type ) const { return m_containsTileType[ ( size_t )i_type ]; }
	std::array<bool, c_suitCount> m_containsSuit{};
	bool ContainsSuit( Suit i_suit ) const { return m_containsSuit[ ( size_t )i_suit ]; }
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

#include "hand.ipp"
