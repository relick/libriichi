#pragma once

#include "Declare.hpp"
#include "Tile.hpp"

#include <optional>

namespace Riichi
{

//-----------------------------------------------------------------------------
struct NoYakuType {};
inline constexpr NoYakuType NoYaku;
struct YakumanType {};
inline constexpr YakumanType Yakuman;

//-----------------------------------------------------------------------------
class HanValue
{
	static constexpr Han c_yakumanValue = 13;

	std::optional<Han> m_value;

public:
	HanValue( Han i_value )
		: m_value{ i_value }
	{}

	HanValue( NoYakuType )
	{}

	HanValue( YakumanType )
		: m_value{ c_yakumanValue }
	{}

	bool IsValid() const { return m_value.has_value(); }
	bool IsYakuman() const { return IsValid() && m_value.value() == c_yakumanValue; }
	Han Get() const { return m_value.value(); }
};

//-----------------------------------------------------------------------------
struct Yaku
{
	virtual ~Yaku() = default;

	virtual char const* Name() const = 0;

	// NB do not need to check whether i_nextTile is in the interp's waits - it will be
	virtual HanValue CalculateValue
	(
		RoundData const& i_round,
		Seat const& i_playerSeat,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const = 0;
};

//-----------------------------------------------------------------------------
template<size_t N>
struct YakuNameString
{
	constexpr YakuNameString( const char( &i_str )[ N ] )
	{
		std::copy_n( i_str, N, m_str );
	}

	char m_str[ N ];
};

//-----------------------------------------------------------------------------
template<YakuNameString t_YakuName>
struct NamedYaku
	: public Yaku
{
	char const* Name() const final { return t_YakuName.m_str; }
};

}
