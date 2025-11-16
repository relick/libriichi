#pragma once

#include "Base.hpp"

#include <cassert>

namespace Riichi::Utils
{
//------------------------------------------------------------------------------
#if NDEBUG
#define riEnsure(TEST, MSG)
#define riError(MSG)
#else
struct ConstexprCompatibleAssert
{
	template<typename F> explicit ConstexprCompatibleAssert( F f ) { f(); }
};

//#define riEnsure(TEST, MSG) assert((MSG, TEST))
#define riEnsure(TEST, MSG) { if ( !( TEST ) ) { throw ::Riichi::Utils::ConstexprCompatibleAssert( [&] { assert( ( MSG, TEST ) ); } ); } }
#define riError(MSG) riEnsure(false, MSG)
#endif

}
