module;
#include "AtlasMacros.hpp"
#include <Eigen/Dense>
module ExchangeModule;

import ExchangePrivateModule;
import StrategyModule;
import AtlasUtilsModule;

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
			EXPECT_FALSE(
				is_descending,
				"Asset timestamps are not in ascending order"
			);
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

	// get all headers in lowercase
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
	// store integer flags for count of tradeable history for each asset at each timestamp
	m_impl->tradeable.resize(
		m_impl->assets.size(),
		m_impl->timestamps.size()
	);


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
				// fill tradeable matrix with 0 if first timestamp, else copy previous value
				if (exchange_index == 0)
				{
					m_impl->tradeable(asset_id, exchange_index) = 0;
				}
				else
				{
					m_impl->tradeable(asset_id, exchange_index) = m_impl->tradeable(asset_id, exchange_index - 1);
				}
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
					m_impl->returns(asset_id, exchange_index) = ret;
				}
				asset_index++;

				// update tradeable matrix
				if (exchange_index == 0)
				{
					m_impl->tradeable(asset_id, exchange_index) = 1;
				}
				else
				{
					m_impl->tradeable(asset_id, exchange_index) = m_impl->tradeable(asset_id, exchange_index - 1) + 1;
				}
			}
		}
	}
	m_impl->assets.clear();
	return true;
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

	m_impl->current_index++;

}


//============================================================================
LinAlg::EigenConstColView<double>
Exchange::getSlice(size_t column, int row_offset) const noexcept
{
	size_t idx = ((m_impl->current_index - 1) * m_impl->col_count) + column;
	if (row_offset)
	{
		idx -= abs(row_offset) * m_impl->col_count;
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
EigenConstColView<int> Exchange::getTradeable() const noexcept
{
	return m_impl->tradeable.col(m_impl->current_index - 1);
}


//============================================================================
EigenConstColView<double>
Exchange::getMarketReturns() const noexcept
{
	return m_impl->returns.col(m_impl->current_index - 1);
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
size_t
Exchange::getAssetCount() const noexcept
{
	return m_impl->asset_id_map.size();
}


//============================================================================
Vector<Int64> const&
Exchange::getTimestamps() const noexcept
{
	return m_impl->timestamps;
}

}