#pragma once

#include "Yaku.hpp"

//-----------------------------------------------------------------------------
// Standard Riichi hands
//-----------------------------------------------------------------------------
namespace Riichi::StandardYaku
{

// Overall TODO: Lots of the yaku need to assess the 'final group' separately from the rest of the groups. This is to avoid creating a container and doing a bunch of copies where unnecessary
// It would be nice to clean this up somehow. It would also be nice if all the yaku could be made to avoid allocating entirely when assessing!

//-----------------------------------------------------------------------------
struct MenzenchinTsumohou
	: public NamedYaku<"MenzenchinTsumohou">
{
	// Win by self-draw with closed hand

	HanValue CalculateValue
	(
		RoundData const& i_round,
		Seat const& i_playerSeat,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const final;
};

//-----------------------------------------------------------------------------
struct Riichi
	: public NamedYaku<"Riichi">
{
	// Win after calling riichi

	HanValue CalculateValue
	(
		RoundData const& i_round,
		Seat const& i_playerSeat,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const final;
};

//-----------------------------------------------------------------------------
struct Ippatsu
	: public NamedYaku<"Ippatsu">
{
	// Win after calling riichi before your next discard and before any calls

	HanValue CalculateValue
	(
		RoundData const& i_round,
		Seat const& i_playerSeat,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const final;
};

//-----------------------------------------------------------------------------
struct Pinfu
	: public NamedYaku<"Pinfu">
{
	// Win on a ryanmen wait with a closed hand and no fu

	HanValue CalculateValue
	(
		RoundData const& i_round,
		Seat const& i_playerSeat,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const final;
};

//-----------------------------------------------------------------------------
struct Iipeikou
	: public NamedYaku<"Iipeikou">
{
	// Closed hand containing two identical sequences

	HanValue CalculateValue
	(
		RoundData const& i_round,
		Seat const& i_playerSeat,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const final;

private:
	static bool IsMatchingSequence( HandGroup const& i_a, HandGroup const& i_b );
};

//-----------------------------------------------------------------------------
struct HaiteiRaoyue
	: public NamedYaku<"HaiteiRaoyue">
{
	// Win with tsumo on final tile from the wall

	HanValue CalculateValue
	(
		RoundData const& i_round,
		Seat const& i_playerSeat,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const final;
};

//-----------------------------------------------------------------------------
struct HouteiRaoyui
	: public NamedYaku<"HouteiRaoyui">
{
	// Win with ron on final discard

	HanValue CalculateValue
	(
		RoundData const& i_round,
		Seat const& i_playerSeat,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const final;
};

//-----------------------------------------------------------------------------
struct RinshanKaihou
	: public NamedYaku<"RinshanKaihou">
{
	// Win on dead wall draw

	HanValue CalculateValue
	(
		RoundData const& i_round,
		Seat const& i_playerSeat,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const final;
};

//-----------------------------------------------------------------------------
struct Chankan
	: public NamedYaku<"Chankan">
{
	// Win on an upgraded kan tile

	HanValue CalculateValue
	(
		RoundData const& i_round,
		Seat const& i_playerSeat,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const final;
};

//-----------------------------------------------------------------------------
template<bool t_KuitanEnabled = true>
struct Tanyao
	: public NamedYaku<"Tanyao">
{
	// Only 2-8 tiles in hand

	HanValue CalculateValue
	(
		RoundData const& i_round,
		Seat const& i_playerSeat,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const final;

private:
	static bool InvalidTile( Tile const& i_tile );
};

//-----------------------------------------------------------------------------
// There are 5 yakuhai, each with their own name
// It's easier if we implement it in as few repetitions as possible
//-----------------------------------------------------------------------------
template<YakuNameString t_YakuhaiName, DragonTileType t_DragonType>
struct DragonYakuhai
	: public NamedYaku<t_YakuhaiName>
{
	// Need a triple of the specified type

	HanValue CalculateValue
	(
		RoundData const& i_round,
		Seat const& i_playerSeat,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const final;

private:
	static bool ValidTile( Tile const& i_tile );
};

//-----------------------------------------------------------------------------
struct Yakuhai_Haku
	: DragonYakuhai<"Haku", DragonTileType::White>
{};

//-----------------------------------------------------------------------------
struct Yakuhai_Hatsu
	: DragonYakuhai<"Hatsu", DragonTileType::Green>
{};

//-----------------------------------------------------------------------------
struct Yakuhai_Chun
	: DragonYakuhai<"Chun", DragonTileType::Red>
{};

//-----------------------------------------------------------------------------
struct Yakuhai_RoundWind
	: NamedYaku<"Bakaze">
{
	// Need a triple of the round wind

	HanValue CalculateValue
	(
		RoundData const& i_round,
		Seat const& i_playerSeat,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const final;

private:
	static bool ValidTile( Tile const& i_tile, Seat const& i_roundWind );
};

//-----------------------------------------------------------------------------
struct Yakuhai_SeatWind
	: NamedYaku<"Jikaze">
{
	// Need a triple of the seat wind

	HanValue CalculateValue
	(
		RoundData const& i_round,
		Seat const& i_playerSeat,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const final;

private:
	static bool ValidTile( Tile const& i_tile, Seat const& i_playerSeat );
};

//-----------------------------------------------------------------------------
struct DoubleRiichi
	: public NamedYaku<"Riichi">
{
	// Win after calling riichi on the very first discard

	HanValue CalculateValue
	(
		RoundData const& i_round,
		Seat const& i_playerSeat,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const final;
};

//-----------------------------------------------------------------------------
struct Chantaiyao
	: public NamedYaku<"Chantaiyao">
{
	// All groups must contain a terminal or honour

	HanValue CalculateValue
	(
		RoundData const& i_round,
		Seat const& i_playerSeat,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const final;

private:
	static bool RequiredTile( Tile const& i_tile );
};

//-----------------------------------------------------------------------------
struct SanshokuDoujun
	: public NamedYaku<"SanshokuDoujun">
{
	// Three sequences with the same numbers but different suits.
	// Openness affects value

	HanValue CalculateValue
	(
		RoundData const& i_round,
		Seat const& i_playerSeat,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const final;

private:
	static bool Sanshoku( HandGroup const& i_a, HandGroup const& i_b, HandGroup const& i_c );
};

//-----------------------------------------------------------------------------
struct Ikkitsuukan
	: public NamedYaku<"Ikkitsuukan">
{
	// Usually called 'ittsuu'
	// One suit, three sequences, 123 456 789
	// Openness affects value

	HanValue CalculateValue
	(
		RoundData const& i_round,
		Seat const& i_playerSeat,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const final;
};

//-----------------------------------------------------------------------------
struct Toitoi
	: public NamedYaku<"Toitoi">
{
	// All non-pair groups must be triplets/quads

	HanValue CalculateValue
	(
		RoundData const& i_round,
		Seat const& i_playerSeat,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const final;
};

//-----------------------------------------------------------------------------
struct Sanankou
	: public NamedYaku<"Sanankou">
{
	// Any 3 concealed triplets or quads in hand

	HanValue CalculateValue
	(
		RoundData const& i_round,
		Seat const& i_playerSeat,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const final;
};

//-----------------------------------------------------------------------------
struct SanshokuDoukou
	: public NamedYaku<"SanshokuDoukou">
{
	// Three triplets with the same number but different suits.

	HanValue CalculateValue
	(
		RoundData const& i_round,
		Seat const& i_playerSeat,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const final;

private:
	static bool Sanshoku( HandGroup const& i_a, HandGroup const& i_b, HandGroup const& i_c );
};

//-----------------------------------------------------------------------------
struct Sankantsu
	: public NamedYaku<"Sankantsu">
{
	// Any 3 quads in hand

	HanValue CalculateValue
	(
		RoundData const& i_round,
		Seat const& i_playerSeat,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const final;
};

//-----------------------------------------------------------------------------
// TODO: The main and interpretations algorithm will fail to recognise 7 pairs
//-----------------------------------------------------------------------------
struct Chiitoitsu
	: public NamedYaku<"Chiitoitsu">
{
	// Hand consists of 7 unique pairs
	// Must be closed, by nature of the hand structure

	HanValue CalculateValue
	(
		RoundData const& i_round,
		Seat const& i_playerSeat,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const final;

private:
	static bool ValidTile( Tile const& i_tile );
};

//-----------------------------------------------------------------------------
struct Honroutou
	: public NamedYaku<"Honroutou">
{
	// All tiles are terminals or honours

	HanValue CalculateValue
	(
		RoundData const& i_round,
		Seat const& i_playerSeat,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const final;

private:
	static bool ValidTile( Tile const& i_tile );
};

//-----------------------------------------------------------------------------
struct Shousangen
	: public NamedYaku<"Shousangen">
{
	// smol 3 dragons
	// triplets/quads of 2 dragons required + pair of 3rd dragon

	HanValue CalculateValue
	(
		RoundData const& i_round,
		Seat const& i_playerSeat,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const final;
};

//-----------------------------------------------------------------------------
struct Honitsu
	: public NamedYaku<"Honitsu">
{
	// All tiles belong to exactly one of the three suits, or are honour tiles

	HanValue CalculateValue
	(
		RoundData const& i_round,
		Seat const& i_playerSeat,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const final;
};

//-----------------------------------------------------------------------------
struct JunchanTaiyao
	: public NamedYaku<"JunchanTaiyao">
{
	// All groups must contain a terminal

	HanValue CalculateValue
	(
		RoundData const& i_round,
		Seat const& i_playerSeat,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const final;

private:
	static bool RequiredTile( Tile const& i_tile );
};

//-----------------------------------------------------------------------------
struct Ryanpeikou
	: public NamedYaku<"Ryanpeikou">
{
	// Closed hand containing two sets of two identical sequences

	HanValue CalculateValue
	(
		RoundData const& i_round,
		Seat const& i_playerSeat,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const final;

private:
	static bool IsMatchingSequence( HandGroup const& i_a, HandGroup const& i_b );
};

//-----------------------------------------------------------------------------
struct Chinitsu
	: public NamedYaku<"Chinitsu">
{
	// All tiles belong to exactly one of the three suits

	HanValue CalculateValue
	(
		RoundData const& i_round,
		Seat const& i_playerSeat,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const final;
};

//-----------------------------------------------------------------------------
// TODO: The main and interpretations algorithm will fail to recognise 13 orphans
//-----------------------------------------------------------------------------
struct KokushiMusou
	: public NamedYaku<"KokushiMusou">
{
	// THIRTEEN ORPHANS
	// 19m19s19p1234567z + any terminal/honor, closed only

	HanValue CalculateValue
	(
		RoundData const& i_round,
		Seat const& i_playerSeat,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const final;

private:
	static bool RequiredTile( Tile const& i_tile );
};

//-----------------------------------------------------------------------------
struct Suuankou
	: public NamedYaku<"Suuankou">
{
	// Any 4 concealed triplets or quads in hand

	HanValue CalculateValue
	(
		RoundData const& i_round,
		Seat const& i_playerSeat,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const final;
};

//-----------------------------------------------------------------------------
struct Daisangen
	: public NamedYaku<"Daisangen">
{
	// beeg 3 dragons
	// triplets/quads of all 3 dragons required

	HanValue CalculateValue
	(
		RoundData const& i_round,
		Seat const& i_playerSeat,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const final;
};

//-----------------------------------------------------------------------------
struct Shousuushii
	: public NamedYaku<"Shousuushii">
{
	// small 4 winds
	// triplets/quads of 3 winds required + pair of 4th wind

	HanValue CalculateValue
	(
		RoundData const& i_round,
		Seat const& i_playerSeat,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const final;
};

//-----------------------------------------------------------------------------
struct Daisuushii
	: public NamedYaku<"Daisuushii">
{
	// big 4 winds
	// triplets/quads of all 4 winds required

	HanValue CalculateValue
	(
		RoundData const& i_round,
		Seat const& i_playerSeat,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const final;
};

//-----------------------------------------------------------------------------
struct Tsuuiisou
	: public NamedYaku<"Tsuuiisou">
{
	// all honors

	HanValue CalculateValue
	(
		RoundData const& i_round,
		Seat const& i_playerSeat,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const final;
};

//-----------------------------------------------------------------------------
struct Chinroutou
	: public NamedYaku<"Chinroutou">
{
	// all terminals

	HanValue CalculateValue
	(
		RoundData const& i_round,
		Seat const& i_playerSeat,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const final;

private:
	static bool RequiredTile( Tile const& i_tile );
};

//-----------------------------------------------------------------------------
struct Ryuuiisou
	: public NamedYaku<"Ryuuiisou">
{
	// all green
	// 23468s7z only

	HanValue CalculateValue
	(
		RoundData const& i_round,
		Seat const& i_playerSeat,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const final;

private:
	static bool RequiredTile( Tile const& i_tile );
};

//-----------------------------------------------------------------------------
struct ChuurenPoutou
	: public NamedYaku<"ChuurenPoutou">
{
	// NINE GATES
	// 1112345678999 + any, in same suit, closed only

	HanValue CalculateValue
	(
		RoundData const& i_round,
		Seat const& i_playerSeat,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const final;
};

//-----------------------------------------------------------------------------
struct Suukantsu
	: public NamedYaku<"Suukantsu">
{
	// Any 4 quads in hand, can be open or closed

	HanValue CalculateValue
	(
		RoundData const& i_round,
		Seat const& i_playerSeat,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const final;
};

//-----------------------------------------------------------------------------
struct Tenhou
	: public NamedYaku<"Tenhou">
{
	// Win as the dealer on the first tile

	HanValue CalculateValue
	(
		RoundData const& i_round,
		Seat const& i_playerSeat,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const final;
};

//-----------------------------------------------------------------------------
struct Chihou
	: public NamedYaku<"Chihou">
{
	// Win as a non-dealer on the first tile drawn

	HanValue CalculateValue
	(
		RoundData const& i_round,
		Seat const& i_playerSeat,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		Tile const& i_nextTile,
		TileDrawType i_nextTileType
	) const final;
};

// TODO: Scoring options that aren't typical yaku
// Kazoe Yakuman
// Nagashi Mangan

}

#include "Yaku_Standard.inl"