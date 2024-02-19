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
import AtlasEnumsModule;
import AtlasLinAlg;

namespace Atlas
{


//============================================================================
class StrategyImpl;


//============================================================================
export class Strategy
{
	friend class AST::StrategyNode;
	friend class AST::StrategyGrid;
	friend class AST::GridDimensionObserver;
	friend class Exchange;
	friend class Hydra;
	friend class CommisionManager;
private:
	String m_name;
	size_t m_id = 0;
	bool m_step_call = false;
	UniquePtr<StrategyImpl> m_impl;

	void evaluate(LinAlg::EigenRef<LinAlg::EigenVectorXd> const& target_weights_buffer) noexcept;
	void lateRebalance(LinAlg::EigenRef<LinAlg::EigenVectorXd> target_weights_buffer) noexcept;
	void step(LinAlg::EigenRef<LinAlg::EigenVectorXd> target_weights_buffer) noexcept;
	void step() noexcept;
	void reset() noexcept;
	void realize() noexcept;
	void setNlv(double nlv_new) noexcept;
	void setID(size_t id) noexcept { m_id = id; }
	SharedPtr<Tracer> getTracerPtr() const noexcept;
	Option<SharedPtr<AST::TradeLimitNode>> getTradeLimitNode() const noexcept;
	LinAlg::EigenRef<LinAlg::EigenVectorXd> getPnL() noexcept;
	void setTracer(SharedPtr<Tracer> tracer) noexcept;
	size_t refreshWarmup() noexcept;
	size_t getWarmup() const noexcept;
public:
	ATLAS_API Strategy(
		String name,
		SharedPtr<AST::StrategyNode> ast,
		double portfolio_weight
	) noexcept;

	ATLAS_API ~Strategy() noexcept;
	ATLAS_API [[nodiscard]] LinAlg::EigenVectorXd const& getAllocationBuffer() const noexcept;
	ATLAS_API [[nodiscard]] double getAllocation(size_t asset_index) const noexcept;
	ATLAS_API [[nodiscard]] Tracer const& getTracer() const noexcept;
	ATLAS_API [[nodiscard]] auto const& getName() const noexcept { return m_name; }
	ATLAS_API [[nodiscard]] auto const& getId() const noexcept { return m_id; }
	ATLAS_API [[nodiscard]] double getNLV() const noexcept;
	ATLAS_API [[nodiscard]] LinAlg::EigenVectorXd const& getHistory(TracerType t) const noexcept;
	ATLAS_API [[nodiscard]] LinAlg::EigenMatrixXd const& getWeightHistory() const noexcept;
	ATLAS_API [[nodiscard]] SharedPtr<CommisionManager> initCommissionManager() noexcept;
	ATLAS_API [[nodiscard]] Exchange const& getExchange() const noexcept;
	ATLAS_API [[nodiscard]] Result<bool, AtlasException> enableTracerHistory(TracerType t) noexcept;
	ATLAS_API [[nodiscard]] Option<SharedPtr<AST::StrategyGrid>> getGrid() const noexcept;
	ATLAS_API void pyEnableTracerHistory(TracerType t);
	ATLAS_API void setVolTracer(SharedPtr<AST::CovarianceNodeBase> node) noexcept;
	
	ATLAS_API [[nodiscard]] Result<SharedPtr<AST::StrategyGrid const>, AtlasException>
	setGridDimmensions(
		std::pair<SharedPtr<AST::GridDimension>, SharedPtr<AST::GridDimension>> dimensions,
		Option<GridType> grid_type = std::nullopt
	) noexcept;
	ATLAS_API [[nodiscard]] SharedPtr<AST::StrategyGrid const>
	pySetGridDimmensions(
		std::pair<SharedPtr<AST::GridDimension>, SharedPtr<AST::GridDimension>> dimensions,
		Option<GridType> grid_type = std::nullopt
	);
};


}

