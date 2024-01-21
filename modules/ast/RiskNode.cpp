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
	m_trigger(trigger)
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


}

}