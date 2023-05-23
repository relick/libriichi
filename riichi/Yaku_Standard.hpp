#pragma once

#include "Yaku.hpp"

//------------------------------------------------------------------------------
// Standard Riichi hands
//------------------------------------------------------------------------------
namespace Riichi::StandardYaku
{

//------------------------------------------------------------------------------
// Win by self-draw with closed hand
//------------------------------------------------------------------------------
DECLARE_YAKU( MenzenchinTsumohou );

//------------------------------------------------------------------------------
// Win after calling riichi
//------------------------------------------------------------------------------
DECLARE_YAKU( Riichi );

//------------------------------------------------------------------------------
// Win after calling riichi before your next discard and before any calls
//------------------------------------------------------------------------------
DECLARE_YAKU( Ippatsu );

//------------------------------------------------------------------------------
// Win on a ryanmen wait with a closed hand and no fu
//------------------------------------------------------------------------------
DECLARE_YAKU( Pinfu );

//------------------------------------------------------------------------------
// Closed hand containing two identical sequences
//------------------------------------------------------------------------------
BEGIN_YAKU( Iipeikou )
	static bool IsMatchingSequence( HandGroup const& i_a, HandGroup const& i_b );
END_YAKU();

//------------------------------------------------------------------------------
// Win with tsumo on final tile from the wall
//------------------------------------------------------------------------------
DECLARE_YAKU( HaiteiRaoyue );

//------------------------------------------------------------------------------
// Win with ron on final discard
//------------------------------------------------------------------------------
DECLARE_YAKU( HouteiRaoyui );

//------------------------------------------------------------------------------
// Win on dead wall draw
//------------------------------------------------------------------------------
DECLARE_YAKU( RinshanKaihou );

//------------------------------------------------------------------------------
// Win on an upgraded kan tile
//------------------------------------------------------------------------------
DECLARE_YAKU( Chankan );

//------------------------------------------------------------------------------
// Only 2-8 tiles in hand
//------------------------------------------------------------------------------
template<bool t_KuitanEnabled = true>
BEGIN_YAKU( Tanyao )
	static bool InvalidTile( Tile const& i_tile );
END_YAKU();

//------------------------------------------------------------------------------
// There are 5 yakuhai, each with their own name
// It's easier if we implement it in as few repetitions as possible
//------------------------------------------------------------------------------
template<YakuNameString t_YakuhaiName, DragonTileType t_DragonType>
struct DragonYakuhai
	: public NamedYaku<t_YakuhaiName>
{
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

//------------------------------------------------------------------------------
// Need a triple of haku
//------------------------------------------------------------------------------
struct Yakuhai_Haku
	: DragonYakuhai<"Haku", DragonTileType::White>
{};

//------------------------------------------------------------------------------
// Need a triple of hatsu
//------------------------------------------------------------------------------
struct Yakuhai_Hatsu
	: DragonYakuhai<"Hatsu", DragonTileType::Green>
{};

//------------------------------------------------------------------------------
// Need a triple of chun
//------------------------------------------------------------------------------
struct Yakuhai_Chun
	: DragonYakuhai<"Chun", DragonTileType::Red>
{};

//------------------------------------------------------------------------------
// Need a triple of the round wind
//------------------------------------------------------------------------------
BEGIN_YAKU( Bakaze )
	static bool ValidTile( Tile const& i_tile, Seat const& i_roundWind );
END_YAKU();

//------------------------------------------------------------------------------
// Need a triple of the seat wind
//------------------------------------------------------------------------------
BEGIN_YAKU( Jikaze )
	static bool ValidTile( Tile const& i_tile, Seat const& i_playerSeat );
END_YAKU();

//------------------------------------------------------------------------------
// Win after calling riichi on the very first discard
//------------------------------------------------------------------------------
DECLARE_YAKU( DoubleRiichi );

//------------------------------------------------------------------------------
// All groups must contain a terminal or honour
//------------------------------------------------------------------------------
BEGIN_YAKU( Chantaiyao )
	static bool RequiredTile( Tile const& i_tile );
END_YAKU();

//------------------------------------------------------------------------------
// Three sequences with the same numbers but different suits.
// Openness affects value
//------------------------------------------------------------------------------
BEGIN_YAKU( SanshokuDoujun )
	static bool Sanshoku( HandGroup const& i_a, HandGroup const& i_b, HandGroup const& i_c );
END_YAKU();

//------------------------------------------------------------------------------
// Usually called 'ittsuu'
// One suit, three sequences, 123 456 789
// Openness affects value
//------------------------------------------------------------------------------
DECLARE_YAKU( Ikkitsuukan );

//------------------------------------------------------------------------------
// All non-pair groups must be triplets/quads
//------------------------------------------------------------------------------
DECLARE_YAKU( Toitoi );

//------------------------------------------------------------------------------
// Any 3 concealed triplets or quads in hand
//------------------------------------------------------------------------------
DECLARE_YAKU( Sanankou );

//------------------------------------------------------------------------------
// Three triplets with the same number but different suits.
//------------------------------------------------------------------------------
BEGIN_YAKU( SanshokuDoukou )
	static bool Sanshoku( HandGroup const& i_a, HandGroup const& i_b, HandGroup const& i_c );
END_YAKU();

//------------------------------------------------------------------------------
// Any 3 quads in hand
//------------------------------------------------------------------------------
DECLARE_YAKU( Sankantsu );

//------------------------------------------------------------------------------
// Hand consists of 7 unique pairs
// Must be closed, by nature of the hand structure
// TODO-MVP: The main hand interpretations algorithm will fail to recognise 7 pairs
//------------------------------------------------------------------------------
BEGIN_YAKU( Chiitoitsu )
	static bool ValidTile( Tile const& i_tile );
END_YAKU();

//------------------------------------------------------------------------------
// All tiles are terminals or honours
//------------------------------------------------------------------------------
BEGIN_YAKU( Honroutou )
	static bool ValidTile( Tile const& i_tile );
END_YAKU();

//------------------------------------------------------------------------------
// smol 3 dragons
// triplets/quads of 2 dragons required + pair of 3rd dragon
//------------------------------------------------------------------------------
DECLARE_YAKU( Shousangen );

//------------------------------------------------------------------------------
// All tiles belong to exactly one of the three suits, or are honour tiles
//------------------------------------------------------------------------------
DECLARE_YAKU( Honitsu );

//------------------------------------------------------------------------------
// All groups must contain a terminal
//------------------------------------------------------------------------------
BEGIN_YAKU( JunchanTaiyao )
	static bool RequiredTile( Tile const& i_tile );
END_YAKU();

//------------------------------------------------------------------------------
// Closed hand containing two sets of two identical sequences
//------------------------------------------------------------------------------
BEGIN_YAKU( Ryanpeikou )
	static bool IsMatchingSequence( HandGroup const& i_a, HandGroup const& i_b );
END_YAKU();

//------------------------------------------------------------------------------
// All tiles belong to exactly one of the three suits
//------------------------------------------------------------------------------
DECLARE_YAKU( Chinitsu );

//------------------------------------------------------------------------------
// THIRTEEN ORPHANS
// 19m19s19p1234567z + any terminal/honor, closed only
// TODO-MVP: The main hand interpretations algorithm will fail to recognise 13 orphans
//------------------------------------------------------------------------------
BEGIN_YAKU( KokushiMusou )
	static bool RequiredTile( Tile const& i_tile );
END_YAKU();

//------------------------------------------------------------------------------
// Any 4 concealed triplets or quads in hand
//------------------------------------------------------------------------------
DECLARE_YAKU( Suuankou );

//------------------------------------------------------------------------------
// beeg 3 dragons
// triplets/quads of all 3 dragons required
//------------------------------------------------------------------------------
DECLARE_YAKU( Daisangen );

//------------------------------------------------------------------------------
// small 4 winds
// triplets/quads of 3 winds required + pair of 4th wind
//------------------------------------------------------------------------------
DECLARE_YAKU( Shousuushii );

//------------------------------------------------------------------------------
// big 4 winds
// triplets/quads of all 4 winds required
//------------------------------------------------------------------------------
DECLARE_YAKU( Daisuushii );

//------------------------------------------------------------------------------
// all honors
//------------------------------------------------------------------------------
DECLARE_YAKU( Tsuuiisou );

//------------------------------------------------------------------------------
// all terminals
//------------------------------------------------------------------------------
BEGIN_YAKU( Chinroutou )
	static bool RequiredTile( Tile const& i_tile );
END_YAKU();

//------------------------------------------------------------------------------
// all green
// 23468s7z only
//------------------------------------------------------------------------------
BEGIN_YAKU( Ryuuiisou )
	static bool RequiredTile( Tile const& i_tile );
END_YAKU();

//------------------------------------------------------------------------------
// NINE GATES
// 1112345678999 + any, in same suit, closed only
//------------------------------------------------------------------------------
DECLARE_YAKU( ChuurenPoutou );

//------------------------------------------------------------------------------
// Any 4 quads in hand, can be open or closed
//------------------------------------------------------------------------------
DECLARE_YAKU( Suukantsu );

//------------------------------------------------------------------------------
// Win as the dealer on the first tile
//------------------------------------------------------------------------------
DECLARE_YAKU( Tenhou );

//------------------------------------------------------------------------------
// Win as a non-dealer on the first tile drawn
//------------------------------------------------------------------------------
DECLARE_YAKU( Chihou );

// TODO-MVP: Scoring options that aren't typical yaku
// Kazoe Yakuman
// Nagashi Mangan

}

#include "Yaku_Standard.inl"