#pragma once

#include "Declare.hpp"
#include "PlayerCount.hpp"
#include "Utils.hpp"

namespace Riichi
{

//------------------------------------------------------------------------------
struct Rules
{
	virtual ~Rules() = default;

	virtual PlayerCount GetPlayerCount() const = 0;
	virtual Points InitialPoints() const = 0;
};

//------------------------------------------------------------------------------
class StandardYonma : public Rules
{
public:
	PlayerCount GetPlayerCount() const override { return 4_Players; }
	Points InitialPoints() const override { return 25'000; }
};

}
