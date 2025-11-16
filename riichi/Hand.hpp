#pragma once

#include "Containers.hpp"
#include "Declare.hpp"
#include "Seat.hpp"
#include "Tile.hpp"
#include "Utils.hpp"

#include <algorithm>

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
};
//------------------------------------------------------------------------------
inline bool ConsiderLikeTriplet( GroupType i_type ) { return i_type >= GroupType::Triplet; }

template<TileInstanceRange TileInstances = DefaultTileInstanceRange>
inline auto MaybeAppendTileInstance( TileInstances&& i_baseRange, Option<TileInstance> i_tile )
{
	return Utils::ConcatRanges<TileInstance>( std::forward<TileInstances>( i_baseRange ), Utils::MaybeSingleView{ i_tile } );
}

//------------------------------------------------------------------------------
// Meld is made up of tiles that may have come from a different seat to the owner of the meld
// A meld can also be closed (e.g. closed kan), in which it is locked in but doesn't affect open-ness of the hand
//------------------------------------------------------------------------------
class Meld
{
public:
	struct CalledTile
	{
		TileInstance m_tile;
		Seat m_from;
	};

	template<TileInstanceRange TileInstances = DefaultTileInstanceRange>
	static Meld MakeSequence( CalledTile i_calledTile, TileInstances&& i_tilesFromHand )
	{
		riEnsure( i_tilesFromHand.size() == 2, "2 hand tiles required for sequence" );
		return { GroupType::Sequence, i_calledTile, std::ranges::to<Vector<TileInstance>>( i_tilesFromHand ) };
	}
	template<TileInstanceRange TileInstances = DefaultTileInstanceRange>
	static Meld MakeTriplet( CalledTile i_calledTile, TileInstances&& i_tilesFromHand )
	{
		riEnsure( i_tilesFromHand.size() == 2, "2 hand tiles required for triplet" );
		return { GroupType::Triplet, i_calledTile, std::ranges::to<Vector<TileInstance>>( i_tilesFromHand ) };
	}
	template<TileInstanceRange TileInstances = DefaultTileInstanceRange>
	static Meld MakeClosedQuad( TileInstances&& i_tilesFromHand )
	{
		riEnsure( i_tilesFromHand.size() == 4, "4 hand tiles required for closed quad" );
		return { GroupType::Quad, std::nullopt, std::ranges::to<Vector<TileInstance>>( i_tilesFromHand ) };
	}
	template<TileInstanceRange TileInstances = DefaultTileInstanceRange>
	static Meld MakeOpenQuad( CalledTile i_calledTile, TileInstances&& i_tilesFromHand )
	{
		riEnsure( i_tilesFromHand.size() == 3, "3 hand tiles required for open quad" );
		return { GroupType::Quad, i_calledTile, std::ranges::to<Vector<TileInstance>>( i_tilesFromHand ) };
	}
	Meld& UpgradeTripletToQuad( TileInstance i_tileFromHand )
	{
		riEnsure( Triplet() && Open(), "Can only upgrade triplets to quads" );
		m_tilesFromHand.push_back( std::move( i_tileFromHand ) );
		return *this;
	}

	bool Sequence() const { return m_type == GroupType::Sequence; }
	bool Triplet() const { return m_type == GroupType::Triplet && !UpgradedQuad(); }
	bool Quad() const { return m_type == GroupType::Quad || UpgradedQuad(); }
	bool UpgradedQuad() const { return m_type == GroupType::Triplet && m_tilesFromHand.size() == 3; }

	bool Open() const { return m_calledTile.has_value(); }
	TileInstance GetCalledTile() const { riEnsure( Open(), "Cannot get called tile data for closed meld" ); return m_calledTile->m_tile; }
	Seat CalledTileFrom() const { riEnsure( Open(), "Cannot get called tile data for closed meld" ); return m_calledTile->m_from; }
	TileKind SharedTileKind() const { riEnsure( !Sequence(), "Cannot get shared kind of sequence meld" ); return m_tilesFromHand.front().Tile().Kind(); }
	auto Tiles() const
	{
		return Open()
			? MaybeAppendTileInstance( m_tilesFromHand, GetCalledTile() )
			: MaybeAppendTileInstance( m_tilesFromHand, std::nullopt )
			;
	}
	TileInstance UpgradedQuadTile() const { riEnsure( UpgradedQuad(), "Cannot get upgraded quad tile if not upgraded quad" ); return m_tilesFromHand.back(); }

private:
	Meld( GroupType i_type, Option<CalledTile> i_calledTile, Vector<TileInstance> i_tilesFromHand )
		: m_type{i_type}
		, m_calledTile{ std::move( i_calledTile ) }
		, m_tilesFromHand{ std::move( i_tilesFromHand ) }
	{}

