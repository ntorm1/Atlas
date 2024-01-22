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
export class AllocationWeightNode : public OpperationNode<void, LinAlg::EigenVectorXd&>
{
protected:
	SharedPtr<TriggerNode> m_trigger;
	Exchange const& m_exchange;

public:
	virtual ~AllocationWeightNode() noexcept;
	AllocationWeightNode(
		SharedPtr<TriggerNode> trigger
	) noexcept;

	virtual void cache() noexcept = 0;
	void evaluate(LinAlg::EigenVectorXd& target) noexcept override = 0;
};


//============================================================================
export class InvVolWeight final : public AllocationWeightNode
{
private:
	LinAlg::EigenVectorXd m_vol;
	size_t m_lookback_window = 0;
public:
	~InvVolWeight() noexcept;
	InvVolWeight(
		size_t lookback_window,
		SharedPtr<TriggerNode> trigger
	) noexcept;

	void cache() noexcept override;
	void evaluate(LinAlg::EigenVectorXd& target) noexcept override = 0;

};


}

}
