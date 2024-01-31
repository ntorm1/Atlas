module;
#include <Eigen/Dense>
module RiskNodeModule;

import HelperNodesModule;
import ExchangeModule;
import AllocationNodeModule;


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
	size_t start_idx = (m_exchange.currentIdx() - m_lookback_window) + 1;
	auto const& returns_block = m_exchange.getMarketReturnsBlock(
		start_idx,
		m_exchange.currentIdx()
	);
	Eigen::MatrixXd returns_block_transpose = returns_block.transpose();
	m_centered_returns = returns_block_transpose.rowwise() - returns_block_transpose.colwise().mean();
	m_covariance = (m_centered_returns.adjoint() * m_centered_returns) / double(returns_block_transpose.rows() - 1);
}


//============================================================================
AllocationWeightNode::~AllocationWeightNode() noexcept
{
}


//============================================================================
AllocationWeightNode::AllocationWeightNode(
	SharedPtr<CovarianceNode> covariance,
	Option<double> vol_target
) noexcept :
	StrategyBufferOpNode(NodeType::ALLOC_WEIGHT, covariance->getExchange(), std::nullopt),
	m_covariance(covariance),
	m_vol_target(vol_target)
{
}


//============================================================================
void
AllocationWeightNode::targetVol(LinAlg::EigenVectorXd& target) const noexcept
{
	auto const& covariance = m_covariance->getCovariance();
	assert(covariance.rows() == covariance.cols());
	assert(m_vol_target);
	auto current_vol = (target.transpose() * covariance * target);
	double vol_scale = m_vol_target.value() / current_vol;
	target *= vol_scale;
}


//============================================================================
InvVolWeight::~InvVolWeight() noexcept
{
}


//============================================================================
InvVolWeight::InvVolWeight(
	SharedPtr<CovarianceNode> covariance,
	Option<double> vol_target
) noexcept :
	AllocationWeightNode(std::move(covariance), vol_target)
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
	if (m_vol_target)
	{
		targetVol(target);
	}
}


}

}