	friend struct HandAssessment;
	GroupType AssessmentType() const
	{
		// Only upgraded quads are strange and have a differing m_type from what needs to be returned here
		if ( Quad() ) { return GroupType::Quad; }
		return m_type;
	}

	GroupType m_type{ GroupType::Sequence };
	Option<CalledTile> m_calledTile;
	Vector<TileInstance> m_tilesFromHand;
};

//------------------------------------------------------------------------------
// An option for any kind of call, i.e. chi/pon/kan on others, and kan from within a hand on the player's turn
//------------------------------------------------------------------------------
template<typename T_Tag>
struct CallOption
{
	TileKind m_callTileKind;
	bool m_closed;
	Option<TileInstance> m_drawnTileInvolved;
	Vector<TileInstance> m_freeHandTilesInvolved;

	friend bool operator==( CallOption const& i_a, CallOption const& i_b )
	{
		return i_a.m_callTileKind == i_b.m_callTileKind
			&& i_a.m_closed == i_b.m_closed;
	}

	auto Tiles() const { return MaybeAppendTileInstance( m_freeHandTilesInvolved, m_drawnTileInvolved ); }
};

using ChiOption = CallOption<struct ChiOptionTag>;
using PonOption = CallOption<struct PonOptionTag>;
using KanOption = CallOption<struct KanOptionTag>;
using HandKanOption = CallOption<struct HandKanOptionTag>;

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
	template<TileInstanceRange TileInstances = DefaultTileInstanceRange>
	inline void AddFreeTiles( TileInstances&& i_newTiles );
	void Discard( TileInstance const& i_toDiscard, Option<TileDraw> const& i_drawToAdd );
	template<std::same_as<TileInstance>... TileInstances>
	inline Meld const& CallMeld( TileInstance i_calledTile, Seat i_calledFrom, TileInstances... i_freeHandTiles );
	inline Meld const& CallMeldFromHand( HandKanOption const& i_kanOption );

	// These questions only consider the hand's tiles against the given TileKind and not the actual validity of the call in the round
	Vector<ChiOption> ChiOptions( TileKind i_tile ) const;
	Vector<PonOption> PonOptions( TileKind i_tile ) const;
	Vector<KanOption> KanOptions( TileKind i_tile ) const;

	// Will return all free tiles (or the drawn tile) that are involvable in a kan this turn,
	// even though that will mean up to 4 entries - it simplifies displaying the options to the player this way.
	Vector<HandKanOption> HandKanOptions( Option<TileInstance> const& i_drawnTile ) const;

	// Goes through all melded tiles, then all free tiles
	inline auto AllTiles() const;

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

	// Projections
	static Tile const& First( HandGroup const& i_group ) { return i_group[ 0 ]; }
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
	bool HasNumbersOfAllSuits() const { return std::ranges::all_of( std::views::zip( m_containsSuitSimples, m_containsSuitTerminals ), []( auto const& p ) { return std::get<0>( p ) || std::get<1>( p ); }); }

	bool HasAnyDragons() const { return m_containsDragons; }
	bool HasAnyWinds() const { return m_containsWinds; }
	bool HasAnyHonours() const { return HasAnyDragons() || HasAnyWinds(); }

	bool IsOpen() const { return m_open; }

	explicit HandAssessment( Hand const& i_hand, Rules const& i_rules );

	Vector<HandInterpretation> const& Interpretations() const { return m_interpretations; }
	Set<TileKind> const& Waits() const { return m_overallWaits; }

private:
	Vector<HandInterpretation> m_interpretations;
	Set<TileKind> m_overallWaits;
};

}

#include "Hand.inl"