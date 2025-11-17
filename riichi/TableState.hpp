#pragma once

#include "Declare.hpp"
#include "Hand.hpp"
#include "NamedUnion.hpp"
#include "PlayerCount.hpp"
#include "Seat.hpp"
#include "TableEvent.hpp"
#include "Tile.hpp"
#include "AI.hpp"

namespace Riichi
{

//------------------------------------------------------------------------------
enum class TableStateType : EnumValueType
{
	// Game not yet begun
	Setup,

	// No round active
	BetweenRounds,
	GameOver,

	// Round active
	Turn_AI,
	Turn_User,
	BetweenTurns,
	RonAKanChance, // It's kinda amusing to me that this is a special state, but it makes sense
};

//------------------------------------------------------------------------------
inline constexpr char const* ToString( TableStateType i_type )
{
	constexpr char const* strs[] =
	{
		"Setup",
		"BetweenRounds",
		"GameOver",
		"Turn_AI",
		"Turn_User",
		"BetweenTurns",
		"RonAKanChance",
	};
	return strs[ ( size_t )i_type ];
}

//------------------------------------------------------------------------------
namespace TableStates
{

//------------------------------------------------------------------------------
struct Base
{
	Base( Table& i_table ) : m_table{ i_table } {}
protected:
	std::reference_wrapper<Table> m_table;
};

//------------------------------------------------------------------------------
// Common function for transitioning to a turn
//------------------------------------------------------------------------------
struct BetweenTurnsBase
	: Base
{
	using Base::Base;

protected:
	void TransitionToTurn( Option<TileDraw> const& i_tileDraw, TableEvent&& i_tableEvent ) const;
	void HandleRon( SeatSet const& i_winners, TileDraw const& i_tileDraw ) const;
};

//------------------------------------------------------------------------------
struct Setup
	: Base
{
	using Base::Base;

	void StartGame() const;
};

//------------------------------------------------------------------------------
struct BetweenRounds
	: BetweenTurnsBase
{
	using BetweenTurnsBase::BetweenTurnsBase;

	void StartRound() const;
};

//------------------------------------------------------------------------------
struct GameOver
	: Base
{
	using Base::Base;

	// Nothing. Game over.
};

//------------------------------------------------------------------------------
struct BaseTurn
	: Base
{
	BaseTurn( Table& i_table, Seat i_seat, bool i_canTsumo, Vector<TileInstance> i_riichiDiscards, bool i_isRiichi, Vector<HandKanOption> i_kanOptions );

	Hand const& GetCurrentHand() const;
	Option<TileInstance> GetCurrentTileDraw() const;
	Seat GetSeat() const { return m_seat; }

	bool CanTsumo() const { return m_canTsumo; }
	bool CanRiichi() const { return !m_riichiDiscards.empty(); }
	Vector<TileInstance> const& RiichiOptions() const { return m_riichiDiscards; }
	bool IsRiichi() const { return m_isRiichi; } // if true, only valid options are tsumo and discard
	bool CanKan() const { return !m_kanOptions.empty(); }
	Vector<HandKanOption> const& KanOptions() const { return m_kanOptions; }

protected:
	void TransitionToBetweenTurns( TileInstance const& i_discardedTile, TableEvent&& i_tableEvent ) const;

	void Tsumo() const;
	void Discard( Option<TileInstance> const& i_handTileToDiscard ) const; // nullopt will discard drawn tile
	void Riichi( Option<TileInstance> const& i_handTileToDiscard ) const; // nullopt will discard drawn tile
	void Kan( HandKanOption const& i_kanOption ) const;

private:
	Seat m_seat;

	bool m_canTsumo;
	Vector<TileInstance> m_riichiDiscards;
	bool m_isRiichi;
	Vector<HandKanOption> m_kanOptions;
};

//------------------------------------------------------------------------------
struct Turn_AI
	: BaseTurn
{
	using BaseTurn::BaseTurn;

	void MakeDecision() const;

protected:
	AI::DecisionToken m_token;
};

//------------------------------------------------------------------------------
struct Turn_User
	: BaseTurn
{
	using BaseTurn::BaseTurn;

	void Tsumo() const;
	void Discard( Option<TileInstance> const& i_handTileToDiscard ) const; // nullopt will discard drawn tile
	void Riichi( Option<TileInstance> const& i_handTileToDiscard ) const; // nullopt will discard drawn tile
	void Kan( HandKanOption const& i_kanOption ) const; // Will meld the 4 matching tiles if a closed kan
};

//------------------------------------------------------------------------------
struct BetweenTurns
	: BetweenTurnsBase
{
	using ChiOptionData = Utils::EnumArray<Vector<ChiOption>, Seats>;
	using PonOptionData = Utils::EnumArray<Vector<PonOption>, Seats>;
	using KanOptionData = Utils::EnumArray<Vector<KanOption>, Seats>;
	using RonOptionData = SeatSet;

	BetweenTurns( Table& i_table, TileDraw i_discardedTile, ChiOptionData i_canChi, PonOptionData i_canPon, KanOptionData i_canKan, RonOptionData i_canRon );

	// TODO-AI indication about AI intent (to allow AI to jump in before user, depending on game implementation)

	ChiOptionData const& CanChi() const { return m_canChi; }
	PonOptionData const& CanPon() const { return m_canPon; }
	KanOptionData const& CanKan() const { return m_canKan; }
	SeatSet const& CanRon() const { return m_canRon; }

	// If any calls are made by AI, they will only happen in UserPass or UserRon
	void UserPass() const;
	void UserChi( Seat i_user, ChiOption const& i_option ) const;
	void UserPon( Seat i_user, PonOption const& i_option ) const;
	void UserKan( Seat i_user, KanOption const& i_option ) const;
	void UserRon( SeatSet const& i_users ) const;

private:
	TileDraw m_discardedTile;
	ChiOptionData m_canChi;
	PonOptionData m_canPon;
	KanOptionData m_canKan;
	RonOptionData m_canRon;

	Utils::EnumArray<AI::BetweenTurnsDecisionData, Seats> m_aiDecisions;
};

//------------------------------------------------------------------------------
struct RonAKanChance
	: BetweenTurnsBase
{
	RonAKanChance( Table& i_table, TileDraw i_kanTile, SeatSet i_canRon );

	SeatSet const& CanRon() const { return m_canRon; }
	
	void Pass() const;
	void Ron( SeatSet const& i_players ) const;

private:
	TileDraw m_kanTile;
	SeatSet m_canRon;
};

}

//------------------------------------------------------------------------------
using TableState = NamedUnion<
	TableStateType,

	TableStates::Setup,
	TableStates::BetweenRounds,
	TableStates::GameOver,
	TableStates::Turn_AI,
	TableStates::Turn_User,
	TableStates::BetweenTurns,
	TableStates::RonAKanChance
>;

}
