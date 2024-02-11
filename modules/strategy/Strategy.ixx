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

namespace Atlas
{


//============================================================================
class StrategyImpl;


//============================================================================
export class Strategy
{
	friend class AST::StrategyNode;
	friend class AST::StrategyGrid;
	friend class Exchange;
	friend class Hydra;
	friend class CommisionManager;
private:
	String m_name;
	size_t m_id = 0;
	bool m_step_call = false;
	UniquePtr<StrategyImpl> m_impl;

	void evaluate(Eigen::Ref<Eigen::VectorXd> const& target_weights_buffer) noexcept;
	void lateRebalance(Eigen::Ref<Eigen::VectorXd> target_weights_buffer) noexcept;
	void step(Eigen::Ref<Eigen::VectorXd> target_weights_buffer) noexcept;
	void step() noexcept;
	void reset() noexcept;
	void setNlv(double nlv_new) noexcept;
	void setID(size_t id) noexcept { m_id = id; }
	SharedPtr<Tracer> getTracerPtr() const noexcept;
	void setTracer(SharedPtr<Tracer> tracer) noexcept;

public:
	ATLAS_API Strategy(
		String name,
		SharedPtr<AST::StrategyNode> ast,
		double portfolio_weight
	) noexcept;

	ATLAS_API ~Strategy() noexcept;
	ATLAS_API [[nodiscard]] Eigen::VectorXd const& getAllocationBuffer() const noexcept;
	ATLAS_API [[nodiscard]] double getAllocation(size_t asset_index) const noexcept;
	ATLAS_API [[nodiscard]] Tracer const& getTracer() const noexcept;
	ATLAS_API [[nodiscard]] auto const& getName() const noexcept { return m_name; }
	ATLAS_API [[nodiscard]] auto const& getId() const noexcept { return m_id; }
	ATLAS_API [[nodiscard]] double getNLV() const noexcept;
	ATLAS_API [[nodiscard]] Eigen::VectorXd const& getHistory(TracerType t) const noexcept;
	ATLAS_API [[nodiscard]] Eigen::MatrixXd const& getWeightHistory() const noexcept;
	ATLAS_API [[nodiscard]] SharedPtr<CommisionManager> initCommissionManager() noexcept;
	ATLAS_API [[nodiscard]] Exchange const& getExchange() const noexcept;
	ATLAS_API [[nodiscard]] Result<bool, AtlasException> enableTracerHistory(TracerType t) noexcept;
	ATLAS_API void pyEnableTracerHistory(TracerType t);
	ATLAS_API void setVolTracer(SharedPtr<AST::CovarianceNodeBase> node) noexcept;
};


}

