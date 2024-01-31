module;
#pragma once
#ifdef ATLAS_EXPORTS
#define ATLAS_API __declspec(dllexport)
#else
#define ATLAS_API  __declspec(dllimport)
#endif
export module RiskNodeModule;

import AtlasLinAlg;
import AtlasCore;
import BaseNodeModule;
import AtlasTimeModule;
import StrategyBufferModule;

namespace Atlas
{

namespace AST
{

//============================================================================
export class CovarianceNode : public StatementNode
{
	friend class Exchange;
private:
	LinAlg::EigenMatrixXd m_covariance;
	LinAlg::EigenMatrixXd m_centered_returns;
	SharedPtr<TriggerNode> m_trigger;
	size_t m_lookback_window = 0;
	Exchange& m_exchange;
	
	//============================================================================
	CovarianceNode(
		Exchange& exchange,
		SharedPtr<TriggerNode> trigger,
		size_t lookback_window
	) noexcept;

public:
	//============================================================================
	~CovarianceNode() noexcept;

	//============================================================================
	template<typename ...Arg> SharedPtr<CovarianceNode>
	static make(Arg&&...arg) {
		struct EnableMakeShared : public CovarianceNode {
			EnableMakeShared(Arg&&...arg) :CovarianceNode(std::forward<Arg>(arg)...) {}
		};
		return std::make_shared<EnableMakeShared>(std::forward<Arg>(arg)...);
	}

	//============================================================================
	SharedPtr<TriggerNode> getTrigger() const noexcept { return m_trigger; }

	//============================================================================
	void evaluate() noexcept override;

	//============================================================================
	size_t getWarmup() const noexcept override { return m_lookback_window; }

	//============================================================================
	Exchange& getExchange() const noexcept { return m_exchange; }

	//============================================================================
	LinAlg::EigenMatrixXd const& getCovariance() const noexcept { return m_covariance; }
};


//============================================================================
export class AllocationWeightNode : public StrategyBufferOpNode
{
protected:
	SharedPtr<CovarianceNode> m_covariance = nullptr;
	SharedPtr<AllocationBaseNode> m_allocation = nullptr;
	Option<double> m_vol_target = std::nullopt;

protected:
	void targetVol(LinAlg::EigenVectorXd& target) const noexcept;

public:
	//============================================================================
	virtual ~AllocationWeightNode() noexcept;

	//============================================================================
	AllocationWeightNode(
		SharedPtr<AllocationBaseNode> allocation,
		SharedPtr<CovarianceNode> covariance,
		Option<double> vol_target
	) noexcept;
	
	//============================================================================
	void evaluate(LinAlg::EigenVectorXd& target) noexcept override;

	//============================================================================
	virtual void evaluateChild(LinAlg::EigenVectorXd& target) noexcept = 0;
};


//============================================================================
export class InvVolWeight final : public AllocationWeightNode
{
private:
public:
	//============================================================================
	ATLAS_API ~InvVolWeight() noexcept;

	//============================================================================
	ATLAS_API InvVolWeight(
		SharedPtr<AllocationBaseNode> allocation,
		SharedPtr<CovarianceNode> covariance,
		Option<double> vol_target = std::nullopt
	) noexcept;
	
	//============================================================================
	size_t getWarmup() const noexcept override { return m_covariance->getWarmup(); }

	//============================================================================
	void evaluateChild(LinAlg::EigenVectorXd& target) noexcept override;
};


}

}
