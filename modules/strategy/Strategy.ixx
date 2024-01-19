module;
#pragma once
#pragma once
#ifdef ATLAS_EXPORTS
#define ATLAS_API __declspec(dllexport)
#else
#define ATLAS_API  __declspec(dllimport)
#endif
export module StrategyModule;

import AtlasCore;

namespace Atlas
{


//============================================================================
class StrategyImpl;


//============================================================================
export class Strategy
{
	friend class Exchange;
	friend class Hydra;
private:
	String m_name;
	size_t m_id = 0;
	bool m_step_call = false;
	UniquePtr<StrategyImpl> m_impl;

	void evaluate() noexcept;
	void step() noexcept;
	void reset() noexcept;
	void setID(size_t id) noexcept { m_id = id; }

public:
	ATLAS_API Strategy(
		String name,
		SharedPtr<AST::StrategyNode> ast,
		double portfolio_weight
	) noexcept;
	ATLAS_API ~Strategy() noexcept;
	ATLAS_API double getAllocation(size_t asset_index) const noexcept;
	ATLAS_API Tracer const& getTracer() const noexcept;
	ATLAS_API auto const& getName() const noexcept { return m_name; }
	ATLAS_API auto const& getId() const noexcept { return m_id; }
	ATLAS_API double getNLV() const noexcept;

	Exchange const& getExchange() const noexcept;

};


}

