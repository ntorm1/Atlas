module;
#include <cassert>
#include "AtlasMacros.hpp"
module ExchangeMapModule;

import ExchangeModule;
import AtlasUtilsModule;

namespace Atlas
{

struct ExchangeMapImpl
{
	HashMap<String, size_t> exchange_id_map;
	Vector<SharedPtr<Exchange>> exchanges;
	Vector<Int64> timestamps;
	Int64 global_time;
	size_t current_index = 0;
};


//============================================================================
void
ExchangeMap::build() noexcept
{
	m_impl->timestamps.clear();
	for (auto const& exchange: m_impl->exchanges)
	{
		auto exchange_timestamps = exchange->getTimestamps();
		m_impl->timestamps = sortedUnion(m_impl->timestamps, exchange_timestamps);
	}
}


//============================================================================
void
ExchangeMap::reset() noexcept
{
	m_impl->current_index = 0;
	m_impl->global_time = 0;
	for (auto& exchange: m_impl->exchanges)
	{
		exchange->reset();
	}
}


//============================================================================
Vector<Int64> const& ExchangeMap::getTimestamps() const noexcept
{
	return m_impl->timestamps;
}


//============================================================================
ExchangeMap::ExchangeMap() noexcept
{
	m_impl = std::make_unique<ExchangeMapImpl>();
}


//============================================================================
ExchangeMap::~ExchangeMap() noexcept
{
}


//============================================================================
HashMap<String, size_t>
const& ExchangeMap::getExchangeIds() const noexcept
{
	return m_impl->exchange_id_map;
}


//============================================================================
Result<SharedPtr<const Exchange>, AtlasException>
ExchangeMap::getExchangeConst(String const& name) const noexcept
{
	return getExchange(name);
}


//============================================================================
Option<String>
ExchangeMap::getParentExchangeName(String const& asset_name) const noexcept
{
	for (auto const& exchange: m_impl->exchanges)
	{
		if (exchange->getAssetMap().count(asset_name))
		{
			return exchange->getName();
		}
	}
	return std::nullopt;
}


//============================================================================
Result<SharedPtr<Exchange>, AtlasException>
ExchangeMap::addExchange(String name, String source) noexcept
{
	EXPECT_FALSE(
		m_impl->exchange_id_map.contains(name),
		"Exchange with name already exists"
	);
	auto exchange = std::make_unique<Exchange>(
		std::move(name),
		std::move(source),
		m_impl->exchanges.size()
	);
	EXPECT_TRUE(res, exchange->init());
	EXPECT_TRUE(res_val, exchange->validate());
	EXPECT_TRUE(res_build, exchange->build());
	SAFE_MAP_INSERT(m_impl->exchange_id_map, exchange->getName(), m_impl->exchanges.size());
	m_impl->exchanges.push_back(std::move(exchange));
	return m_impl->exchanges.back();
}


//============================================================================
Result<SharedPtr<Exchange>, AtlasException>
ExchangeMap::getExchange(String const& name) const noexcept
{
	if (m_impl->exchange_id_map.contains(name))
	{
		return m_impl->exchanges[m_impl->exchange_id_map[name]];
	}
	return std::unexpected<AtlasException>("Exchange not found");
}


//============================================================================
size_t
ExchangeMap::getCurrentIdx() const noexcept
{
	return m_impl->current_index;
}


//============================================================================
void
ExchangeMap::step() noexcept
{
	assert(m_impl->current_index < m_impl->timestamps.size());
	m_impl->global_time = m_impl->timestamps[m_impl->current_index];
	for (auto& exchange: m_impl->exchanges)
	{
		exchange->step(m_impl->global_time);
	}
	++m_impl->current_index;
}

}