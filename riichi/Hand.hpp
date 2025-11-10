#pragma once

#include "Containers.hpp"
#include "Declare.hpp"
#include "Seat.hpp"
#include "Tile.hpp"
#include "Utils.hpp"

#include <algorithm>
#include "range/v3/view.hpp"

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
	using MeldTile = Pair<TileInstance, Option<Seat>>;

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
	Vector<TileInstance> m_freeTiles;
	Vector<Meld> m_melds;
public:

	Vector<TileInstance> const& FreeTiles() const { return m_freeTiles; }
	Vector<Meld> const& Melds() const { return m_melds; }
	void AddFreeTiles( Vector<TileInstance> const& i_newTiles );
	void Discard( TileInstance const& i_toDiscard, Option<TileDraw> const& i_drawToAdd );
	void MakeMeld( Pair<Seat, TileInstance> const& i_meldTile, Pair<TileInstance, TileInstance> const& i_otherTiles, GroupType i_meldType );
	struct KanResult
	{
		bool m_upgradedFromPon{ false };
		bool m_open{ false };
	};
	KanResult MakeKan( TileInstance const& i_meldTile, bool i_drawnTile, Option<Seat> i_calledFrom );

	// These questions only consider the hand's tiles and not the actual validity of the call in the round
	Vector<Pair<Tile, Tile>> ChiOptions( TileKind const& i_tile ) const;
	bool CanPon( TileKind const& i_tile ) const;
	bool CanCallKan( TileKind const& i_tile ) const;
	struct DrawKanResult
	{
		TileInstance kanTile;
		bool closed;
	};
	Vector<DrawKanResult> DrawKanOptions( Option<TileInstance> const& i_drawnTile ) const;

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
	bool IsNumbers() const;
	bool IsDragons() const;
	bool IsWinds() const;
	Suit CommonSuit() const;
	Face CommonNumber() const;
};

//------------------------------------------------------------------------------
struct HandInterpretation
{
	char const* m_interpreter;
	Vector<HandGroup> m_groups;
	Vector<Tile> m_ungrouped;
	Set<TileKind> m_waits;
	WaitType m_waitType{ WaitType::None };
};

//------------------------------------------------------------------------------
// Useful pre-calculations that saves every yaku checking for the same simple things
//------------------------------------------------------------------------------
struct HandAssessment
{
	Utils::EnumArray<bool, Suits> m_containsSuitSimples{ false, false, false, };
	Utils::EnumArray<bool, Suits> m_containsSuitTerminals{ false, false, false, };
	bool m_containsDragons{ false };
	bool m_containsWinds{ false };
	bool m_open{ false };

	bool HasAnySimples() const { return std::ranges::any_of( m_containsSuitSimples, std::identity{} ); }
	bool HasAnyTerminals() const { return std::ranges::any_of( m_containsSuitTerminals, std::identity{} ); }
	bool HasAnyNumbers() const { return HasAnySimples() || HasAnyTerminals(); }
	bool HasAnySimplesOfSuit( Suit i_suit ) const { return m_containsSuitSimples[ i_suit ]; }
	bool HasAnyTerminalsOfSuit( Suit i_suit ) const { return m_containsSuitTerminals[ i_suit ]; }
	bool HasAnyNumbersOfSuit( Suit i_suit ) const { return HasAnySimplesOfSuit( i_suit ) || HasAnyTerminalsOfSuit( i_suit ); }
	bool HasSimplesOfAllSuits() const { return std::ranges::all_of( m_containsSuitSimples, std::identity{} ); }
	bool HasTerminalsOfAllSuits() const { return std::ranges::all_of( m_containsSuitTerminals, std::identity{} ); }
	bool HasNumbersOfAllSuits() const { return std::ranges::all_of( ranges::view::zip( m_containsSuitSimples, m_containsSuitTerminals ), []( auto const& p ) { return p.first || p.second; } ); }

	bool HasAnyDragons() const { return m_containsDragons; }
	bool HasAnyWinds() const { return m_containsWinds; }
	bool HasAnyHonours() const { return HasAnyDragons() || HasAnyWinds(); }

	bool IsOpen() const { return m_open; }

	explicit HandAssessment( Hand const& i_hand, Rules const& i_rules );

	Vector<HandInterpretation> const& Interpretations() const { return m_interpretations; }
	Set<Tile> const& Waits() const { return m_overallWaits; }

private:
	Vector<HandInterpretation> m_interpretations;
	Set<Tile> m_overallWaits;
};

}

#include "Hand.inl"