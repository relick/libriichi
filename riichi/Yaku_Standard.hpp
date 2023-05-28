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
DECLARE_YAKU( MenzenchinTsumohou, Standard );

//------------------------------------------------------------------------------
// Win after calling riichi
//------------------------------------------------------------------------------
BEGIN_YAKU( Riichi, Standard )
	bool AddsYakuToRiichi() const override { return true; }
END_YAKU();

//------------------------------------------------------------------------------
// Win after calling riichi before your next discard and before any calls
//------------------------------------------------------------------------------
DECLARE_YAKU( Ippatsu, Standard );

//------------------------------------------------------------------------------
// Win on a ryanmen wait with a closed hand and no fu
//------------------------------------------------------------------------------
DECLARE_YAKU( Pinfu, Standard );

//------------------------------------------------------------------------------
// Closed hand containing two identical sequences
//------------------------------------------------------------------------------
BEGIN_YAKU( Iipeikou, Standard )
	static bool IsMatchingSequence( HandGroup const& i_a, HandGroup const& i_b );
END_YAKU();

//------------------------------------------------------------------------------
// Win with tsumo on final tile from the wall
//------------------------------------------------------------------------------
DECLARE_YAKU( HaiteiRaoyue, Standard );

//------------------------------------------------------------------------------
// Win with ron on final discard
//------------------------------------------------------------------------------
DECLARE_YAKU( HouteiRaoyui, Standard );

//------------------------------------------------------------------------------
// Win on dead wall draw
//------------------------------------------------------------------------------
DECLARE_YAKU( RinshanKaihou, Standard );

//------------------------------------------------------------------------------
// Win on an upgraded kan tile
//------------------------------------------------------------------------------
DECLARE_YAKU( Chankan, Standard );

//------------------------------------------------------------------------------
// Only 2-8 tiles in hand
//------------------------------------------------------------------------------
template<bool t_KuitanEnabled = true>
BEGIN_YAKU( Tanyao, Standard )
	static bool InvalidTile( Tile const& i_tile );
END_YAKU();

