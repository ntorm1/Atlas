module;
#include <Eigen/Dense>
#ifdef _DEBUG 
#include <iostream>
#endif _DEBUG 
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

	// find the index location of the first non-zero value
	Eigen::VectorXi const& mask = m_trigger->getMask();
	size_t idx = 0;
	for (size_t i = 0; i < static_cast<size_t>(mask.size()); ++i)
	{
		if (mask(i) != 0)
		{
			idx = i;
			break;
		}
	}
	// set the warmup equal to the max of the lookback window and the index
	m_warmup = std::max(m_lookback_window, idx);
}


//============================================================================
CovarianceNode::~CovarianceNode() noexcept
{

}


//============================================================================
size_t CovarianceNode::getWarmup() const noexcept
{
	return m_warmup;
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
	m_cached = true;
}


//============================================================================
void
CovarianceNode::reset() noexcept
{
	m_covariance.setZero();
	m_centered_returns.setZero();
	m_cached = false;
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
	auto v = (target.transpose() * covariance * target);
	double annualized_vol = std::sqrt(v(0)) * std::sqrt(252);
	double vol_scale = m_vol_target.value() / annualized_vol;
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
	// prevent division by zero
	assert(target.array().abs().sum() > 1e-10);
	
	auto const& covariance = m_covariance->getCovariance();

	assert(covariance.rows() == covariance.cols());
	auto vol = covariance.diagonal().array().sqrt();
	assert(!vol.array().isNaN().any());
	
	target = (target.array() / vol.array()).matrix();
	double abs_weight_sum = target.array().abs().sum();
	assert(abs_weight_sum > 1e-10);
	target = (target.array() / abs_weight_sum).matrix();
	if (m_vol_target)
	{
		targetVol(target);
	}
	assert(!target.array().isNaN().any());
}


}

}