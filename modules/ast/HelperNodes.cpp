module;
#include <Eigen/Dense>
#include "AtlasMacros.hpp"
module HelperNodesModule;

import ExchangeModule;
import StrategyModule;
import AtlasTimeModule;

namespace Atlas
{

namespace AST

{


//============================================================================
Int64
applyDateOffset()
{
	return Int64();
}
	
//============================================================================
StrategyMonthlyRunnerNode::StrategyMonthlyRunnerNode(
	Exchange const& exchange,
	bool eom_trigger
) noexcept: 
	TriggerNode(exchange),
	m_eom_trigger(eom_trigger)
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
		auto month = Time::getMonthFromEpoch(timestamp);
		if (month == -1)
		{
			return Err("Failed to get month from epoch");
		}
		// if not eom trigger, then set true to index locations where the current
		// month does not equal the previous month
		if (!m_eom_trigger)
		{
			if (t == 0 || month != previous_month)
			{
				m_tradeable_mask[t] = 1;
				previous_month = month;
			}
		}
		// if eom trigger, then set true to index locations where the current month
		// does not equal the next month. Default do nothing on last timestamp
		else
		{
			// default do trigger on first timestamp
			if (t == 0)
			{
				m_tradeable_mask[t] = 1;
			}

			else if (t < timestamps.size() - 1)
			{
				auto next_timestamp = timestamps[t + 1];
				auto next_month = Time::getMonthFromEpoch(next_timestamp);
				if (next_month != month)
				{
					m_tradeable_mask[t] = 1;
				}
			}
		}
	}
	return true;
}


//============================================================================
void
StrategyMonthlyRunnerNode::reset() noexcept
{
	m_index_counter = 0;
}


//============================================================================
bool
StrategyMonthlyRunnerNode::evaluate() noexcept
{
	bool is_tradeable = static_cast<bool>(m_tradeable_mask(m_index_counter));
	m_index_counter++;
	return is_tradeable;
}


//============================================================================
SharedPtr<TriggerNode>
StrategyMonthlyRunnerNode::pyMake(
	SharedPtr<Exchange> exchange,
	bool eom_trigger
)
{
	auto node = std::make_unique<StrategyMonthlyRunnerNode>(*exchange);
	auto result = node->build();
	if (!result)
	{
		throw std::runtime_error(result.error().what());
	}
	return node;
}


}

}