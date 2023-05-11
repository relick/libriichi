#pragma once

#include "utils.hpp"
#include "yaku.hpp"

namespace Riichi
{

using PlayerCount = Utils::RestrictedIntegral<size_t, 1, 4, 4>;

using Points = unsigned int;

}

constexpr Riichi::PlayerCount operator""_Players(unsigned long long int n)
{
	return Riichi::PlayerCount{n};
}

namespace Riichi
{

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
