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
	bool m_cached = false;
	size_t m_lookback_window = 0;
	size_t m_warmup = 0;
	Exchange& m_exchange;
	
	CovarianceNode(
		Exchange& exchange,
		SharedPtr<TriggerNode> trigger,
		size_t lookback_window
	) noexcept;

public:
	~CovarianceNode() noexcept;

	template<typename ...Arg> SharedPtr<CovarianceNode>
	static make(Arg&&...arg) {
		struct EnableMakeShared : public CovarianceNode {
			EnableMakeShared(Arg&&...arg) :CovarianceNode(std::forward<Arg>(arg)...) {}
		};
		return std::make_shared<EnableMakeShared>(std::forward<Arg>(arg)...);
	}

	SharedPtr<TriggerNode> getTrigger() const noexcept { return m_trigger; }
	void evaluate() noexcept override;
	void reset() noexcept;
	bool getIsCached() const noexcept { return m_cached; }
	size_t getWarmup() const noexcept override;
	Exchange& getExchange() const noexcept { return m_exchange; }
	LinAlg::EigenMatrixXd const& getCovariance() const noexcept { return m_covariance; }
};


//============================================================================
export class AllocationWeightNode : public StrategyBufferOpNode
{
protected:
	SharedPtr<CovarianceNode> m_covariance = nullptr;
	Option<double> m_vol_target = std::nullopt;

protected:
	void targetVol(LinAlg::EigenVectorXd& target) const noexcept;

public:
	virtual ~AllocationWeightNode() noexcept;

	AllocationWeightNode(
		SharedPtr<CovarianceNode> covariance,
		Option<double> vol_target
	) noexcept;
	
	virtual void evaluate(LinAlg::EigenVectorXd& target) noexcept = 0;
	bool getIsCached() const noexcept { return m_covariance->getIsCached(); }
};


//============================================================================
export class InvVolWeight final : public AllocationWeightNode
{
private:
public:
	ATLAS_API ~InvVolWeight() noexcept;

	ATLAS_API InvVolWeight(
		SharedPtr<CovarianceNode> covariance,
		Option<double> vol_target = std::nullopt
	) noexcept;
	
	size_t getWarmup() const noexcept override { return m_covariance->getWarmup(); }
	void evaluate(LinAlg::EigenVectorXd& target) noexcept override;
};


}

}
