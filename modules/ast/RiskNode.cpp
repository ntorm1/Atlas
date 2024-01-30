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
CovarianceNode::CovarianceNode(
	Exchange& exchange,
	SharedPtr<TriggerNode> trigger,
	size_t lookback_window
) noexcept :
	StatementNode(NodeType::COVARIANCE),
	m_trigger(trigger),
	m_exchange(exchange),
	m_lookback_window(lookback_window)
{
	size_t row_count = exchange.getAssetCount();
	m_covariance.resize(row_count, row_count);
	m_covariance.setZero();
}


//============================================================================
CovarianceNode::~CovarianceNode() noexcept
{

}


//============================================================================
void
CovarianceNode::evaluate() noexcept
{
	if (!m_trigger->evaluate())
	{
		return;
	}
	size_t start_idx = m_exchange.currentIdx() - m_lookback_window;
	auto const& returns_block = m_exchange.getMarketReturnsBlock(
		start_idx,
		m_exchange.currentIdx()
	);
	assert(returns_block.rows() == m_covariance.rows());
	m_centered_returns = returns_block.rowwise() - returns_block.colwise().mean();
	m_covariance = (m_centered_returns.adjoint() * m_centered_returns) / double(returns_block.rows() - 1);
}


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