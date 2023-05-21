#pragma once

#include <random>

namespace Riichi
{

//-----------------------------------------------------------------------------
// Class for random numbers that:
// - allows for serialisation that avoids the ostream nonsense
// - encapsulates engine choice from usage
// - separates the types of engines used for different purposes to prevent misuse
//-----------------------------------------------------------------------------
template<typename T_Disambiguator, typename T_EngineType = std::mt19937>
class RandomEngine
{
public:
	using result_type = typename T_EngineType::result_type;

private:
	result_type m_initialSeed;
	T_EngineType m_engine;
	unsigned long long m_advanceCount{ 0 };

public:
	explicit RandomEngine( result_type i_seed )
		: m_initialSeed{ i_seed }
		, m_engine{ i_seed }
	{}

	result_type operator()()
	{
		++m_advanceCount;
		return m_engine();
	}

	void discard( unsigned long long z )
	{
		m_advanceCount += z;
		m_engine.discard( z );
	}

	static constexpr result_type min() { return T_EngineType::min(); }
	static constexpr result_type max() { return T_EngineType::max(); }

	friend bool operator==( RandomEngine const& i_a, RandomEngine const& i_b ) = default;
};

//-----------------------------------------------------------------------------
using ShuffleRNG = RandomEngine<struct ShuffleRNGType>;
using AIRNG = RandomEngine<struct AIRNGType>;

}