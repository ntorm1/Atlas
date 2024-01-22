module;
#include <Eigen/Dense>
module RiskNodeModule;

import HelperNodesModule;
import ExchangeModule;


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
	SharedPtr<TriggerNode> trigger
) noexcept :
	OpperationNode<void, LinAlg::EigenVectorXd&>(NodeType::ALLOC_WEIGHT),
	m_trigger(trigger),
	m_exchange(trigger->getExchange())
{
}


//============================================================================
InvVolWeight::~InvVolWeight() noexcept
{
}


//============================================================================
InvVolWeight::InvVolWeight(
	size_t window,
	SharedPtr<TriggerNode> trigger
) noexcept :
	AllocationWeightNode(trigger),
	m_lookback_window(window)
{
	m_vol.resize(m_exchange.getAssetCount());
	m_vol.setZero();
}


//============================================================================
void
InvVolWeight::cache() noexcept
{ 
	// pull out the block of returns data we need using the lookback defs
	// to determine the start and end indices. Lookback returns block has #rows 
	// equal to the number of assets, with columnds for timestamps.
	size_t exchange_idx = m_exchange.currentIdx();
	assert(exchange_idx > m_lookback_window);
	size_t start_idx = exchange_idx - m_lookback_window;
	auto returns_block = m_exchange.getMarketReturnsBlock(
		start_idx,
		exchange_idx
	);
	for (size_t i = 0; i < m_exchange.getAssetCount(); ++i)
	{
		auto ys = returns_block.row(i);
		m_vol[i] =  (ys.array() - ys.mean()).square().sum() / (ys.size() - 1);
	}
}


//============================================================================
void
InvVolWeight::evaluate(LinAlg::EigenVectorXd& target) noexcept
{
	if (m_trigger->evaluate())
	{
		cache();
	}
	assert(m_vol.size() == target.size());
	target = target.cwiseQuotient(m_vol);
	target /= target.sum();
}


}

}