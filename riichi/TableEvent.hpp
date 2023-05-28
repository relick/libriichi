#pragma once

#include "Base.hpp"
#include "Containers.hpp"
#include "NamedUnion.hpp"
#include "Utils.hpp"

#include <utility>

namespace Riichi
{

//------------------------------------------------------------------------------
enum class TableEventType : EnumValueType
{
	None,

	// Draw possibilities
	DealerDraw, // Equivalent to round start.
	Draw,
	Call,

	// Turn possibilities
	Discard,
	Riichi, // also a discard
	HandKan,

	// Round end possibilities
	Tsumo,
	Ron,
	WallDepleted,

	Error,
};

//------------------------------------------------------------------------------
inline constexpr char const* ToString( TableEventType i_type )
{
	constexpr char const* strs[] =
	{
		"None",

		"DealerDraw",
		"Draw",
		"Call",
		"Discard",
		"Riichi",
		"HandKan",
		"Tsumo",
		"Ron",
		"WallDepleted",

		"Error",
	};
	return strs[ ( size_t )i_type ];
}

//------------------------------------------------------------------------------
namespace TableEvents
{

//------------------------------------------------------------------------------
class Draw
{
	TileDraw m_tileDrawn;
	Seat m_player;
public:
	explicit Draw( TileDraw const& i_tileDrawn, Seat i_player )
		: m_tileDrawn{ i_tileDrawn }
		, m_player{ i_player }
	{}

	TileDraw const& TileDrawn() const { return m_tileDrawn; }
	Seat Player() const { return m_player; }
};
using DealerDraw = Draw;

//------------------------------------------------------------------------------
enum class CallType : EnumValueType
{
	Pon,
	Chi,
	Kan,
};

//------------------------------------------------------------------------------
class Call
{
	CallType m_callType;
	Tile m_calledTile;
	Seat m_takenFrom;
public:
	explicit Call( CallType i_callType, Tile const& i_calledTile, Seat i_takenFrom )
		: m_callType{ i_callType }
		, m_calledTile{ i_calledTile }
		, m_takenFrom{ i_takenFrom }
	{}

	CallType GetCallType() const { return m_callType; }
	Tile const& CalledTile() const { return m_calledTile; }
	Seat TakenFrom() const { return m_takenFrom; }
};

//------------------------------------------------------------------------------
class Discard
{
	Tile m_tileDiscarded;
	Seat m_player;
public:
	explicit Discard( Tile const& i_tileDiscarded, Seat i_player )
		: m_tileDiscarded{ i_tileDiscarded }
		, m_player{ i_player }
	{}

	Tile const& TileDiscarded() const { return m_tileDiscarded; }
	Seat Player() const { return m_player; }
};
using Riichi = Discard;

//------------------------------------------------------------------------------
class HandKan
{
	Tile m_kanTile;
	bool m_closed;
public:
	explicit HandKan( Tile const& i_kanTile, bool i_closed )
		: m_kanTile{ i_kanTile }
		, m_closed{ i_closed }
	{}

	Tile const& KanTile() const { return m_kanTile; }
	bool Closed() const { return m_closed; }
};

//------------------------------------------------------------------------------
class Tsumo
{
	Tile m_winningTile;
	Seat m_winner;
public:
	explicit Tsumo( Tile const& i_winningTile, Seat i_winner )
		: m_winningTile{ i_winningTile }
		, m_winner{ i_winner }
	{}

	Tile const& WinningTile() const { return m_winningTile; }
	Seat Winner() const { return m_winner; }
};

//------------------------------------------------------------------------------
class Ron
{
	Tile m_winningTile;
	SeatSet m_winners;
	Seat m_loser;
public:
	explicit Ron( Tile const& i_winningTile, SeatSet i_winners, Seat i_loser )
		: m_winningTile{ i_winningTile }
		, m_winners{ std::move( i_winners ) }
	{}

	Tile const& WinningTile() const { return m_winningTile; }
	SeatSet const& Winners() const { return m_winners; }
	Seat Loser() const { return m_loser; }
};

//------------------------------------------------------------------------------
class WallDepleted
{
	SeatSet m_inTenpai;
public:
	explicit WallDepleted( SeatSet i_inTenpai )
		: m_inTenpai{ i_inTenpai }
	{}

	SeatSet const& InTenpai() const { return m_inTenpai; }
};

}

//------------------------------------------------------------------------------
using TableEvent = NamedUnion<
	TableEventType,

	Utils::NullType, // None

	TableEvents::DealerDraw,
	TableEvents::Draw,
	TableEvents::Call,
	TableEvents::Discard,
	TableEvents::Riichi,
	TableEvents::HandKan,
	TableEvents::Tsumo,
	TableEvents::Ron,
	TableEvents::WallDepleted,

	std::string // Error
>;

}