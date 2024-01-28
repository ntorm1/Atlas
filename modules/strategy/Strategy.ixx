module;
#pragma once
#pragma once
#ifdef ATLAS_EXPORTS
#define ATLAS_API __declspec(dllexport)
#else
#define ATLAS_API  __declspec(dllimport)
#endif
#include <Eigen/Dense>
export module StrategyModule;

import AtlasCore;
import AtlasEnumsModule;
import PyNodeWrapperModule;

namespace Atlas
{


//============================================================================
class StrategyImpl;


//============================================================================
export class Strategy
{
	friend class AST::StrategyNode;
	friend class Exchange;
	friend class Hydra;
	friend class CommisionManager;
private:
	String m_name;
	size_t m_id = 0;
	bool m_step_call = false;
	bool m_late_rebalance_call = false;
	UniquePtr<StrategyImpl> m_impl;

	void evaluate() noexcept;
	void step() noexcept;
	void reset() noexcept;
	void lateRebalance() noexcept;
	void setNlv(double nlv_new) noexcept;
	void setID(size_t id) noexcept { m_id = id; }

public:
	ATLAS_API Strategy(
		String name,
		SharedPtr<AST::StrategyNode> ast,
		double portfolio_weight
	) noexcept;
	ATLAS_API Strategy(
		String name,
		AST::PyNodeWrapper<AST::StrategyNode> ast,
		double portfolio_weight
	) noexcept;

	ATLAS_API ~Strategy() noexcept;
	ATLAS_API Eigen::VectorXd const& getAllocationBuffer() const noexcept;
	ATLAS_API double getAllocation(size_t asset_index) const noexcept;
	ATLAS_API Tracer const& getTracer() const noexcept;
	ATLAS_API auto const& getName() const noexcept { return m_name; }
	ATLAS_API auto const& getId() const noexcept { return m_id; }
	ATLAS_API double getNLV() const noexcept;
	ATLAS_API void enableTracerHistory(TracerType t) noexcept;
	ATLAS_API Eigen::VectorXd const& getHistory(TracerType t) const noexcept;
	ATLAS_API Eigen::MatrixXd const& getWeightHistory() const noexcept;
	ATLAS_API SharedPtr<CommisionManager> initCommissionManager() noexcept;

	Exchange const& getExchange() const noexcept;

};


}

