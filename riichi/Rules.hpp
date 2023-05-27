#pragma once

#include "Containers.hpp"
#include "Declare.hpp"
#include "HandInterpreter.hpp"
#include "PlayerCount.hpp"
#include "Tile.hpp"
#include "Yaku.hpp"

#include <memory>

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
	size_t DeadWallSize() const { return ( 1 + DeadWallDrawsAvailable() ) * 2 + DeadWallDrawsAvailable(); }

	template<typename T_Visitor>
	void VisitInterpreters( T_Visitor&& i_visitor ) const;

	template<typename T_Visitor>
	void VisitYaku( T_Visitor&& i_visitor ) const;

protected:
	Vector<std::unique_ptr<HandInterpreter>> m_interpreters;
	Vector<std::unique_ptr<Yaku>> m_yaku;
};

}

#include "Rules.inl"
