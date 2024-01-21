module;

export module RiskNodeModule;


import AtlasLinAlg;
import AtlasCore;
import BaseNodeModule;


namespace Atlas
{

namespace AST
{


//============================================================================
export enum class RiskLookbackType
{
	DAILY = 0
};


//============================================================================
export struct RiskLookback
{
	RiskLookbackType type;
	size_t count;
};



//============================================================================
export class AllocationWeightNode : public OpperationNode<void, LinAlg::EigenVectorXd&>
{
	RiskLookback m_lookback;
	SharedPtr<TriggerNode> m_trigger;
public:
	virtual ~AllocationWeightNode() noexcept;
	AllocationWeightNode(
		RiskLookback l,
		SharedPtr<TriggerNode> trigger
	) noexcept;

	void evaluate(LinAlg::EigenVectorXd& target) noexcept override = 0;

};


//============================================================================
export class InvVolWeight final : public AllocationWeightNode
{
public:
	~InvVolWeight() noexcept;
	InvVolWeight(
		RiskLookback l,
		SharedPtr<TriggerNode> trigger
	) noexcept;

	void evaluate(LinAlg::EigenVectorXd& target) noexcept override = 0;

};


}

}
