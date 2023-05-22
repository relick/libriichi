#pragma once

#include "Containers.hpp"
#include "Declare.hpp"
#include "PlayerCount.hpp"
#include "Tile.hpp"

namespace Riichi
{

//------------------------------------------------------------------------------
struct Rules
{
	virtual ~Rules() = default;

	virtual PlayerCount GetPlayerCount() const = 0;
	virtual Points InitialPoints() const = 0;
	virtual Vector<Tile> const& Tileset() const = 0;
	// Not sure there's a reason to vary this, but it's a nice place to put it
	// Dead wall size derived from this, and kan call maximum is treated as equal
	virtual size_t DeadWallDrawsAvailable() const = 0;
	size_t DeadWallSize() const { return  ( 1 + DeadWallDrawsAvailable() ) * 2 + DeadWallDrawsAvailable(); }
};

//------------------------------------------------------------------------------
class StandardYonma : public Rules
{
	Vector<Tile> m_tileSet;
public:
	StandardYonma();

	PlayerCount GetPlayerCount() const override { return 4_Players; }
	Points InitialPoints() const override { return 25'000; }
	virtual Vector<Tile> const& Tileset() const { return m_tileSet; }
	size_t DeadWallDrawsAvailable() const { return 4u; }
};

}
