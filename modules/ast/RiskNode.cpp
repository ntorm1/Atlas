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
	Time::TimeOffset l,
	SharedPtr<TriggerNode> trigger
) noexcept :
	OpperationNode<void, LinAlg::EigenVectorXd&>(NodeType::ALLOC_WEIGHT),
	m_lookback(l),
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
	Time::TimeOffset l,
	SharedPtr<TriggerNode> trigger
) noexcept :
	AllocationWeightNode(l, trigger)
{
	m_vol.resize(m_exchange.getAssetCount());
	m_vol.setZero();
}


//============================================================================
void
AllocationWeightNode::buildLookbackDefs() noexcept
{
	Eigen::VectorXi const& mask = m_trigger->getMask();
	Exchange const& exchange = m_trigger->getExchange();
	auto const& timestamps = exchange.getTimestamps();

	assert(mask.size() == timestamps.size());
	m_lookback_defs.reserve(mask.size());

	for (size_t i = 0; i < timestamps.size(); ++i)
	{
		if (mask[i] == 0)
		{
			continue;
		}
		auto timestamp = timestamps[i];
		auto timestamp_start = applyTimeOffset(timestamp, m_lookback);
		auto it = std::lower_bound(
			timestamps.begin() + i,
			timestamps.end(),
			timestamp_start
		);
		assert(it != timestamps.end());
		size_t lookback_start = it - timestamps.begin();
		m_lookback_defs.emplace_back(i, lookback_start, i);
	}
}


//============================================================================
void
InvVolWeight::cache() noexcept
{ 
	// pull out the block of returns data we need using the lookback defs
	// to determine the start and end indices. Lookback returns block has #rows 
	// equal to the number of assets, with columnds for timestamps.
	assert(m_lookback_idx < m_lookback_defs.size());
	RiskLookbackDef const& def = m_lookback_defs[m_lookback_idx];
	auto returns_block = m_exchange.getMarketReturnsBlock(
		def.returns_start_idx,
		def.returns_end_idx
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