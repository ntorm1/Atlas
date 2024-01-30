module;

export module RiskNodeModule;


import AtlasLinAlg;
import AtlasCore;
import BaseNodeModule;
import AtlasTimeModule;

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
	Exchange const& m_exchange;
	
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
	template<typename ...Arg> std::shared_ptr<CovarianceNode>
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
	LinAlg::EigenMatrixXd const& getCovariance() const noexcept { return m_covariance; }
};


//============================================================================
export class AllocationWeightNode : public OpperationNode<void, LinAlg::EigenVectorXd&>
{
protected:
	SharedPtr<TriggerNode> m_trigger;
	Exchange const& m_exchange;

public:
	//============================================================================
	virtual ~AllocationWeightNode() noexcept;

	//============================================================================
	AllocationWeightNode(
		SharedPtr<TriggerNode> trigger
	) noexcept;

	//============================================================================
	virtual void cache() noexcept = 0;
	
	//============================================================================
	void evaluate(LinAlg::EigenVectorXd& target) noexcept override = 0;
};


//============================================================================
export class InvVolWeight final : public AllocationWeightNode
{
private:
	LinAlg::EigenVectorXd m_vol;
	size_t m_lookback_window = 0;
public:
	//============================================================================
	~InvVolWeight() noexcept;

	//============================================================================
	InvVolWeight(
		size_t lookback_window,
		SharedPtr<TriggerNode> trigger
	) noexcept;

	//============================================================================
	void cache() noexcept override;
	
	//============================================================================
	size_t getWarmup() const noexcept override { return m_lookback_window; }

	//============================================================================
	void evaluate(LinAlg::EigenVectorXd& target) noexcept override = 0;
};


}

}
