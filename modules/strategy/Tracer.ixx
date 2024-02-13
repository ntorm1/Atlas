module;
#pragma once
#ifdef ATLAS_EXPORTS
#define ATLAS_API __declspec(dllexport)
#else
#define ATLAS_API  __declspec(dllimport)
#endif
export module TracerModule;

import AtlasCore;
import AtlasEnumsModule;
import AtlasLinAlg;

namespace Atlas
{

//============================================================================
export constexpr size_t ALLOC_PCT_INDEX = 0;


//============================================================================
export class Tracer
{
	friend class Strategy;
	friend class AST::AllocationBaseNode;
	friend class AST::StrategyGrid;
private:
	Exchange const& m_exchange;
	Strategy const& m_strategy;
	Option<SharedPtr<AST::CovarianceNodeBase>> m_covariance;
	LinAlg::EigenMatrixXd m_weight_history;
	LinAlg::EigenVectorXd m_nlv_history;
	LinAlg::EigenVectorXd m_volatility_history;
	LinAlg::EigenVectorXd m_pnl;
	size_t m_idx = 0;
	double m_cash = 0.0;
	double m_initial_cash = 0.0;
	double m_nlv = 0.0;

	void evaluate() noexcept;
	void reset() noexcept;

	[[nodiscard]] Result<bool,AtlasException> enableTracerHistory(TracerType t) noexcept;
	void setNLV(double nlv) noexcept { m_nlv = nlv; }
	void setCovarianceNode(SharedPtr<AST::CovarianceNodeBase> covariance) noexcept { m_covariance = covariance; }
	LinAlg::EigenVectorXd const& getHistory(TracerType t) const noexcept;
	LinAlg::EigenVectorXd& getPnL() noexcept;
	void setPnL(LinAlg::EigenRef<LinAlg::EigenVectorXd> pnl) noexcept;
	void initPnL() noexcept;

public:
	Tracer(Strategy const& strategy, Exchange const& exchange, double cash) noexcept;
	ATLAS_API double getCash() const noexcept { return m_cash; }
	ATLAS_API double getNLV() const noexcept { return m_nlv; }
	ATLAS_API double getInitialCash() const noexcept { return m_initial_cash; }
};



}