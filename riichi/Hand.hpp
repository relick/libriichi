#pragma once

#include "Containers.hpp"
#include "Declare.hpp"
#include "Seat.hpp"
#include "Tile.hpp"
#include "Utils.hpp"

namespace Riichi
{

//------------------------------------------------------------------------------
enum class GroupType : EnumValueType
{
	Pair,
	Sequence,
	Triplet,

	// Kan only:
	// We never assess a quad inside a hand interpretation (other than to offer the option to the player)
	Quad,
	UpgradedQuad, // Pon -> Kan
};
//------------------------------------------------------------------------------
inline bool TripletCompatible( GroupType i_type ) { return i_type == GroupType::Triplet || i_type == GroupType::Quad || i_type == GroupType::UpgradedQuad; }

//------------------------------------------------------------------------------
// Meld is made up of tiles that may have come from a different seat to the owner of the meld
// A meld can also be closed (e.g. closed kan), in which it is locked in but doesn't affect open-ness of the hand
//------------------------------------------------------------------------------
struct Meld
{
	using MeldTile = Pair<Tile, Option<Seat>>;

	Vector<MeldTile> m_tiles;
	GroupType m_type{ GroupType::Sequence };
	bool m_open{ true };
};

//------------------------------------------------------------------------------
// A hand is made up of free tiles, and a set of melds
// Melds are ordered in the order they occurred
//------------------------------------------------------------------------------
class Hand
{
	Vector<Tile> m_freeTiles;
	Vector<Meld> m_melds;
public:

	Vector<Tile> const& FreeTiles() const { return m_freeTiles; }
	Vector<Meld> const& Melds() const { return m_melds; }
	void AddFreeTiles( Vector<Tile> const& i_newTiles );
	void Discard( Tile const& i_toDiscard, Option<TileDraw> const& i_drawToAdd );
	void MakeMeld( Pair<Seat, Tile> const& i_meldTile, Pair<Tile, Tile> const& i_otherTiles, GroupType i_meldType );
	struct KanResult
	{
		bool m_upgradedFromPon{ false };
		bool m_open{ false };
	};
	KanResult MakeKan( Tile const& i_meldTile, bool i_drawnTile, Option<Seat> i_calledFrom );

	// These questions only consider the hand's tiles and not the actual validity of the call in the round
	Vector<Pair<Tile, Tile>> ChiOptions( Tile const& i_tile ) const;
	bool CanPon( Tile const& i_tile ) const;
	bool CanCallKan( Tile const& i_tile ) const;
	struct DrawKanResult
	{
		Tile kanTile;
		bool closed;
	};
	Vector<DrawKanResult> DrawKanOptions( Option<Tile> const& i_drawnTile ) const;

	template<typename T_Visitor>
	void VisitTiles( T_Visitor&& i_visitor ) const;

	friend std::ostream& operator<<( std::ostream& io_out, Hand const& i_hand );
};

//------------------------------------------------------------------------------
enum class WaitType : EnumValueType
{
	None,

	Tanki, // Pair
	Kanchan, // Middle/closed
	Penchan, // Edge
	Ryanmen, // Open
	Shanpon, // Triplet
};
//------------------------------------------------------------------------------
inline bool SequenceWait( WaitType i_type ) { return i_type != WaitType::Tanki && i_type != WaitType::Shanpon; }

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
	Vector<Tile> m_tiles; // Sorted, if a sequence
	GroupType m_type{ GroupType::Sequence };
	bool m_open{ true };

public:
	HandGroup( Vector<Tile> i_tiles, GroupType i_type, bool i_open );
	HandGroup( HandInterpretation const& i_interp, Tile i_winningTile ); // Make a group from the ungrouped + winning tile
	HandGroup( HandGroup const& ) = default;
	HandGroup( HandGroup&& ) = default;

	Vector<Tile> const& Tiles() const { return m_tiles; }
	Tile const& operator[]( size_t i ) const { return m_tiles[ i ]; }
	GroupType Type() const { return m_type; }
	bool Open() const { return m_open; }
	TileType TilesType() const;
	Suit CommonSuit() const;
	Number CommonNumber() const;
};

//------------------------------------------------------------------------------
struct HandInterpretation
{
	char const* m_interpreter;
	Vector<HandGroup> m_groups;
	Vector<Tile> m_ungrouped;
	Set<Tile> m_waits;
	WaitType m_waitType{ WaitType::None };
};

//------------------------------------------------------------------------------
// Useful pre-calculations that saves every yaku checking for the same simple things
//------------------------------------------------------------------------------
struct HandAssessment
{
	Utils::EnumArray<bool, TileTypes> m_containsTileType{};
	Utils::EnumArray<bool, Suits> m_containsSuit{};
	bool m_containsTerminals{ false }; // 1 or 9 of suit tiles
	bool m_containsHonours{ false }; // wind or dragon
	bool m_open{ false };

	explicit HandAssessment( Hand const& i_hand, Rules const& i_rules );

	Vector<HandInterpretation> const& Interpretations() const { return m_interpretations; }
	Set<Tile> const& Waits() const { return m_overallWaits; }

private:
	Vector<HandInterpretation> m_interpretations;
	Set<Tile> m_overallWaits;
};

}

#include "Hand.inl"