#pragma once

#include "playercount.hpp"
#include "utils.hpp"
#include "yaku.hpp"

namespace Riichi
{

using Points = unsigned int;

struct Rules
{
	virtual ~Rules() = default;

	virtual PlayerCount GetPlayerCount() const = 0;
	virtual Points InitialPoints() const = 0;
};

class StandardYonma : public Rules
{
public:
	PlayerCount GetPlayerCount() const override { return 4_Players; }
	Points InitialPoints() const override { return 25'000; }
};

}
