#pragma once

#include "hand.hpp"
#include "player.hpp"
#include "round.hpp"
#include "tile.hpp"
#include "utils.hpp"

#include <optional>
#include <numeric>

namespace Riichi
{

using Han = uint8_t;

enum NoYakuE
{
	NoYaku,
};
enum YakumanE
{
	Yakuman,
};
class HanValue
{
	static constexpr Han c_yakumanValue = 13;

	std::optional<Han> m_value;

public:
	HanValue( Han i_value )
		: m_value{ i_value }
	{}

	HanValue( NoYakuE )
	{}

	HanValue( YakumanE )
		: m_value{ c_yakumanValue }
	{}

	bool IsValid() const { return m_value.has_value(); }
	bool IsYakuman() const { return IsValid() && m_value.value() == c_yakumanValue; }
	Han Get() const { return m_value.value(); }
};

struct Yaku
{
	virtual ~Yaku() = default;

	virtual char const* Name() const = 0;

	// NB do not need to check whether i_nextTile is in the interp's waits - it will be
	virtual HanValue CalculateValue
	(
		Round const& i_round,
		Seat const& i_playerSeat,
		Player const& i_player,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const = 0;
};

template<size_t N>
struct YakuNameString
{
	constexpr YakuNameString( const char( &i_str )[ N ] )
	{
		std::copy_n( i_str, N, m_str );
	}

