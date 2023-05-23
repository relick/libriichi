#pragma once

#include "Base.hpp"

namespace Riichi
{

//------------------------------------------------------------------------------
// Aliases
//------------------------------------------------------------------------------
using Points = uint32_t;
using Han = uint8_t;

//------------------------------------------------------------------------------
// Hand
//------------------------------------------------------------------------------
enum class GroupType : EnumValueType;
struct Meld;
class Hand;
enum class WaitType : EnumValueType;
struct HandInterpretation;
class HandGroup;
struct HandAssessment;

//------------------------------------------------------------------------------
// Player
//------------------------------------------------------------------------------
enum class PlayerType : EnumValueType;
class Player;

//------------------------------------------------------------------------------
// PlayerCount
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Random
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// RoundData
//------------------------------------------------------------------------------
class RoundData;

//------------------------------------------------------------------------------
// Rules
//------------------------------------------------------------------------------
struct Rules;
class StandardYonma;

//------------------------------------------------------------------------------
// Seat
//------------------------------------------------------------------------------
enum class Seat : EnumValueType;

//------------------------------------------------------------------------------
// Table
//------------------------------------------------------------------------------
struct Standings;
class Table;

//------------------------------------------------------------------------------
// TableEvent
//------------------------------------------------------------------------------
enum class TableEventType : EnumValueType;
namespace TableEvents
{
class Draw;
enum class CallType : EnumValueType;
class Call;
class Discard;
class HandKan;
class Tsumo;
class Ron;
class WallDepleted;
}

//------------------------------------------------------------------------------
// TableState
//------------------------------------------------------------------------------
enum class TableStateType : EnumValueType;
namespace TableStates
{
struct Base;
struct Setup;
struct BetweenRounds;
struct GameOver;
struct Turn_AI;
struct Turn_Player;
struct BetweenTurns;
struct RonAKanChance;
}

//------------------------------------------------------------------------------
// Tile
//------------------------------------------------------------------------------
enum class TileType : EnumValueType;
enum class Suit : EnumValueType;
struct SuitTile;
enum class DragonTileType : EnumValueType;
enum class WindTileType : EnumValueType;
enum class TileDrawType : EnumValueType;

//------------------------------------------------------------------------------
// Utils
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Yaku
//------------------------------------------------------------------------------
class HanValue;
struct Yaku;
// Yaku themselves not forward declared

}