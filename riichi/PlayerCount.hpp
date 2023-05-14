#pragma once

#include "Utils.hpp"

namespace Riichi
{

using PlayerCount = Utils::RestrictedIntegral<size_t, 1, 4, 4>;

}

constexpr Riichi::PlayerCount operator""_Players( unsigned long long int n )
{
	return Riichi::PlayerCount{n};
}
