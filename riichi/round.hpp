#pragma once

#include "seat.hpp"

namespace Riichi
{

class Round
{
public:
	Seat Wind() const { return Seat::East; }
};

}