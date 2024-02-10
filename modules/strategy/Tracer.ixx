module;
#pragma once
#ifdef ATLAS_EXPORTS
#define ATLAS_API __declspec(dllexport)
#else
#define ATLAS_API  __declspec(dllimport)
#endif
#include <Eigen/Dense>
export module TracerModule;

import AtlasCore;
import AtlasEnumsModule;

namespace Atlas
{

//============================================================================
export constexpr size_t ALLOC_PCT_INDEX = 0;


//============================================================================
export class Tracer
{
	friend class Strategy;
private:
	Exchange const& m_exchange;
	Strategy const& m_strategy;
	Option<SharedPtr<AST::CovarianceNode>> m_covariance;
	Eigen::MatrixXd m_weight_history;
	Eigen::VectorXd m_nlv_history;
	Eigen::VectorXd m_volatility_history;
	size_t m_idx = 0;
	double m_cash;
	double m_initial_cash;
	double m_nlv;

	void evaluate() noexcept;
	void reset() noexcept;

	[[nodiscard]] Result<bool,AtlasException> enableTracerHistory(TracerType t) noexcept;
	void setNLV(double nlv) noexcept { m_nlv = nlv; }
	void setCovarianceNode(SharedPtr<AST::CovarianceNode> covariance) noexcept { m_covariance = covariance; }
	Eigen::VectorXd const& getHistory(TracerType t) const noexcept;


public:
	Tracer(Strategy const& strategy, Exchange const& exchange, double cash) noexcept;
	ATLAS_API double getCash() const noexcept { return m_cash; }
	ATLAS_API double getNLV() const noexcept { return m_nlv; }
	ATLAS_API double getInitialCash() const noexcept { return m_initial_cash; }
};



}