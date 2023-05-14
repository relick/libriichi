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

// Overall TODO: Lots of the yaku need to assess the 'final group' separately from the rest of the groups. This is to avoid creating a container and doing a bunch of copies where unnecessary
// It would be nice to clean this up somehow. It would also be nice if all the yaku could be made to avoid allocating entirely when assessing!


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

struct Riichi
	: public NamedYaku<"Riichi">
{
	// Win after calling riichi

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
		i_round.CalledRiichi( i_playerSeat ) ? 1 : NoYaku;
	}
};

struct Ippatsu
	: public NamedYaku<"Ippatsu">
{
	// Win after calling riichi before your next discard and before any calls

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
		// Oh yeah, we're in 'hardcode the yaku' town
		// it's just easier this way, for now
		if ( i_round.RiichiIppatsuValid( i_playerSeat ) )
		{
			return 1;
		}
		
		return NoYaku;
	}
};

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
		if ( i_assessment.m_open || i_assessment.m_containsTileType[ TileType::Dragon ] )
		{
			// Invalid hand for pinfu
			return NoYaku;
		}

		if ( i_interp.m_waitType != WaitType::Ryanmen )
		{
			// Didn't win on open wait
			return NoYaku;
		}

		for ( HandGroup const& group : i_interp.m_groups )
		{
			switch ( group.Type() )
			{
			using enum GroupType;
			case Triplet:
			case Quad:
			{
				return NoYaku;
			}
			case Pair:
			{
				if ( group.TilesType() == TileType::Wind )
				{
					WindTileType const wind = group[ 0 ].Get<TileType::Wind>();
					if ( wind == i_playerSeat || wind == i_round.Wind() )
					{
						// No Fu in my pinfu :(
						return NoYaku;
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

		if ( SequenceWait( i_interp.m_waitType ) )
		{
			// Need to consider the final group, as it completed a sequence
			HandGroup const finalGroup( i_interp, i_nextTile );

			for ( HandGroup const& group : i_interp.m_groups )
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
	static bool IsMatchingSequence( HandGroup const& i_a, HandGroup const& i_b )
	{
		if ( i_a.Type() == GroupType::Sequence && i_b.Type() == GroupType::Sequence )
		{
			return i_a.Tiles() == i_b.Tiles();
		}
		return false;
	}
};

struct HaiteiRaoyue
	: public NamedYaku<"HaiteiRaoyue">
{
	// Win with tsumo on final tile from the wall

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
		if ( i_nextTileType == TileDrawType::SelfDraw && i_round.WallSize() == 0u )
		{
			return 1;
		}

		return NoYaku;
	}
};

struct HouteiRaoyui
	: public NamedYaku<"HouteiRaoyui">
{
	// Win with ron on final discard

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
		if ( i_nextTileType == TileDrawType::DiscardDraw && i_round.WallSize() == 0u )
		{
			return 1;
		}

		return NoYaku;
	}
};

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

struct Chankan
	: public NamedYaku<"Chankan">
{
	// Win on an upgraded kan tile

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
		return i_nextTileType == TileDrawType::KanTheft ? 1 : NoYaku;
	}
};

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

		for ( HandGroup const& group : i_interp.m_groups )
		{
			for ( Tile const& tile : group.Tiles() )
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
		for ( HandGroup const& group : i_interp.m_groups )
		{
			if ( TripletCompatible( group.Type() ) )
			{
				if ( ValidTile( group[ 0 ] ) )
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
		for ( HandGroup const& group : i_interp.m_groups )
		{
			if ( TripletCompatible( group.Type() ) )
			{
				if ( ValidTile( group[ 0 ], i_round.Wind() ) )
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
		for ( HandGroup const& group : i_interp.m_groups )
		{
			if ( TripletCompatible( group.Type() ) )
			{
				if ( ValidTile( group[ 0 ], i_playerSeat ) )
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

struct DoubleRiichi
	: public NamedYaku<"Riichi">
{
	// Win after calling riichi on the very first discard

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
		i_round.CalledDoubleRiichi( i_playerSeat )  ? 2 : NoYaku;
	}
};

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
		for ( HandGroup const& group : i_interp.m_groups )
		{
			if ( !std::ranges::any_of( group.Tiles(), RequiredTile ) )
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

struct SanshokuDoujun
	: public NamedYaku<"SanshokuDoujun">
{
	// Three sequences with the same numbers but different suits.
	// Openness affects value

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
		// Note 1: Unfortunately, the fourth group and pair are totally unrelated, so we can't make quick rule-outs without checking all combos
		// Note 2: We can also stop when we find a match or when we are sure we don't have a match, so it's not too bad of a search at least.

		if ( !std::ranges::all_of( i_assessment.m_containsSuit, std::identity{} ) )
		{
			return NoYaku;
		}

		// the literal worst yaku

		if ( SequenceWait( i_interp.m_waitType ) )
		{
			// Need to consider the final group, as it completed a sequence
			HandGroup const finalGroup( i_interp, i_nextTile );

			for ( size_t groupI = 0; groupI < i_interp.m_groups.size(); ++groupI )
			{
				for ( size_t secondGroupI = groupI + 1; secondGroupI < i_interp.m_groups.size(); ++secondGroupI )
				{
					if ( Sanshoku( finalGroup, i_interp.m_groups[ groupI ], i_interp.m_groups[ secondGroupI ] ) )
					{
						return i_assessment.m_open ? 1 : 2;
					}
				}
			}
		}

		// Didn't find one with final group, so check the existing groups
		for ( size_t groupI = 0; groupI < i_interp.m_groups.size(); ++groupI )
		{
			for ( size_t secondGroupI = groupI + 1; secondGroupI < i_interp.m_groups.size(); ++secondGroupI )
			{
				for ( size_t thirdGroupI = secondGroupI + 1; thirdGroupI < i_interp.m_groups.size(); ++thirdGroupI )
				{
					if ( Sanshoku( i_interp.m_groups[ groupI ], i_interp.m_groups[ secondGroupI ], i_interp.m_groups[ thirdGroupI ] ) )
					{
						return i_assessment.m_open ? 1 : 2;
					}
				}
			}
		}

		// Failed to find matches
		return NoYaku;
	}
private:
	static bool Sanshoku( HandGroup const& i_a, HandGroup const& i_b, HandGroup const& i_c )
	{
		if ( i_a.Type() != GroupType::Sequence || i_b.Type() != GroupType::Sequence || i_c.Type() != GroupType::Sequence )
		{
			return false;
		}

		if ( i_a.CommonSuit() == i_b.CommonSuit() || i_b.CommonSuit() == i_c.CommonSuit() || i_a.CommonSuit() == i_c.CommonSuit() )
		{
			return false;
		}

		return std::ranges::all_of(
			std::views::zip( i_a.Tiles(), i_b.Tiles(), i_c.Tiles() ),
			[]( auto const& i_tileSet )
			{
				auto const& [tileA, tileB, tileC] = i_tileSet;
				return tileA.Get<TileType::Suit>().m_value == tileB.Get<TileType::Suit>().m_value
					&& tileB.Get<TileType::Suit>().m_value == tileC.Get<TileType::Suit>().m_value;
			}
		);
	}
};

struct Ikkitsuukan
	: public NamedYaku<"Ikkitsuukan">
{
	// Usually called 'ittsuu'
	// One suit, three sequences, 123 456 789
	// Openness affects value

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
		// This one isn't so bad. We'll make a matrix and just search all our groups

		Utils::EnumIndexedArray<std::array<bool, 3>, Suit, c_suitCount> groupsPerSuit{};

		auto fnEvalGroup = [ &groupsPerSuit ]( HandGroup const& i_group )
		{
			if ( i_group.Type() != GroupType::Sequence )
			{
				return;
			}

			for ( uint8_t i = 1; i <= 3; ++i )
			{
				if ( i_group[ 0 ].Get<TileType::Suit>().m_value == ( SuitTileValue::Set<1>() * i )
					&& i_group[ 1 ].Get<TileType::Suit>().m_value == ( SuitTileValue::Set<2>() * i )
					&& i_group[ 2 ].Get<TileType::Suit>().m_value == ( SuitTileValue::Set<3>() * i ) )
				{
					groupsPerSuit[ i_group.CommonSuit() ][ i - 1 ] = true;
					break;
				}
			}
		};

		if ( SequenceWait( i_interp.m_waitType ) )
		{
			// Need to consider the final group, as it completed a sequence
			HandGroup const finalGroup( i_interp, i_nextTile );

			fnEvalGroup( finalGroup );
		}

		for ( HandGroup const& group : i_interp.m_groups )
		{
			fnEvalGroup( group );
		}

		if ( std::ranges::any_of( groupsPerSuit, []( auto const& arr ) { return std::ranges::all_of( arr, std::identity{} ); } ) )
		{
			return i_assessment.m_open ? 1 : 2;
		}

		// Failed to find ittsuu
		return NoYaku;
	}
};

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
		for ( HandGroup const& group : i_interp.m_groups )
		{
			if ( group.Type() == GroupType::Sequence )
			{
				return NoYaku;
			}
		}

		// Check if we're completing a sequence too
		if ( SequenceWait( i_interp.m_waitType ) )
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
		for ( HandGroup const& group : i_interp.m_groups )
		{
			if ( !group.Open() && TripletCompatible( group.Type() ) )
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

struct SanshokuDoukou
	: public NamedYaku<"SanshokuDoukou">
{
	// Three triplets with the same number but different suits.

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
		// Note 1: Unfortunately, the fourth group and pair are totally unrelated, so we can't make quick rule-outs without checking all combos
		// Note 2: We can also stop when we find a match or when we are sure we don't have a match, so it's not too bad of a search at least.

		if ( !std::ranges::all_of( i_assessment.m_containsSuit, std::identity{} ) )
		{
			return NoYaku;
		}

		// the literal second worst yaku

		if ( i_interp.m_waitType == WaitType::Shanpon )
		{
			// Need to consider the final group, as it completed a triplet
			HandGroup const finalGroup( i_interp, i_nextTile );

			for ( size_t groupI = 0; groupI < i_interp.m_groups.size(); ++groupI )
			{
				for ( size_t secondGroupI = groupI + 1; secondGroupI < i_interp.m_groups.size(); ++secondGroupI )
				{
					if ( Sanshoku( finalGroup, i_interp.m_groups[ groupI ], i_interp.m_groups[ secondGroupI ] ) )
					{
						return 2;
					}
				}
			}
		}

		// Didn't find one with final group, so check the existing groups
		for ( size_t groupI = 0; groupI < i_interp.m_groups.size(); ++groupI )
		{
			for ( size_t secondGroupI = groupI + 1; secondGroupI < i_interp.m_groups.size(); ++secondGroupI )
			{
				for ( size_t thirdGroupI = secondGroupI + 1; thirdGroupI < i_interp.m_groups.size(); ++thirdGroupI )
				{
					if ( Sanshoku( i_interp.m_groups[ groupI ], i_interp.m_groups[ secondGroupI ], i_interp.m_groups[ thirdGroupI ] ) )
					{
						return 2;
					}
				}
			}
		}

		// Failed to find matches
		return NoYaku;
	}
private:
	static bool Sanshoku( HandGroup const& i_a, HandGroup const& i_b, HandGroup const& i_c )
	{
		if ( !TripletCompatible( i_a.Type() ) || !TripletCompatible( i_b.Type() ) || !TripletCompatible( i_c.Type() ) )
		{
			return false;
		}

		// Check suits
		if ( i_a.CommonSuit() == i_b.CommonSuit() || i_b.CommonSuit() == i_c.CommonSuit() || i_a.CommonSuit() == i_c.CommonSuit() )
		{
			return false;
		}
		
		// Check values match
		return i_a.CommonSuitTileValue() == i_b.CommonSuitTileValue() && i_b.CommonSuitTileValue() == i_c.CommonSuitTileValue();
	}
};

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
		for ( HandGroup const& group : i_interp.m_groups )
		{
			if ( group.Type() == GroupType::Quad )
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

// TODO: The main and interpretations algorithm will fail to recognise 13 orphans
struct Chiitoitsu
	: public NamedYaku<"Chiitoitsu">
{
	// Hand consists of 7 unique pairs
	// Must be closed, by nature of the hand structure

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
		if ( i_interp.m_waitType != WaitType::Tanki )
		{
			return NoYaku;
		}

		std::unordered_set<Tile> uniqueTiles;
		uniqueTiles.insert( i_nextTile );
		
		for ( HandGroup const& group : i_interp.m_groups )
		{
			if ( group.Type() != GroupType::Pair )
			{
				return NoYaku;
			}
			uniqueTiles.insert( group[ 0 ] );
		}

		return uniqueTiles.size() == 7 ? 2 : NoYaku;
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
		for ( HandGroup const& group : i_interp.m_groups )
		{
			if ( !std::ranges::all_of( group.Tiles(), ValidTile ) )
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

struct Shousangen
	: public NamedYaku<"Shousangen">
{
	// smol 3 dragons
	// triplets/quads of 2 dragons required + pair of 3rd dragon

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
		if ( !i_assessment.m_containsTileType[ TileType::Dragon ] )
		{
			return NoYaku;
		}

		int dragonTripletCount = 0; // can't have more than one triplet of the same dragon so 2 triplets = 2 different dragons

		for ( HandGroup const& group : i_interp.m_groups )
		{
			if ( TripletCompatible( group.Type() ) )
			{
				if ( group.TilesType() == TileType::Dragon )
				{
					++dragonTripletCount;
				}
			}
			else if ( group.Type() == GroupType::Pair )
			{
				if ( group.TilesType() != TileType::Dragon )
				{
					return NoYaku;
				}
			}
		}

		if ( i_interp.m_waitType == WaitType::Tanki && i_nextTile.Type() != TileType::Dragon )
		{
			return NoYaku;
		}
		else if ( i_interp.m_waitType == WaitType::Shanpon && i_nextTile.Type() == TileType::Dragon )
		{
			++dragonTripletCount;
		}

		// Require exactly 2, as shousangen is exclusive from daisangen
		if ( dragonTripletCount == 2 )
		{
			return Yakuman;
		}

		return NoYaku;
	}
};

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
		for ( Suit suit : Suits{} )
		{
			if ( i_assessment.m_containsSuit[ suit ]
				|| ( i_nextTile.Type() == TileType::Suit && i_nextTile.Get<TileType::Suit>().m_suit == suit ) )
			{
				++suitTypeCount;
			}
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

		for ( HandGroup const& group : i_interp.m_groups )
		{
			if ( !std::ranges::any_of( group.Tiles(), RequiredTile ) )
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

struct Ryanpeikou
	: public NamedYaku<"Ryanpeikou">
{
	// Closed hand containing two sets of two identical sequences

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
		if ( i_assessment.m_open || i_interp.m_waitType == WaitType::Shanpon )
		{
			return NoYaku;
		}

		// All my homies hate ryanpeikou
		// Alright, note 1: if we find an iipeikou, it's sufficient to check for another iipeikou in the remaining groups i.e. there's no way we'd miss it by selecting a 'wrong' different pair
		// Note 2: ryanpeikou needs 4 sequences, therefore it must involve the winning group unless it's a tanki wait
		// this actually makes it slightly simpler than iipeikou, in a way

		if ( i_interp.m_waitType == WaitType::Tanki )
		{
			for ( size_t groupI = 0; groupI < i_interp.m_groups.size(); ++groupI )
			{
				for ( size_t secondGroupI = groupI + 1; secondGroupI < i_interp.m_groups.size(); ++secondGroupI )
				{
					if ( IsMatchingSequence( i_interp.m_groups[ groupI ], i_interp.m_groups[ secondGroupI ] ) )
					{
						// We got one! Check the other!

						// I don't like this code, but it works I guess
						size_t thirdGroupI = 0;
						size_t fourthGroupI = 0;
						while ( thirdGroupI == groupI || thirdGroupI == secondGroupI )
						{
							thirdGroupI++;
						}
						while ( fourthGroupI == groupI || fourthGroupI == secondGroupI || fourthGroupI == thirdGroupI )
						{
							fourthGroupI++;
						}
						return IsMatchingSequence( i_interp.m_groups[ thirdGroupI ], i_interp.m_groups[ fourthGroupI ] ) ? 3 : NoYaku;
					}
				}
			}
		}
		else
		{
			// Need to consider the final group, as it completed a sequence
			HandGroup const finalGroup( i_interp, i_nextTile );

			for ( size_t groupI = 0; groupI < i_interp.m_groups.size(); ++groupI )
			{
				if ( IsMatchingSequence(  finalGroup, i_interp.m_groups[ groupI ] ) )
				{
					// We got one! Check the other!
					size_t thirdGroupI = 0;
					size_t fourthGroupI = 0;
					while ( thirdGroupI == groupI )
					{
						thirdGroupI++;
					}
					while ( fourthGroupI == groupI || fourthGroupI == thirdGroupI )
					{
						fourthGroupI++;
					}
					return IsMatchingSequence( i_interp.m_groups[ thirdGroupI ], i_interp.m_groups[ fourthGroupI ] ) ? 3 : NoYaku;
				}
			}
		}

		// Failed to find ryanpeikou
		return NoYaku;
	}
private:
	static bool IsMatchingSequence( HandGroup const& i_a, HandGroup const& i_b )
	{
		if ( i_a.Type() == GroupType::Sequence && i_b.Type() == GroupType::Sequence )
		{
			return i_a.Tiles() == i_b.Tiles();
		}
		return false;
	}
};

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
		for ( Suit suit : Suits{} )
		{
			if ( i_assessment.m_containsSuit[ suit ] || i_nextTile.Get<TileType::Suit>().m_suit == suit )
			{
				++suitTypeCount;
			}
		}

		if ( suitTypeCount == 1 )
		{
			return i_assessment.m_open ? 5 : 6;
		}

		return NoYaku;
	}
};

// TODO: The main and interpretations algorithm will fail to recognise 13 orphans
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

		for ( HandGroup const& group : i_interp.m_groups )
		{
			for ( Tile const& tile : group.Tiles() )
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
		for ( HandGroup const& group : i_interp.m_groups )
		{
			if ( !group.Open() && TripletCompatible( group.Type() ) )
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
		if ( !i_assessment.m_containsTileType[ TileType::Dragon ] )
		{
			return NoYaku;
		}

		int dragonTripletCount = 0; // can't have more than one triplet of the same dragon so 3 triplets = 3 dragon types

		for ( HandGroup const& group : i_interp.m_groups )
		{
			if ( TripletCompatible( group.Type() ) )
			{
				if ( group.TilesType() == TileType::Dragon )
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
	// triplets/quads of 3 winds required + pair of 4th wind

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
		if ( !i_assessment.m_containsTileType[ TileType::Wind ] )
		{
			return NoYaku;
		}

		int windTripletCount = 0; // can't have more than one triplet of the same wind so 3 triplets = 3 different winds

		for ( HandGroup const& group : i_interp.m_groups )
		{
			if ( TripletCompatible( group.Type() ) )
			{
				if ( group.TilesType() == TileType::Wind )
				{
					++windTripletCount;
				}
			}
			else if ( group.Type() == GroupType::Pair )
			{
				if ( group.TilesType() != TileType::Wind )
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
		if ( !i_assessment.m_containsTileType[ TileType::Wind ] )
		{
			return NoYaku;
		}

		int windTripletCount = 0; // can't have more than one triplet of the same wind so 4 triplets = 4 big winds

		for ( HandGroup const& group : i_interp.m_groups )
		{
			if ( TripletCompatible( group.Type() ) )
			{
				if ( group.TilesType() == TileType::Wind )
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
		if ( i_assessment.m_containsTileType[ TileType::Suit ] || i_nextTile.Type() == TileType::Suit )
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

		for ( HandGroup const& group : i_interp.m_groups )
		{
			if ( !std::ranges::all_of( group.Tiles(), RequiredTile ) )
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
		if ( i_assessment.m_containsTileType[ TileType::Wind ]
			|| i_assessment.m_containsSuit[ Suit::Manzu ]
			|| i_assessment.m_containsSuit[ Suit::Pinzu ]
			|| !i_assessment.m_containsSuit[ Suit::Souzu ]
			|| i_nextTile.Type() == TileType::Wind
			|| ( i_nextTile.Type() == TileType::Suit && i_nextTile.Get<TileType::Suit>().m_suit != Suit::Souzu ) )
		{
			return NoYaku;
		}

		for ( HandGroup const& group : i_interp.m_groups )
		{
			if ( !std::ranges::all_of( group.Tiles(), RequiredTile ) )
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

		for ( HandGroup const& group : i_interp.m_groups )
		{
			for ( Tile const& tile : group.Tiles() )
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
		for ( HandGroup const& group : i_interp.m_groups )
		{
			if ( group.Type() == GroupType::Quad )
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

struct Tenhou
	: public NamedYaku<"Tenhou">
{
	// Win as the dealer on the first tile

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
		if ( i_round.IsDealer( i_playerSeat ) && i_round.Discards( i_playerSeat ).empty() )
		{
			return Yakuman;
		}

		return NoYaku;
	}
};

struct Chihou
	: public NamedYaku<"Chihou">
{
	// Win as a non-dealer on the first tile drawn

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
		if ( i_nextTileType == TileDrawType::SelfDraw
			&& !i_round.CallsMade()
			&& !i_round.IsDealer( i_playerSeat )
			&& i_round.Discards( i_playerSeat ).empty()
			)
		{
			return Yakuman;
		}

		return NoYaku;
	}
};

// TODO: Scoring options that aren't typical yaku
// Kazoe Yakuman
// Nagashi Mangan

}
}
