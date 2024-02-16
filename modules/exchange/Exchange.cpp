module;
#include "AtlasMacros.hpp"
#include <Eigen/Dense>
module ExchangeModule;

import ExchangePrivateModule;
import StrategyModule;
import AtlasUtilsModule;
import AtlasTimeModule;
import HelperNodesModule;
import RiskNodeModule;

namespace Atlas
{

//============================================================================
Exchange::Exchange(
	String name,
	String source,
	size_t id
) noexcept
{
	m_impl = std::make_unique<ExchangeImpl>();
	m_name = std::move(name);
	m_source = std::move(source);
	m_id = id;
}


//============================================================================
Result<bool, AtlasException>
Exchange::validate() noexcept
{

	for (auto const& asset : m_impl->assets)
	{
		// validate all assets have the same headers
		auto const& asset_headers = asset.headers;
		if (m_impl->headers.size() == 0) {
			m_impl->headers = asset_headers;
			m_impl->col_count = asset_headers.size();
		}
		else
		{
			for (auto const& [key, value] : asset_headers)
			{
				EXPECT_FALSE(
					m_impl->headers.count(key) == 0,
					"Asset header does not exist in exchange"
				);
			}
		}

		//validate asset timestamps are in ascending order
		auto const& timestamps = asset.timestamps;
		if (timestamps.size() == 0)
		{
			return Err("Asset has no timestamps");
		}
		for (size_t i = 1; i < timestamps.size(); i++)
		{
			bool is_descending = timestamps[i] < timestamps[i - 1];
			if (is_descending)
			{
				String message = std::format(
					"Asset timestamps are not in ascending order: i-1: {}, i: {}",
					Time::convertNanosecondsToTime(timestamps[i - 1]),
					Time::convertNanosecondsToTime(timestamps[i])
				);
				return Err(message);
			};
		}
		m_impl->asset_id_map[asset.name] = m_impl->asset_id_map.size();
		m_impl->timestamps = sortedUnion(m_impl->timestamps, timestamps);
	}

	for (auto const& asset : m_impl->assets)
	{
		// validate each asset's timestamps are a contiguous subset of the exchange timestamps
		auto const& timestamps = asset.timestamps;
		EXPECT_FALSE(
			!isContinuousSubset(m_impl->timestamps, timestamps),
			"Asset timestamps are not a contiguous subset of exchange timestamps"
		);
		
	}

	Option<size_t> close_index = getCloseIndex();
	EXPECT_FALSE(
		!close_index.has_value(),
		"Exchange does not have a close column"
	);
	m_impl->close_index = close_index.value();
	return true;
}


//============================================================================
Result<bool, AtlasException>
Exchange::build() noexcept
{
	// eigen stores data in column major order, so the exchange's data 
	// matrix has rows = #assets, cols = #timestamps * #headers
	m_impl->data.resize(
		m_impl->assets.size(),
		m_impl->timestamps.size() * m_impl->headers.size()
	);
	// store the percentage change in price for each asset at each timestamp
	m_impl->returns.resize(
		m_impl->assets.size(),
		m_impl->timestamps.size()
	);
	// store 1 + the percentage change in price for each asset at the current timestamp
	m_impl->returns_scalar.resize(
		m_impl->assets.size()
	);
	m_impl->returns_scalar.setZero();

	for (auto const& asset : m_impl->assets)
	{
		size_t asset_index = 0;
		size_t asset_id = asset.id;
		auto const& asset_datetime_index = asset.timestamps;
		for (size_t exchange_index = 0; exchange_index < m_impl->timestamps.size(); exchange_index++)
		{
			auto exchange_datetime = m_impl->timestamps[exchange_index];
			size_t asset_datetime = 0;
			if (asset_index < asset_datetime_index.size()) 
			{
				asset_datetime = asset_datetime_index[asset_index];
			}
			// asset datetime is out of bounds or does not match exchange datetime
			if (
				!asset_datetime ||
				exchange_datetime != asset_datetime)
			{
				// fill data matrix with NAN
				for (size_t i = 0; i < m_impl->headers.size(); i++)
				{
					m_impl->data(asset_id, exchange_index * m_impl->headers.size() + i) = NAN_DOUBLE;
				}

				// fill returns matrix with 0
				m_impl->returns(asset_id, exchange_index) = 0;
				// update null count
			}
			else 
			{
				// copy asset data into exchange data matrix
				for (size_t i = 0; i < m_impl->headers.size(); i++)
				{
					size_t asset_data_index = asset_index * m_impl->headers.size() + i;
					double value = asset.data[asset_data_index];
					size_t data_index = exchange_index * m_impl->headers.size() + i;
					m_impl->data(asset_id, data_index) = value;
				}

				// calculate returns
				if (!asset_index)
				{
					m_impl->returns(asset_id, exchange_index) = 0.0;
				}
				else
				{
					double prev_close = m_impl->data(asset_id, (exchange_index - 1) * m_impl->headers.size() + m_impl->close_index);
					double curr_close = m_impl->data(asset_id, exchange_index * m_impl->headers.size() + m_impl->close_index);
					double ret = (curr_close - prev_close) / prev_close;
					if (ret != ret)
					{
						ret = 0.0;
					}
					m_impl->returns(asset_id, exchange_index) = ret;
				}
				asset_index++;
			}
		}
	}
	m_impl->assets.clear();
	return true;
}


//============================================================================
void
Exchange::reset() noexcept
{
	m_impl->current_index = 0;
	m_impl->current_timestamp = 0;

	// Reset strategies
	for (auto& strategy : m_impl->registered_strategies)
	{
		strategy->m_step_call = false;
	}

	// Collect triggers to erase
	Vector<SharedPtr<AST::TriggerNode>> triggersToErase;
	for (auto& trigger : m_impl->registered_triggers)
	{
		assert(trigger);
		if (trigger.use_count() == 1)
		{
			triggersToErase.push_back(trigger);
		}
		else
		{
			trigger->reset();
		}
	}

	// Erase collected triggers
	for (auto& trigger : triggersToErase)
	{
		m_impl->registered_triggers.erase(
			std::remove(
				m_impl->registered_triggers.begin(),
				m_impl->registered_triggers.end(),
				trigger
			),
			m_impl->registered_triggers.end()
		);
	}
	cleanupCovarianceNodes();
}

//============================================================================
void
Exchange::step(Int64 global_time) noexcept
{
	if (m_impl->current_index >= m_impl->timestamps.size())
	{
		return;
	}

	m_impl->current_timestamp = m_impl->timestamps[m_impl->current_index];
	
	if (m_impl->current_timestamp != global_time)
	{
		return;
	}

	for (auto& strategy : m_impl->registered_strategies)
	{
		strategy->m_step_call = true;
	}

	std::for_each(m_impl->registered_triggers.begin(), m_impl->registered_triggers.end(), [](auto& trigger) {assert(trigger); trigger->step(); });
	m_impl->current_index++; // cov node valls currentIdx on first step
	std::for_each(m_impl->covariance_nodes.begin(), m_impl->covariance_nodes.end(), [](auto& node_pair) { node_pair.second->evaluate(); });

	// cache the scalar returns used for evaluating portfolio
	LinAlg::EigenConstColView<double> market_returns = getMarketReturns();
	m_impl->returns_scalar = market_returns.array() + 1.0;
}


//============================================================================
SharedPtr<AST::TriggerNode>
Exchange::registerTrigger(SharedPtr<AST::TriggerNode>&& trigger) noexcept
{
	// test to see if trigger with same definition already exists
	for (auto const& existing_trigger : m_impl->registered_triggers)
	{
		if (*trigger == *existing_trigger)
		{
			return existing_trigger;
		}
	}
	m_impl->registered_triggers.push_back(trigger);
	return trigger;
}


//============================================================================
void
Exchange::cleanupCovarianceNodes() noexcept
{
	Vector<String> keysToRemove; // Store keys of nodes to remove

	// Iterate over covariance_nodes and check use_count of each node,
	// if exchange is holding the only reference to the node, remove it
	for (auto it = m_impl->covariance_nodes.begin(); it != m_impl->covariance_nodes.end();)
	{
		auto& [id, node] = *it;
		node->reset();
		size_t use_count = node.use_count();
		if (use_count == 1)
		{
			keysToRemove.push_back(id);
		}
		++it;
	}

	// Remove nodes from covariance_nodes and registered_triggers if they are not being used
	for (const auto& key : keysToRemove)
	{
		auto node = m_impl->covariance_nodes[key];
		auto trigger = node->getTrigger();
		m_impl->covariance_nodes.erase(key);

		// delete node to clear up trigger use count. If the use count is equal to 2,
		// i.e. the current node and the trigger held by the exchange then we can remove the trigger
		node.reset();
		if (trigger.use_count() == 2)
		{
			m_impl->registered_triggers.erase(
				std::remove(
					m_impl->registered_triggers.begin(),
					m_impl->registered_triggers.end(),
					trigger
				),
				m_impl->registered_triggers.end()
			);
		}
	}
}


//============================================================================
void
Exchange::cleanupTriggerNodes() noexcept
{
	for (auto it = m_impl->registered_triggers.begin(); it != m_impl->registered_triggers.end();)
	{
		auto trigger = *it;
		if (trigger.use_count() == 1)
		{
			it = m_impl->registered_triggers.erase(it);
		}
		else
		{
			++it;
		}
	}
}


//============================================================================
void
Exchange::setExchangeOffset(size_t _offset) noexcept
{
	m_impl->setExchangeOffset(_offset);
}


//============================================================================
Option<size_t>
Exchange::getCloseIndex() const noexcept
{
	Option<size_t> close_index = std::nullopt;
	for (auto const& [key, value] : m_impl->headers)
	{
		auto toLower = [](String const& str) -> String {
			String lower_string;
			lower_string.resize(str.size());
			for (size_t i = 0; i < str.size(); i++)
			{
				lower_string[i] = std::tolower(str[i]);
			}
			return lower_string;
			};
		if (toLower(key) == "close")
		{
			close_index = value;
		}
	}
	return close_index;
}


//============================================================================
LinAlg::EigenConstColView<double>
Exchange::getSlice(size_t column, int row_offset) const noexcept
{
	assert(m_impl->current_index > 0);
	size_t idx = ((m_impl->current_index - 1) * m_impl->col_count) + column;
	if (row_offset)
	{
		size_t offset = abs(row_offset) * m_impl->col_count;
		assert(idx >= offset);
		idx -= offset;
	}
	assert(idx < static_cast<size_t>(m_impl->data.cols()));
	return m_impl->data.col(idx);
}


//============================================================================
Exchange::~Exchange()
{
}


//============================================================================
void
Exchange::registerStrategy(Strategy* strategy) noexcept
{
	m_impl->registered_strategies.push_back(strategy);
}


//============================================================================
size_t
Exchange::currentIdx() const noexcept
{
	assert(m_impl->current_index > 0);
	return (m_impl->current_index - 1);
}


//============================================================================
Eigen::VectorXd const&
Exchange::getReturnsScalar() const noexcept
{
	return m_impl->returns_scalar;
}


//============================================================================
SharedPtr<AST::CovarianceNodeBase>
Exchange::getCovarianceNode(
	String const& id,
	SharedPtr<AST::TriggerNode> trigger,
	size_t lookback,
	CovarianceType type
) noexcept
{
	// free and covariance nodes not being used and register the trigger
	// if it is not already registered
	cleanupCovarianceNodes();
	if (m_impl->covariance_nodes.count(id) > 0)
	{
		return m_impl->covariance_nodes[id];
	}
	auto it = std::find(m_impl->registered_triggers.begin(), m_impl->registered_triggers.end(), trigger);
	if (it == m_impl->registered_triggers.end())
	{
		trigger = registerTrigger(std::move(trigger));
	}

	SharedPtr<AST::CovarianceNodeBase> node;
	switch (type)
	{
		case CovarianceType::FULL:
			node = AST::CovarianceNode::make(*this, trigger, lookback);
			break;
		case CovarianceType::INCREMENTAL:
			node = AST::IncrementalCovarianceNode::make(*this, trigger, lookback);
			break;
	}
	m_impl->covariance_nodes[id] = node;
	return node;
}

//============================================================================
EigenConstRowView<double>
Exchange::getAssetSlice(size_t asset_index) const noexcept
{
	assert(asset_index < static_cast<size_t>(m_impl->data.rows()));
	return m_impl->data.row(asset_index);
}


//============================================================================
EigenConstColView<double>
Exchange::getMarketReturns(int offset) const noexcept
{
	assert(m_impl->current_index > 0);
	assert(offset <= 0);
	assert(static_cast<size_t>(abs(offset)) <= m_impl->current_index - 1);
	assert(m_impl->current_index - 1 + offset < static_cast<size_t>(m_impl->returns.cols()));
	return m_impl->returns.col(m_impl->current_index - 1 + offset);
}


//============================================================================
EigenBlockView<double>
Exchange::getMarketReturnsBlock(
	size_t start_idx,
	size_t end_idx) const noexcept
{
	assert(start_idx < end_idx);
	assert(end_idx < static_cast<size_t>(m_impl->returns.cols()));
	return m_impl->returns(
		Eigen::all,
		Eigen::seq(start_idx, end_idx)
	);
}


//============================================================================
Option<size_t>
Exchange::getColumnIndex(String const& column) const noexcept
{
	if (m_impl->headers.count(column) == 0)
	{
		return std::nullopt;
	}
	return m_impl->headers[column];
}

	
//============================================================================
size_t
Exchange::getExchangeOffset() const noexcept
{
	return m_impl->exchange_offset;
}


//============================================================================
Option<size_t>
Exchange::getAssetIndex(String const& asset) const noexcept
{
	if (m_impl->asset_id_map.count(asset) == 0)
	{
		return std::nullopt;
	}
	return m_impl->asset_id_map[asset];
}


//============================================================================
HashMap<String, size_t> const&
Exchange::getAssetMap() const noexcept
{
	return m_impl->asset_id_map;
}


//============================================================================
HashMap<String, size_t> const&
Exchange::getHeaders() const noexcept
{
	return m_impl->headers;
}

//============================================================================
size_t
Exchange::getAssetCount() const noexcept
{
	return m_impl->asset_id_map.size();
}


//============================================================================
Int64
Exchange::getCurrentTimestamp() const noexcept
{
	assert(m_impl->current_index > 0);
	return m_impl->current_timestamp;
}


//============================================================================
Vector<Int64> const&
Exchange::getTimestamps() const noexcept
{
	return m_impl->timestamps;
}

}