module;
#pragma once
#ifdef ATLAS_EXPORTS
#define ATLAS_API __declspec(dllexport)
#else
#define ATLAS_API  __declspec(dllimport)
#endif
#include "AtlasStruct.hpp"
export module TracerModule;

import AtlasCore;
import AtlasEnumsModule;
import AtlasLinAlg;

namespace Atlas
{

//============================================================================
export constexpr size_t ALLOC_PCT_INDEX = 0;


//============================================================================
export struct StructTracer
{
	Tracer const& m_tracer;
	Exchange const& m_exchange;
	Vector<Order> m_orders;
	Vector<Trade> m_trades;
	size_t close_index = 0;
	bool orders_eager = false;

	StructTracer(
		Exchange const& exchange,
		Tracer const& tracer
	) noexcept;
	StructTracer(StructTracer const&) = delete;
	StructTracer(StructTracer&&) = delete;
	StructTracer& operator=(StructTracer const&) = delete;
	StructTracer& operator=(StructTracer&&) = delete;
	ATLAS_API ~StructTracer() noexcept;

	bool eager() const noexcept { return orders_eager; }
	void enabelTracerHistory(TracerType t) noexcept;
	void evaluate(
		LinAlg::EigenVectorXd const& weights,
		LinAlg::EigenVectorXd const& previous_weights
	) noexcept;
	void reset() noexcept;
};


//============================================================================
export class Tracer
{
	friend struct StructTracer;
	friend class Strategy;
	friend class AST::AllocationBaseNode;
	friend class AST::StrategyGrid;
private:
	Exchange const& m_exchange;
	Strategy const& m_strategy;
	Option<UniquePtr<StructTracer>> m_struct_tracer = std::nullopt;
	Option<SharedPtr<AST::CovarianceNodeBase>> m_covariance;
	LinAlg::EigenMatrixXd m_weight_history;
	LinAlg::EigenVectorXd m_nlv_history;
	LinAlg::EigenVectorXd m_volatility_history;
	LinAlg::EigenVectorXd m_pnl;
	LinAlg::EigenVectorXd m_weights_buffer;
	size_t m_idx = 0;
	double m_cash = 0.0;
	double m_initial_cash = 0.0;
	double m_nlv = 0.0;

	void realize() noexcept;
	void evaluate() noexcept;
	void reset() noexcept;

	[[nodiscard]] Result<bool,AtlasException> enableTracerHistory(TracerType t) noexcept;
	void setNLV(double nlv) noexcept { m_nlv = nlv; }
	void setCovarianceNode(SharedPtr<AST::CovarianceNodeBase> covariance) noexcept { m_covariance = covariance; }
	LinAlg::EigenVectorXd& getPnL() noexcept;
	void setPnL(LinAlg::EigenRef<LinAlg::EigenVectorXd> pnl) noexcept;
	void initPnL() noexcept;

public:
	Tracer(Strategy const& strategy, Exchange const& exchange, double cash) noexcept;
	ATLAS_API Vector<Order> const& getOrders() const noexcept;
	ATLAS_API LinAlg::EigenVectorXd const& getHistory(TracerType t) const noexcept;
	ATLAS_API double getCash() const noexcept { return m_cash; }
	ATLAS_API double getNLV() const noexcept { return m_nlv; }
	ATLAS_API double getInitialCash() const noexcept { return m_initial_cash; }
};



}