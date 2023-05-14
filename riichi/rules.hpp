#pragma once

#include "playercount.hpp"
#include "utils.hpp"
#include "yaku.hpp"

namespace Riichi
{

using Points = unsigned int;

template<PlayerCount t_PlayerCount>
struct Rules
{
	virtual ~Rules() = default;

	static size_t PlayerCount() { return t_PlayerCount; }

	virtual Points InitialPoints() const = 0;
};

class StandardYonma : public Rules<4_Players>
{
public:
	Points InitialPoints() const override { return 25'000; }
};

}
