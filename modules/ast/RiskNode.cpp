module;

module RiskNodeModule;

import HelperNodesModule;


namespace Atlas
{

namespace AST
{


//============================================================================
AllocationWeightNode::~AllocationWeightNode() noexcept
{
}


//============================================================================
AllocationWeightNode::AllocationWeightNode(
	RiskLookback l,
	SharedPtr<TriggerNode> trigger
) noexcept :
	OpperationNode<void, LinAlg::EigenVectorXd&>(NodeType::ALLOC_WEIGHT),
	m_lookback(l),
	m_trigger(trigger)
{
}


//============================================================================
InvVolWeight::~InvVolWeight() noexcept
{
}


//============================================================================
InvVolWeight::InvVolWeight(
	RiskLookback l,
	SharedPtr<TriggerNode> trigger
) noexcept :
	AllocationWeightNode(l, trigger)
{
}

}

}