	char m_str[ N ];
};

// Single name Yaku should implement CalculateValue
template<YakuNameString t_YakuName>
struct NamedYaku
	: public Yaku
{
	char const* Name() const final { return t_YakuName.m_str; }
};

///////////////////
// Riichi hands
///////////////////
namespace MainYaku
{

struct MenzenchinTsumohou
	: public NamedYaku<"MenzenchinTsumohou">
{
	// Win by self-draw with closed hand

	HanValue CalculateValue
	(
		Round const& i_round,
		Seat const& i_playerSeat,
		Player const& i_player,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const final
	{
		if ( !i_assessment.m_open && i_nextTileType != TileDrawType::DiscardDraw )
		{
			return 1;
		}

		return NoYaku;
	}
};

// TODO
struct Riichi{};

// TODO
struct Ippatsu{};

struct Pinfu
	: public NamedYaku<"Pinfu">
{
	// Win on a ryanmen wait with a closed hand and no fu

	HanValue CalculateValue
	(
		Round const& i_round,
		Seat const& i_playerSeat,
		Player const& i_player,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const final
	{
		if ( i_assessment.m_open || i_assessment.m_containsDragons )
		{
			// Invalid hand for pinfu
			return NoYaku;
		}

		if ( i_interp.m_waitType != WaitType::Ryanmen )
		{
			// Didn't win on open wait
			return NoYaku;
		}

		bool invalidGroups = false;
		for ( HandInterpretation::Group const& group : i_interp.m_groups )
		{
			switch ( group.m_type )
			{
			using enum GroupType;
			case Triplet:
			case Quad:
			{
				invalidGroups = true;
				break;
			}
			case Pair:
			{
				if ( group.m_tiles.front().Type() == TileType::Wind )
				{
					WindTileType const wind = group.m_tiles.front().Get<TileType::Wind>();
					if ( wind == i_playerSeat || wind == i_round.Wind() )
					{
						// No Fu in my pinfu :(
						invalidGroups = true;
					}
				}
				break;
			}
			case Sequence:
			{
				// We love sequences in this house
				break;
			}
			}
		}

		if ( invalidGroups )
		{
			return NoYaku;
		}

		// Passed all checks
		return 1;
	}
};

struct Iipeikou
	: public NamedYaku<"Iipeikou">
{
	// Closed hand containing two identical sequences

	HanValue CalculateValue
	(
		Round const& i_round,
		Seat const& i_playerSeat,
		Player const& i_player,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const final
	{
		if ( i_assessment.m_open )
		{
			return NoYaku;
		}

		// God, calculating this sucks.
		// what an annoying yaku

		if ( i_interp.m_waitType != WaitType::Shanpon && i_interp.m_waitType != WaitType::Tanki )
		{
			// Need to consider the final group, as it completed a sequence
			HandInterpretation::Group finalGroup;
			finalGroup.m_type = GroupType::Sequence;
			finalGroup.m_tiles = i_interp.m_ungrouped;
			finalGroup.m_tiles.push_back( i_nextTile );

			for ( HandInterpretation::Group const& group : i_interp.m_groups )
			{
				if ( IsMatchingSequence( finalGroup, group ) )
				{
					return 1;
				}
			}
		}

		for ( size_t groupI = 0; groupI < i_interp.m_groups.size(); ++groupI )
		{
			for ( size_t secondGroupI = groupI + 1; secondGroupI < i_interp.m_groups.size(); ++secondGroupI )
			{
				if ( IsMatchingSequence( i_interp.m_groups[ groupI ], i_interp.m_groups[ secondGroupI ] ) )
				{
					return 1;
				}
			}
		}

		// Failed to find matches
		return NoYaku;
	}
private:
	static bool IsMatchingSequence( HandInterpretation::Group const& i_a, HandInterpretation::Group const& i_b )
	{
		if ( i_a.m_type == GroupType::Sequence && i_b.m_type == GroupType::Sequence )
		{
			return std::ranges::contains( i_b.m_tiles, i_a.m_tiles[ 0 ] )
				&& std::ranges::contains( i_b.m_tiles, i_a.m_tiles[ 1 ] )
				&& std::ranges::contains( i_b.m_tiles, i_a.m_tiles[ 2 ] );
		}
		return false;
	}
};

// TODO
struct HaiteiRaoyue{};

// TODO
struct HouteiRaoyui{};

struct RinshanKaihou
	: public NamedYaku<"RinshanKaihou">
{
	// Win on dead wall draw

	HanValue CalculateValue
	(
		Round const& i_round,
		Seat const& i_playerSeat,
		Player const& i_player,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const final
	{
		return i_nextTileType == TileDrawType::DeadWallDraw ? 1 : NoYaku;
	}
};

// TODO
struct Chankan{};

template<bool t_KuitanEnabled = true>
struct Tanyao
	: public NamedYaku<"Tanyao">
{
	// Only 2-8 tiles in hand

	HanValue CalculateValue
	(
		Round const& i_round,
		Seat const& i_playerSeat,
		Player const& i_player,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const final
	{
		// Kuitan rule required for open tanyao hands
		if constexpr ( !t_KuitanEnabled )
		{
			if ( i_assessment.m_open )
			{
				return NoYaku;
			}
		}

		if ( InvalidTile( i_nextTile ) || std::ranges::any_of( i_interp.m_ungrouped, InvalidTile ) )
		{
			return NoYaku;
		}

		for ( HandInterpretation::Group const& group : i_interp.m_groups )
		{
			for ( Tile const& tile : group.m_tiles )
			{
				if ( InvalidTile( tile ) )
				{
					return NoYaku;
				}
			}
		}

		return 1;
	}

private:
	static bool InvalidTile( Tile const& i_tile )
	{
		if ( i_tile.Type() != TileType::Suit )
		{
			return true;
		}

		SuitTile const& suitTile = i_tile.Get<TileType::Suit>();
		if ( suitTile.m_value == 1 || suitTile.m_value == 9 )
		{
			return true;
		}

		return false;
	}
};

// There are 5 yakuhai, each with their own name
// It's easier if we implement it in as few repetitions as possible
template<YakuNameString t_YakuhaiName, DragonTileType t_DragonType>
struct DragonYakuhai
	: public NamedYaku<t_YakuhaiName>
{
	// Need a triple of the specified type

	HanValue CalculateValue
	(
		Round const& i_round,
		Seat const& i_playerSeat,
		Player const& i_player,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const final
	{
		for ( HandInterpretation::Group const& group : i_interp.m_groups )
		{
			if ( group.m_type == GroupType::Quad || group.m_type == GroupType::Triplet )
			{
				if ( ValidTile( group.m_tiles.front() ) )
				{
					return 1;
				}
			}
		}

		if ( i_interp.m_waitType == WaitType::Shanpon && ValidTile( i_nextTile ) )
		{
			return 1;
		}

		return NoYaku;
	}
private:
	static bool ValidTile( Tile const& i_tile )
	{
		return i_tile.Type() == TileType::Dragon && i_tile.Get<TileType::Dragon>() == t_DragonType;
	}
};

struct Yakuhai_Haku
	: DragonYakuhai<"Haku", DragonTileType::White>
{};

struct Yakuhai_Hatsu
	: DragonYakuhai<"Hatsu", DragonTileType::Green>
{};

struct Yakuhai_Chun
	: DragonYakuhai<"Chun", DragonTileType::Red>
{};

struct Yakuhai_RoundWind
	: NamedYaku<"Bakaze">
{
	// Need a triple of the round wind

	HanValue CalculateValue
	(
		Round const& i_round,
		Seat const& i_playerSeat,
		Player const& i_player,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const final
	{
		for ( HandInterpretation::Group const& group : i_interp.m_groups )
		{
			if ( group.m_type == GroupType::Quad || group.m_type == GroupType::Triplet )
			{
				if ( ValidTile( group.m_tiles.front(), i_round.Wind() ) )
				{
					return 1;
				}
			}
		}

		if ( i_interp.m_waitType == WaitType::Shanpon && ValidTile( i_nextTile, i_round.Wind() ) )
		{
			return 1;
		}

		return NoYaku;
	}
private:
	static bool ValidTile( Tile const& i_tile, Seat const& i_roundWind )
	{
		return i_tile.Type() == TileType::Wind && i_tile.Get<TileType::Wind>() == i_roundWind;
	}
};

struct Yakuhai_SeatWind
	: NamedYaku<"Jikaze">
{
	// Need a triple of the seat wind

	HanValue CalculateValue
	(
		Round const& i_round,
		Seat const& i_playerSeat,
		Player const& i_player,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const final
	{
		for ( HandInterpretation::Group const& group : i_interp.m_groups )
		{
			if ( group.m_type == GroupType::Quad || group.m_type == GroupType::Triplet )
			{
				if ( ValidTile( group.m_tiles.front(), i_playerSeat ) )
				{
					return 1;
				}
			}
		}

		if ( i_interp.m_waitType == WaitType::Shanpon && ValidTile( i_nextTile, i_playerSeat ) )
		{
			return 1;
		}

		return NoYaku;
	}
private:
	static bool ValidTile( Tile const& i_tile, Seat const& i_playerSeat )
	{
		return i_tile.Type() == TileType::Wind && i_tile.Get<TileType::Wind>() == i_playerSeat;
	}
};

// TODO
struct DoubleRiichi{};

struct Chantaiyao
	: public NamedYaku<"Chantaiyao">
{
	// All groups must contain a terminal or honour

	HanValue CalculateValue
	(
		Round const& i_round,
		Seat const& i_playerSeat,
		Player const& i_player,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const final
	{
		for ( HandInterpretation::Group const& group : i_interp.m_groups )
		{
			if ( !std::ranges::any_of( group.m_tiles, RequiredTile ) )
			{
				return NoYaku;
			}
		}

		// Check final group too
		if ( !RequiredTile( i_nextTile )
			&& !std::ranges::any_of( i_interp.m_ungrouped, RequiredTile ) )
		{
			return NoYaku;
		}

		return i_assessment.m_open ? 1 : 2;
	}

private:
	static bool RequiredTile( Tile const& i_tile )
	{
		if ( i_tile.Type() != TileType::Suit )
		{
			return true;
		}

		SuitTile const& suitTile = i_tile.Get<TileType::Suit>();
		return suitTile.m_value == 1 || suitTile.m_value == 9;
	}
};

// TODO
struct SanshokuDoujun{};

// TODO
struct Ittsu{};

struct Toitoi
	: public NamedYaku<"Toitoi">
{
	// All non-pair groups must be triplets/quads

	HanValue CalculateValue
	(
		Round const& i_round,
		Seat const& i_playerSeat,
		Player const& i_player,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const final
	{
		for ( HandInterpretation::Group const& group : i_interp.m_groups )
		{
			if ( group.m_type == GroupType::Sequence )
			{
				return NoYaku;
			}
		}

		// Check if we're completing a sequence too
		if ( i_interp.m_waitType != WaitType::Shanpon || i_interp.m_waitType != WaitType::Tanki )
		{
			// Sequence wait
			return NoYaku;
		}

		return 2;
	}
};

struct Sanankou
	: public NamedYaku<"Sanankou">
{
	// Any 3 concealed triplets or quads in hand

	HanValue CalculateValue
	(
		Round const& i_round,
		Seat const& i_playerSeat,
		Player const& i_player,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const final
	{
		int concealedTripleCount = 0;
		for ( HandInterpretation::Group const& group : i_interp.m_groups )
		{
			if ( !group.m_open && ( group.m_type == GroupType::Quad || group.m_type == GroupType::Triplet ) )
			{
				++concealedTripleCount;
			}
		}

		// Check if we're completing a closed triplet too
		if ( i_nextTileType != TileDrawType::DiscardDraw && i_interp.m_waitType == WaitType::Shanpon )
		{
			++concealedTripleCount;
		}

		if ( concealedTripleCount >= 3 )
		{
			return 2;
		}

		return NoYaku;
	}
};

// TODO
struct SanshokuDoukou{};

struct Sankantsu
	: public NamedYaku<"Sankantsu">
{
	// Any 3 quads in hand

	HanValue CalculateValue
	(
		Round const& i_round,
		Seat const& i_playerSeat,
		Player const& i_player,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const final
	{
		// NB don't need to check winning group as it can't be a quad
		int quadCount = 0;
		for ( HandInterpretation::Group const& group : i_interp.m_groups )
		{
			if ( group.m_type == GroupType::Quad )
			{
				++quadCount;
			}
		}

		if ( quadCount >= 3 )
		{
			return 2;
		}

		return NoYaku;
	}
};

// TODO
struct Chiitoitsu{};

struct Honroutou
	: public NamedYaku<"Honroutou">
{
	// All tiles are terminals or honours

	HanValue CalculateValue
	(
		Round const& i_round,
		Seat const& i_playerSeat,
		Player const& i_player,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const final
	{
		for ( HandInterpretation::Group const& group : i_interp.m_groups )
		{
			if ( !std::ranges::all_of( group.m_tiles, ValidTile ) )
			{
				return NoYaku;
			}
		}

		// Check final group too
		if ( !ValidTile( i_nextTile )
			|| !std::ranges::all_of( i_interp.m_ungrouped, ValidTile ) )
		{
			return NoYaku;
		}

		return 2;
	}

private:
	static bool ValidTile( Tile const& i_tile )
	{
		if ( i_tile.Type() != TileType::Suit )
		{
			return true;
		}

		SuitTile const& suitTile = i_tile.Get<TileType::Suit>();
		return suitTile.m_value == 1 || suitTile.m_value == 9;
	}
};

// TODO
struct Shousangen{};

struct Honitsu
	: public NamedYaku<"Honitsu">
{
	// All tiles belong to exactly one of the three suits, or are honour tiles

	HanValue CalculateValue
	(
		Round const& i_round,
		Seat const& i_playerSeat,
		Player const& i_player,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const final
	{
		int suitTypeCount = 0;
		if ( i_assessment.m_containsManzu
			|| ( i_nextTile.Type() == TileType::Suit && i_nextTile.Get<TileType::Suit>().m_suit == Suit::Manzu ) )
		{
			++suitTypeCount;
		}
		if ( i_assessment.m_containsPinzu
			|| ( i_nextTile.Type() == TileType::Suit && i_nextTile.Get<TileType::Suit>().m_suit == Suit::Pinzu ) )
		{
			++suitTypeCount;
		}
		if ( i_assessment.m_containsSouzu
			|| ( i_nextTile.Type() == TileType::Suit && i_nextTile.Get<TileType::Suit>().m_suit == Suit::Souzu ) )
		{
			++suitTypeCount;
		}

		if ( suitTypeCount == 1 )
		{
			return i_assessment.m_open ? 2 : 3;
		}

		return NoYaku;
	}
};

struct JunchanTaiyao
	: public NamedYaku<"JunchanTaiyao">
{
	// All groups must contain a terminal

	HanValue CalculateValue
	(
		Round const& i_round,
		Seat const& i_playerSeat,
		Player const& i_player,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const final
	{
		// Only suit groups possible in a junchan hand
		if ( i_assessment.m_containsHonours || i_nextTile.Type() != TileType::Suit )
		{
			return NoYaku;
		}

		for ( HandInterpretation::Group const& group : i_interp.m_groups )
		{
			if ( !std::ranges::any_of( group.m_tiles, RequiredTile ) )
			{
				return NoYaku;
			}
		}

		// Check final group too
		if ( !RequiredTile( i_nextTile )
			&& !std::ranges::any_of( i_interp.m_ungrouped, RequiredTile ) )
		{
			return NoYaku;
		}

		return i_assessment.m_open ? 2 : 3;
	}

private:
	static bool RequiredTile( Tile const& i_tile )
	{
		if ( i_tile.Type() != TileType::Suit )
		{
			return false;
		}

		SuitTile const& suitTile = i_tile.Get<TileType::Suit>();
		return suitTile.m_value == 1 || suitTile.m_value == 9;
	}
};

// TODO
struct Ryanpeikou{};

struct Chinitsu
	: public NamedYaku<"Chinitsu">
{
	// All tiles belong to exactly one of the three suits

	HanValue CalculateValue
	(
		Round const& i_round,
		Seat const& i_playerSeat,
		Player const& i_player,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const final
	{
		if ( i_assessment.m_containsHonours || i_nextTile.Type() != TileType::Suit )
		{
			return NoYaku;
		}

		int suitTypeCount = 0;
		if ( i_assessment.m_containsManzu || i_nextTile.Get<TileType::Suit>().m_suit == Suit::Manzu )
		{
			++suitTypeCount;
		}
		if ( i_assessment.m_containsPinzu || i_nextTile.Get<TileType::Suit>().m_suit == Suit::Pinzu )
		{
			++suitTypeCount;
		}
		if ( i_assessment.m_containsSouzu || i_nextTile.Get<TileType::Suit>().m_suit == Suit::Souzu )
		{
			++suitTypeCount;
		}

		if ( suitTypeCount == 1 )
		{
			return i_assessment.m_open ? 5 : 6;
		}

		return NoYaku;
	}
};

// TODO: Kazoe Yakuman - not really a yaku

struct KokushiMusou
	: public NamedYaku<"KokushiMusou">
{
	// THIRTEEN ORPHANS
	// 19m19s19p1234567z + any terminal/honor, closed only

	HanValue CalculateValue
	(
		Round const& i_round,
		Seat const& i_playerSeat,
		Player const& i_player,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const final
	{
		if ( i_assessment.m_open )
		{
			return NoYaku;
		}

		// Sufficient to check that all tiles are terminals/honors and that the distinct tile count >= 13
		std::unordered_set<Tile> uniqueTiles;
		uniqueTiles.reserve( 14 );

		for ( HandInterpretation::Group const& group : i_interp.m_groups )
		{
			for ( Tile const& tile : group.m_tiles )
			{
				if ( !RequiredTile( tile ) )
				{
					return NoYaku;
				}
				uniqueTiles.insert( tile );
			}
		}

		for ( Tile const& tile : i_interp.m_ungrouped )
		{
			if ( !RequiredTile( tile ) )
			{
				return NoYaku;
			}
			uniqueTiles.insert( tile );
		}

		// And finally the big tile itself
		if ( !RequiredTile( i_nextTile ) )
		{
			return NoYaku;
		}
		uniqueTiles.insert( i_nextTile );

		if ( uniqueTiles.size() >= 13 )
		{
			return Yakuman;
		}

		return NoYaku;
	}
private:
	static bool RequiredTile( Tile const& i_tile )
	{
		if ( i_tile.Type() != TileType::Suit )
		{
			return true;
		}
		SuitTile const& suitTile = i_tile.Get<TileType::Suit>();
		return suitTile.m_value == 1 || suitTile.m_value == 9;
	}
};

struct Suuankou
	: public NamedYaku<"Suuankou">
{
	// Any 4 concealed triplets or quads in hand

	HanValue CalculateValue
	(
		Round const& i_round,
		Seat const& i_playerSeat,
		Player const& i_player,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const final
	{
		int concealedTripleCount = 0;
		for ( HandInterpretation::Group const& group : i_interp.m_groups )
		{
			if ( !group.m_open && ( group.m_type == GroupType::Quad || group.m_type == GroupType::Triplet ) )
			{
				++concealedTripleCount;
			}
		}

		// Check if we're completing a closed triplet too
		if ( i_nextTileType != TileDrawType::DiscardDraw && i_interp.m_waitType == WaitType::Shanpon )
		{
			++concealedTripleCount;
		}

		if ( concealedTripleCount >= 4 )
		{
			return Yakuman;
		}

		return NoYaku;
	}
};

struct Daisangen
	: public NamedYaku<"Daisangen">
{
	// beeg 3 dragons
	// triplets/quads of all 3 dragons required

	HanValue CalculateValue
	(
		Round const& i_round,
		Seat const& i_playerSeat,
		Player const& i_player,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const final
	{
		if ( !i_assessment.m_containsDragons )
		{
			return NoYaku;
		}

		int dragonTripletCount = 0; // can't have more than one triplet of the same dragon so 3 triplets = 3 dragon types

		for ( HandInterpretation::Group const& group : i_interp.m_groups )
		{
			if ( group.m_type == GroupType::Triplet || group.m_type == GroupType::Quad )
			{
				if ( group.m_tiles.front().Type() == TileType::Dragon )
				{
					++dragonTripletCount;
				}
			}
		}

		if ( i_interp.m_waitType == WaitType::Shanpon && i_nextTile.Type() == TileType::Dragon )
		{
			++dragonTripletCount;
		}

		if ( dragonTripletCount >= 3 )
		{
			return Yakuman;
		}

		return NoYaku;
	}
};

struct Shousuushii
	: public NamedYaku<"Shousuushii">
{
	// small 4 winds
	// triplets/quads of all 3 winds required + pair of 4th wind

	HanValue CalculateValue
	(
		Round const& i_round,
		Seat const& i_playerSeat,
		Player const& i_player,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const final
	{
		if ( !i_assessment.m_containsWinds )
		{
			return NoYaku;
		}

		int windTripletCount = 0; // can't have more than one triplet of the same wind so 3 triplets = 3 different winds
		bool foundWindPair = false;

		for ( HandInterpretation::Group const& group : i_interp.m_groups )
		{
			if ( group.m_type == GroupType::Triplet || group.m_type == GroupType::Quad )
			{
				if ( group.m_tiles.front().Type() == TileType::Wind )
				{
					++windTripletCount;
				}
			}
			else if ( group.m_type == GroupType::Pair )
			{
				if ( group.m_tiles.front().Type() != TileType::Wind )
				{
					return NoYaku;
				}
			}
		}

		if ( i_interp.m_waitType == WaitType::Tanki && i_nextTile.Type() != TileType::Wind )
		{
			return NoYaku;
		}
		else if ( i_interp.m_waitType == WaitType::Shanpon && i_nextTile.Type() == TileType::Wind )
		{
			++windTripletCount;
		}

		// Require exactly 3, as shousuushii is exclusive from daisuushii
		if ( windTripletCount == 3 )
		{
			return Yakuman;
		}

		return NoYaku;
	}
};

struct Daisuushii
	: public NamedYaku<"Daisuushii">
{
	// big 4 winds
	// triplets/quads of all 4 winds required

	HanValue CalculateValue
	(
		Round const& i_round,
		Seat const& i_playerSeat,
		Player const& i_player,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const final
	{
		if ( !i_assessment.m_containsWinds )
		{
			return NoYaku;
		}

		int windTripletCount = 0; // can't have more than one triplet of the same wind so 4 triplets = 4 big winds

		for ( HandInterpretation::Group const& group : i_interp.m_groups )
		{
			if ( group.m_type == GroupType::Triplet || group.m_type == GroupType::Quad )
			{
				if ( group.m_tiles.front().Type() == TileType::Wind )
				{
					++windTripletCount;
				}
			}
		}

		if ( i_interp.m_waitType == WaitType::Shanpon && i_nextTile.Type() == TileType::Wind )
		{
			++windTripletCount;
		}

		if ( windTripletCount >= 4 )
		{
			return Yakuman;
		}

		return NoYaku;
	}
};

struct Tsuuiisou
	: public NamedYaku<"Tsuuiisou">
{
	// all honors

	HanValue CalculateValue
	(
		Round const& i_round,
		Seat const& i_playerSeat,
		Player const& i_player,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const final
	{
		if ( i_assessment.m_containsSuits || i_nextTile.Type() == TileType::Suit )
		{
			return NoYaku;
		}

		return Yakuman;
	}
};

struct Chinroutou
	: public NamedYaku<"Chinroutou">
{
	// all terminals

	HanValue CalculateValue
	(
		Round const& i_round,
		Seat const& i_playerSeat,
		Player const& i_player,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const final
	{
		if ( i_assessment.m_containsHonours || !i_assessment.m_containsTerminals ||  i_nextTile.Type() != TileType::Suit )
		{
			return NoYaku;
		}

		for ( HandInterpretation::Group const& group : i_interp.m_groups )
		{
			if ( !std::ranges::all_of( group.m_tiles, RequiredTile ) )
			{
				return NoYaku;
			}
		}

		if ( !RequiredTile( i_nextTile )
			|| !std::ranges::all_of( i_interp.m_ungrouped, RequiredTile ) )
		{
			return NoYaku;
		}

		return Yakuman;
	}
private:
	static bool RequiredTile( Tile const& i_tile )
	{
		SuitTile const& suitTile = i_tile.Get<TileType::Suit>();
		return suitTile.m_value == 1 || suitTile.m_value == 9;
	}
};

struct Ryuuiisou
	: public NamedYaku<"Ryuuiisou">
{
	// all green
	// 23468s7z only

	HanValue CalculateValue
	(
		Round const& i_round,
		Seat const& i_playerSeat,
		Player const& i_player,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const final
	{
		// Early-out on some broad strokes, for optimisation
		if ( i_assessment.m_containsWinds
			|| i_assessment.m_containsManzu
			|| i_assessment.m_containsPinzu
			|| !i_assessment.m_containsSouzu
			|| i_nextTile.Type() == TileType::Wind
			|| ( i_nextTile.Type() == TileType::Suit && i_nextTile.Get<TileType::Suit>().m_suit != Suit::Souzu ) )
		{
			return NoYaku;
		}

		for ( HandInterpretation::Group const& group : i_interp.m_groups )
		{
			if ( !std::ranges::all_of( group.m_tiles, RequiredTile ) )
			{
				return NoYaku;
			}
		}

		if ( !RequiredTile( i_nextTile )
			|| !std::ranges::all_of( i_interp.m_ungrouped, RequiredTile ) )
		{
			return NoYaku;
		}

		return Yakuman;
	}
private:
	static bool RequiredTile( Tile const& i_tile )
	{
		if ( i_tile.Type() == TileType::Suit )
		{
			// Already checked tile is souzu
			SuitTile const& suitTile = i_tile.Get<TileType::Suit>();
			return suitTile.m_value == 2
				|| suitTile.m_value == 3
				|| suitTile.m_value == 4
				|| suitTile.m_value == 6
				|| suitTile.m_value == 8
				;
		}

		// Already checked tile, if not suit, is a dragon
		return i_tile.Get<TileType::Dragon>() == DragonTileType::Green;
	}
};

struct ChuurenPoutou
	: public NamedYaku<"ChuurenPoutou">
{
	// NINE GATES
	// 1112345678999 + any, in same suit, closed only

	HanValue CalculateValue
	(
		Round const& i_round,
		Seat const& i_playerSeat,
		Player const& i_player,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const final
	{
		if ( i_assessment.m_open || i_assessment.m_containsHonours || i_nextTile.Type() != TileType::Suit )
		{
			return NoYaku;
		}

		Suit const requiredSuit = i_nextTile.Get<TileType::Suit>().m_suit;

		std::array<int, 9> requiredOfEachValue = {
			3, 1, 1,
			1, 1, 1,
			1, 1, 3,
		};

		auto fnEvalTile = [ requiredSuit, &requiredOfEachValue ]( Tile const& i_tile )
		{
			if ( i_tile.Get<TileType::Suit>().m_suit != requiredSuit )
			{
				return false;
			}
			--requiredOfEachValue[ i_tile.Get<TileType::Suit>().m_value ];
			return true;
		};

		for ( HandInterpretation::Group const& group : i_interp.m_groups )
		{
			for ( Tile const& tile : group.m_tiles )
			{
				if ( !fnEvalTile( tile ) )
				{
					return NoYaku;
				}
			}
		}

		for ( Tile const& tile : i_interp.m_ungrouped )
		{
			if ( !fnEvalTile( tile ) )
			{
				return NoYaku;
			}
		}

		// And finally the big tile itself
		fnEvalTile( i_nextTile );

		// If every value is 0 or less (technically, there should be exactly one value with -1, the others all 0)
		// then we have the yakuman
		if ( std::ranges::all_of( requiredOfEachValue, []( int n ) { return n <= 0; } ) )
		{
			return Yakuman;
		}

		return NoYaku;
	}
};

struct Suukantsu
	: public NamedYaku<"Suukantsu">
{
	// Any 4 quads in hand, can be open or closed

	HanValue CalculateValue
	(
		Round const& i_round,
		Seat const& i_playerSeat,
		Player const& i_player,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const final
	{
		int kanCount = 0;
		for ( HandInterpretation::Group const& group : i_interp.m_groups )
		{
			if ( group.m_type == GroupType::Quad )
			{
				++kanCount;
			}
		}

		// N.B. winning tile is necessarily a pair, so no need to check the wait tiles

		if ( kanCount >= 4 )
		{
			return Yakuman;
		}

		return NoYaku;
	}
};

// TODO
struct Tenhou{};

// TODO
struct Chihou{};

// TODO
struct NagashiMangan{};

}
}