//------------------------------------------------------------------------------
// There are 5 yakuhai, each with their own name
// It's easier if we implement it in as few repetitions as possible
//------------------------------------------------------------------------------
template<NameString t_YakuhaiName, DragonTileType t_DragonType>
struct DragonYakuhai
	: public NamedYaku<t_YakuhaiName, "Standard">
{
	HanValue CalculateValue
	(
		Round const& i_round,
		Seat const& i_playerSeat,
		Hand const& i_hand,
		HandAssessment const& i_assessment,
		HandInterpretation const& i_interp,
		TileDraw const& i_lastTile
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
BEGIN_YAKU( Bakaze, Standard )
	static bool ValidTile( Tile const& i_tile, Seat const& i_roundWind );
END_YAKU();

//------------------------------------------------------------------------------
// Need a triple of the seat wind
//------------------------------------------------------------------------------
BEGIN_YAKU( Jikaze, Standard )
	static bool ValidTile( Tile const& i_tile, Seat const& i_playerSeat );
END_YAKU();

//------------------------------------------------------------------------------
// Win after calling riichi on the very first discard
//------------------------------------------------------------------------------
BEGIN_YAKU( DoubleRiichi, Standard )
	bool AddsYakuToRiichi() const override { return true; }
END_YAKU();

//------------------------------------------------------------------------------
// All groups must contain a terminal or honour
//------------------------------------------------------------------------------
BEGIN_YAKU( Chantaiyao, Standard )
	static bool RequiredTile( Tile const& i_tile );
END_YAKU();

//------------------------------------------------------------------------------
// Three sequences with the same numbers but different suits.
// Openness affects value
//------------------------------------------------------------------------------
BEGIN_YAKU( SanshokuDoujun, Standard )
	static bool Sanshoku( HandGroup const& i_a, HandGroup const& i_b, HandGroup const& i_c );
END_YAKU();

//------------------------------------------------------------------------------
// Usually called 'ittsuu'
// One suit, three sequences, 123 456 789
// Openness affects value
//------------------------------------------------------------------------------
DECLARE_YAKU( Ikkitsuukan, Standard );

//------------------------------------------------------------------------------
// All non-pair groups must be triplets/quads
//------------------------------------------------------------------------------
DECLARE_YAKU( Toitoi, Standard );

//------------------------------------------------------------------------------
// Any 3 concealed triplets or quads in hand
//------------------------------------------------------------------------------
DECLARE_YAKU( Sanankou, Standard );

//------------------------------------------------------------------------------
// Three triplets with the same number but different suits.
//------------------------------------------------------------------------------
BEGIN_YAKU( SanshokuDoukou, Standard )
	static bool Sanshoku( HandGroup const& i_a, HandGroup const& i_b, HandGroup const& i_c );
END_YAKU();

//------------------------------------------------------------------------------
// Any 3 quads in hand
//------------------------------------------------------------------------------
DECLARE_YAKU( Sankantsu, Standard );

//------------------------------------------------------------------------------
// Hand consists of 7 unique pairs
// Must be closed, by nature of the hand structure
//------------------------------------------------------------------------------
DECLARE_YAKU( Chiitoitsu, SevenPairs );

//------------------------------------------------------------------------------
// All tiles are terminals or honours
//------------------------------------------------------------------------------
BEGIN_YAKU( Honroutou, Standard )
	static bool ValidTile( Tile const& i_tile );
END_YAKU();

//------------------------------------------------------------------------------
// smol 3 dragons
// triplets/quads of 2 dragons required + pair of 3rd dragon
//------------------------------------------------------------------------------
DECLARE_YAKU( Shousangen, Standard );

//------------------------------------------------------------------------------
// All tiles belong to exactly one of the three suits, or are honour tiles
//------------------------------------------------------------------------------
DECLARE_YAKU( Honitsu, Standard );

//------------------------------------------------------------------------------
// All groups must contain a terminal
//------------------------------------------------------------------------------
BEGIN_YAKU( JunchanTaiyao, Standard )
	static bool RequiredTile( Tile const& i_tile );
END_YAKU();

//------------------------------------------------------------------------------
// Closed hand containing two sets of two identical sequences
//------------------------------------------------------------------------------
BEGIN_YAKU( Ryanpeikou, Standard )
	static bool IsMatchingSequence( HandGroup const& i_a, HandGroup const& i_b );
END_YAKU();

//------------------------------------------------------------------------------
// All tiles belong to exactly one of the three suits
//------------------------------------------------------------------------------
DECLARE_YAKU( Chinitsu, Standard );

//------------------------------------------------------------------------------
// THIRTEEN ORPHANS
// 19m19s19p1234567z + any terminal/honor, closed only
//------------------------------------------------------------------------------
BEGIN_YAKU( KokushiMusou, ThirteenOrphans )
	static bool RequiredTile( Tile const& i_tile );
END_YAKU();

//------------------------------------------------------------------------------
// Any 4 concealed triplets or quads in hand
//------------------------------------------------------------------------------
DECLARE_YAKU( Suuankou, Standard );

//------------------------------------------------------------------------------
// beeg 3 dragons
// triplets/quads of all 3 dragons required
//------------------------------------------------------------------------------
DECLARE_YAKU( Daisangen, Standard );

//------------------------------------------------------------------------------
// small 4 winds
// triplets/quads of 3 winds required + pair of 4th wind
//------------------------------------------------------------------------------
DECLARE_YAKU( Shousuushii, Standard );

//------------------------------------------------------------------------------
// big 4 winds
// triplets/quads of all 4 winds required
//------------------------------------------------------------------------------
DECLARE_YAKU( Daisuushii, Standard );

//------------------------------------------------------------------------------
// all honors
//------------------------------------------------------------------------------
DECLARE_YAKU( Tsuuiisou, Standard );

//------------------------------------------------------------------------------
// all terminals
//------------------------------------------------------------------------------
BEGIN_YAKU( Chinroutou, Standard )
	static bool RequiredTile( Tile const& i_tile );
END_YAKU();

//------------------------------------------------------------------------------
// all green
// 23468s7z only
//------------------------------------------------------------------------------
BEGIN_YAKU( Ryuuiisou, Standard )
	static bool RequiredTile( Tile const& i_tile );
END_YAKU();

//------------------------------------------------------------------------------
// NINE GATES
// 1112345678999 + any, in same suit, closed only
//------------------------------------------------------------------------------
DECLARE_YAKU( ChuurenPoutou, Standard );

//------------------------------------------------------------------------------
// Any 4 quads in hand, can be open or closed
//------------------------------------------------------------------------------
DECLARE_YAKU( Suukantsu, Standard );

//------------------------------------------------------------------------------
// Win as the dealer on the first tile
//------------------------------------------------------------------------------
DECLARE_YAKU( Tenhou, Standard );

//------------------------------------------------------------------------------
// Win as a non-dealer on the first tile drawn
//------------------------------------------------------------------------------
DECLARE_YAKU( Chihou, Standard );

// Kazoe Yakuman left up to game system to use or not

}

#include "Yaku_Standard.inl"