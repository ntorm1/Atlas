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
	if (m_exchange.currentIdx() < m_lookback_window)
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
	assert(m_centered_returns.rows() == returns_block.rows());
	m_covariance = (m_centered_returns.adjoint() * m_centered_returns) / double(returns_block.rows() - 1);
	assert(m_covariance.rows() == returns_block.rows());
	assert(m_covariance.cols() == returns_block.rows());
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
	SharedPtr<CovarianceNode> covariance
) noexcept :
	AllocationWeightNode(covariance->getTrigger()),
	m_lookback_window(covariance->getWarmup()),
	m_covariance(std::move(covariance))
{
}


//============================================================================
void
InvVolWeight::evaluate(LinAlg::EigenVectorXd& target) noexcept
{
	auto const& covariance = m_covariance->getCovariance();
	assert(covariance.rows() == covariance.cols());
	auto vol = covariance.diagonal().array().sqrt();
	target = (target.array() / vol.array()).matrix();
	target = (target.array() / target.sum()).matrix();
}


}

}