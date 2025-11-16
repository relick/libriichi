#pragma once

#include "Containers.hpp"
#include "Declare.hpp"
#include "Tile.hpp"

namespace Riichi
{

//------------------------------------------------------------------------------
struct NoYakuType {};
inline constexpr NoYakuType NoYaku;
struct YakumanType {};
inline constexpr YakumanType Yakuman;

//------------------------------------------------------------------------------
class HanValue
{
	static constexpr Han c_yakumanValue = 13;

	Option<Han> m_value;

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

//------------------------------------------------------------------------------
struct YakuEvaluator
{
	virtual ~YakuEvaluator() = default;

	virtual char const* Name() const = 0;
	virtual char const* InterpreterName() const = 0;
	bool UsesInterpreter( char const* i_interp ) const { /*TODO-OPT*/return std::strcmp(InterpreterName(), i_interp) == 0; }

	// NB do not need to check whether i_lastTile is in the interp's waits - it will be
	virtual HanValue CalculateValue
	(
		Round const& i_round,
		Seat const& i_playerSeat,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_lastTile,
		TileDrawType const& i_lastTileDrawType
	) const = 0;

	// Would this yaku be valid if we riichi'd? This will determine if riichi can be done or not
	// A little bit of a bodge but saves having to do hand assessments more than once
	virtual bool AddsYakuToRiichi() const { return false; }
};

//------------------------------------------------------------------------------
template<size_t N>
struct NameString
{
	constexpr NameString( const char( &i_str )[ N ] )
	{
		std::copy_n( i_str, N, m_str );
	}

	char m_str[ N ];
};

//------------------------------------------------------------------------------
template<NameString t_YakuName, NameString t_InterpreterName>
struct NamedYaku
	: public YakuEvaluator
{
	char const* Name() const final { return t_YakuName.m_str; }
	char const* InterpreterName() const final { return t_InterpreterName.m_str; }
};

//------------------------------------------------------------------------------
// Helper macros for reducing boilerplate. Define a yaku class just by giving a name
// Use DECLARE_YAKU(name); to declare a whole class in one go
// Use BEGIN_YAKU(name) and END_YAKU(); to add your own (private) contents to the class
// Finally, use YAKU_CALCULATEVALUE_PARAMS() to cut all the parameter boilerplate
//------------------------------------------------------------------------------
#define YAKU_CALCULATEVALUE_PARAMS()		\
Round const& i_round,					\
Seat const& i_playerSeat,					\
Hand const& i_hand,							\
HandAssessment const& i_assessment,			\
HandInterpretation const& i_interp,			\
Tile const& i_lastTile,						\
TileDrawType const& i_lastTileDrawType

#define BEGIN_YAKU( NAME, INTERPRETER )		\
struct NAME									\
	: public NamedYaku< #NAME, #INTERPRETER >\
{											\
	HanValue CalculateValue					\
	(										\
		YAKU_CALCULATEVALUE_PARAMS()		\
	) const final;							\
private:

#define END_YAKU()							\
}

#define DECLARE_YAKU( NAME, INTERPRETER ) BEGIN_YAKU( NAME, INTERPRETER ) END_YAKU()

}
