#pragma once

#include "Rules.hpp"
#include "Seat.hpp"

namespace Riichi
{

//------------------------------------------------------------------------------
class StandardYonmaCore
	: public Rules
{
	Vector<TileInstance> m_tileSet;
public:
	StandardYonmaCore();

	size_t GetPlayerCount() const override { return 4_Players; }
	Points InitialPoints() const override { return 25'000; }
	Points RiichiBetPoints() const override { return 1'000; }
	Vector<TileInstance> const& Tileset() const override { return m_tileSet; }
	size_t DeadWallDrawsAvailable() const override { return 4u; }
	bool HasPermissionToRiichi( Seat i_player, Points i_currentPoints ) const override { return i_currentPoints >= RiichiBetPoints(); }

	Pair<Set<TileKind>, Vector<TileInstance>> WaitsWithYaku
	(
		Round const& i_round,
		Seat const& i_playerSeat,
		Hand const& i_hand,
		TileDraw const& i_lastTile,
		bool i_considerForRiichi
	) const override;

	HandScore CalculateBasicPoints
	(
		Round const& i_round,
		Seat const& i_playerSeat,
		Hand const& i_hand,
		TileDraw const& i_lastTile
	) const override;

	bool NoMoreRoundsCore( Table const& i_table, Round const& i_previousRound, Seat i_gameLength ) const;
	bool RepeatRound( Round const& i_previousRound ) const override;
	bool ShouldAddHonba( Round const& i_previousRound ) const override;

	TablePayments HonbaPotPayments( size_t i_honbaSticks, SeatSet const& i_winners, Option<Seat> i_ronLoser ) const override;
	TablePayments RiichiBetPotPayments( size_t i_riichiSticks, SeatSet const& i_winners, Option<Seat> i_ronLoser ) const override;
	TablePayments TsumoPayments( HandScore const& i_handScore, Seat i_winner ) const override;
	TablePayments RonPayments( HandScore const& i_handScore, Seat i_winner, Seat i_loser ) const override;
	TablePayments ExhaustiveDrawPayments( SeatSet const& i_playersInTenpai ) const override;

private:
	static Points RoundTo100( Points i_finalPoints );
};

//------------------------------------------------------------------------------
template<Seat t_GameLength>
class StandardYonma
	: public StandardYonmaCore
{
public:
	using StandardYonmaCore::StandardYonmaCore;

	bool NoMoreRounds( Table const& i_table, Round const& i_previousRound ) const override;
};

}

#include "Rules_Standard.inl"
