module;
#include <Eigen/Dense>
#include <chrono>
#include "AtlasMacros.hpp"
module HelperNodesModule;

import ExchangeModule;
import StrategyModule;

namespace Atlas
{

namespace AST

{

	
//============================================================================
static int getMonthFromEpoch(int64_t epoch) noexcept
{
	auto const& time = std::chrono::system_clock::from_time_t(epoch);
	auto const& time_point = std::chrono::system_clock::to_time_t(time);

	// Using localtime_s to address deprecation warning
	std::tm time_info;
	if (localtime_s(&time_info, &time_point) != 0)
	{
		// Handle error, localtime_s failed
		return -1;
	}

	return time_info.tm_mon;
}


//============================================================================
StrategyMonthlyRunnerNode::StrategyMonthlyRunnerNode(
	Strategy const& strategy
) noexcept: 
	TriggerNode(strategy.getExchange())
{
}


//============================================================================
Result<bool, AtlasException>
StrategyMonthlyRunnerNode::build() noexcept
{
	auto const& timestamps = m_exchange.getTimestamps();
	m_tradeable_mask.resize(timestamps.size());
	m_tradeable_mask.setZero();

	int previous_month = -1;
	for (size_t t = 0; t < timestamps.size(); ++t)
	{
		auto timestamp = timestamps[t];
		auto month = getMonthFromEpoch(t);
		if (month == -1)
		{
			return Err("Failed to get month from epoch");
		}
		if (t == 0 || month != previous_month)
		{
			m_tradeable_mask[t] = 1;
			previous_month = month;
		}
	}
	return true;
}

//============================================================================
bool
StrategyMonthlyRunnerNode::evaluate() noexcept
{
	bool is_tradeable = static_cast<bool>(m_tradeable_mask[m_index_counter]);
	m_index_counter++;
	return is_tradeable;
}


//============================================================================
Result<UniquePtr<TriggerNode>, AtlasException>
StrategyMonthlyRunnerNode::make(Strategy const& strategy) noexcept
{
	auto node = std::make_unique<StrategyMonthlyRunnerNode>(strategy);
	auto result = node->build();
	if (!result)
	{
		return Err(result.error());
	}
	return node;
}


}

}