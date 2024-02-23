#include <fstream>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include "../include/AtlasXImpl.h"


import AtlasSerializeModule;
import AtlasTimeModule;
import ExchangeMapModule;
import ExchangeModule;
import HydraModule;
import OptimizeNodeModule;
import StrategyModule;
import StrategyBufferModule;
import TracerModule;

namespace AtlasX
{


//============================================================================
AtlasXAppImpl::AtlasXAppImpl()
{
	hydra = std::make_shared<Atlas::Hydra>();

}


//============================================================================
AtlasXAppImpl::~AtlasXAppImpl()
{

}


//============================================================================
void
AtlasXAppImpl::step() noexcept
{
	hydra->step();
}


//============================================================================
Result<bool, Atlas::AtlasException>
AtlasXAppImpl::run() noexcept
{
	return hydra->run();
}


//============================================================================
Result<SharedPtr<Atlas::Exchange>, Atlas::AtlasException>
AtlasXAppImpl::addExchange(
	String name,
	String source,
	Option<String> datetime_format
) noexcept
{
	return hydra->addExchange(name, source, datetime_format);
}


//============================================================================
Result<SharedPtr<Atlas::Portfolio>, Atlas::AtlasException>
AtlasXAppImpl::addPortfolio(String name, SharedPtr<Atlas::Exchange> exchange, double intial_cash) noexcept
{
	return hydra->addPortfolio(name, *exchange, intial_cash);
}


//============================================================================
Result<SharedPtr<Atlas::Portfolio>, Atlas::AtlasException>
AtlasXAppImpl::getPortfolio(String name) noexcept
{
	try {
		return hydra->pyGetPortfolio(name);
	}
	catch (...) {
		return std::unexpected<Atlas::AtlasException>("portfolio does not exsist");
	}
}


//============================================================================
Option<SharedPtr<Atlas::Strategy>> AtlasXAppImpl::getStrategy(String const& strategy_name) noexcept
{
	return hydra->getStrategy(strategy_name);
}

//============================================================================
Result<SharedPtr<Atlas::Exchange>, Atlas::AtlasException>
AtlasXAppImpl::getExchange(String name) noexcept
{
	return hydra->getExchange(name);
}

Result<bool, Atlas::AtlasException> AtlasXAppImpl::removeExchange(String name) noexcept
{
	return hydra->removeExchange(name);
}


//============================================================================
HashMap<String, size_t>
AtlasXAppImpl::getPortfolioIdxMap() const noexcept
{
	return hydra->getPortfolioIdxMap();
}


//============================================================================
HashMap<String, SharedPtr<Atlas::AST::StrategyBufferOpNode>>
AtlasXAppImpl::getASTCache(SharedPtr<Atlas::Exchange> exchange) const noexcept
{
	return exchange->getASTCache();
}


//============================================================================
Result<bool, Atlas::AtlasException>
AtlasXAppImpl::deserialize(String path) noexcept
{
	auto res = Atlas::deserialize_hydra(path);
	if (res)
	{
		UniquePtr<Atlas::Hydra> new_hydra = std::move(res.value());
		hydra = std::move(new_hydra);
		return true;
	}
	else
	{
		return std::unexpected<Atlas::AtlasException>(res.error().what());
	}
}


//============================================================================
Result<bool, Atlas::AtlasException>
AtlasXAppImpl::serialize(String path) noexcept
{
	rapidjson::Document doc;
	doc.SetObject();  // Create a JSON object to store the data.
	auto& allocator = doc.GetAllocator();
	auto res = Atlas::serialize_hydra(allocator, *hydra);

	if (!res)
	{
		return std::unexpected<Atlas::AtlasException>(res.error().what());
	}
	doc.AddMember("hydra_config", res.value(), allocator);

	// Dump the JSON output to a file
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	doc.Accept(writer);
	std::string jsonString = buffer.GetString();
	std::ofstream outputFile(path);

	if (outputFile.is_open()) {
		outputFile << jsonString;
		outputFile.close();
		return true;
	}
	else {
		auto msg = "Failed to open file for writing: " + path;
		return std::unexpected<Atlas::AtlasException>(msg);
	}
}


//============================================================================
Result<bool, Atlas::AtlasException>
AtlasXAppImpl::build() noexcept
{
	return hydra->build();
}


//============================================================================
Result<bool, Atlas::AtlasException>
AtlasXAppImpl::reset() noexcept
{
	return hydra->reset();
}


//============================================================================
Result<Atlas::LinAlg::EigenConstRowView<double>, Atlas::AtlasException>
AtlasXAppImpl::getAssetSlice(String const& asset_name) const noexcept
{
	auto parent_name = getParentExchangeName(asset_name);
	if (!parent_name)
	{
		return std::unexpected<Atlas::AtlasException>("failed to find parent exchange");
	}
	auto exchange = hydra->getExchange(parent_name.value()).value();
	auto asset_index = exchange->getAssetIndex(asset_name);
	return exchange->getAssetSlice(*asset_index);
}


//============================================================================
Option<Vector<double>>
AtlasXAppImpl::getCacheSlice(
	SharedPtr<Atlas::Exchange> exchange,
	String const& asset_name,
	SharedPtr<Atlas::AST::StrategyBufferOpNode> node
) noexcept
{
	auto asset_index_opt = exchange->getAssetIndex(asset_name);
	if (!asset_index_opt)
		return std::nullopt;
	auto asset_index = asset_index_opt.value();
	auto const& cache = node->cache();
	if (asset_index >= cache.rows())
		return std::nullopt;
	if (cache.cols() <= 1)
		return std::nullopt;
	Vector<double> slice(cache.cols());
	for (size_t i = 0; i < cache.cols(); ++i)
	{
		slice[i] = cache(asset_index, i);
	}
	return std::move(slice);
}


//============================================================================
HashMap<String, size_t>
AtlasXAppImpl::getExchangeIds() noexcept
{
	auto const& exchange_map = hydra->getExchangeMap();
	return exchange_map.getExchangeIds();
}


//============================================================================
size_t*
AtlasXAppImpl::getCurrentIdxPtr() const noexcept
{
	return hydra->getCurrentIdxPtr();
}

//============================================================================
HashMap<String, size_t> const&
AtlasXAppImpl::getExchangeHeaders(SharedPtr<Atlas::Exchange> exchange) noexcept
{
	return exchange->getHeaders();
}


//============================================================================
Vector<Int64> const&
AtlasXAppImpl::getTimestamps(SharedPtr<Atlas::Exchange> exchange) noexcept
{
	return exchange->getTimestamps();
}


//============================================================================
Vector<Int64> const&
AtlasXAppImpl::getTimestamps() noexcept
{
	return hydra->getTimestamps();
}


//============================================================================
QStringList const& AtlasXAppImpl::getTimestampsStr(SharedPtr<Atlas::Exchange> exchange) noexcept
{
	if (timestamp_cache.contains(exchange->getName()))
	{
		return timestamp_cache[exchange->getName()];
	}
	else {
		timestamp_cache[exchange->getName()] = QStringList(exchange->getTimestamps().size());
	}

	auto const& timestamps = exchange->getTimestamps();
	auto& timestamps_str = timestamp_cache[exchange->getName()];
	for (size_t i = 0; i < timestamps.size(); ++i)
	{
		String time = Atlas::Time::convertNanosecondsToTime(timestamps[i]);
		timestamps_str[i] = QString::fromStdString(time);
	}
	return timestamps_str;
}


//============================================================================
Int64
AtlasXAppImpl::currentGlobalTime() const noexcept
{
	return hydra->currentGlobalTime();
}


//============================================================================
Int64
AtlasXAppImpl::nextGlobalTime() const noexcept
{
	return hydra->nextGlobalTime();
}


//============================================================================
size_t
AtlasXAppImpl::getCurrentIdx() const noexcept
{
	return hydra->getCurrentIdx();
}


//============================================================================
String
AtlasXAppImpl::convertNanosecondsToTime(Int64 nanoseconds)
{
	return Atlas::Time::convertNanosecondsToTime(nanoseconds);
}

//============================================================================
Option<String>
AtlasXAppImpl::getStrategyParentExchange(String const& strategy_name) const noexcept
{
	auto strategy = hydra->getStrategy(strategy_name);
	if (!strategy)
	{
		return std::nullopt;
	}
	return (*strategy)->getExchange().getName();
}


//============================================================================
Atlas::LinAlg::EigenVectorXd const&
AtlasXAppImpl::getStrategyMeasure(
	String const& strategy_name,
	String const& measure
) const noexcept
{
	auto strategy = hydra->getStrategy(strategy_name);
	assert(strategy);
	if (measure == "NLV")
	{
		return (*strategy)->getHistory(Atlas::TracerType::NLV);
	}
	else if (measure == "Volatility")
	{
		return (*strategy)->getHistory(Atlas::TracerType::VOLATILITY);
	}
	return (*strategy)->getHistory(Atlas::TracerType::NLV); //TODO fix 
}


//============================================================================
Option<String>
AtlasXAppImpl::getParentExchangeName(String const& asset_name) const noexcept
{
	return hydra->getParentExchangeName(asset_name);
}


//============================================================================
Option<String>
AtlasXAppImpl::getParentExchangeName(SharedPtr<Atlas::Strategy> strategy) const noexcept
{
	return strategy->getExchange().getName();
}


//============================================================================
HashMap<String, size_t>
AtlasXAppImpl::getAssetMap(SharedPtr<Atlas::Exchange> e) noexcept
{
	return e->getAssetMap();
}


//============================================================================
Option<SharedPtr<GridState>>
AtlasXAppImpl::getStrategyGridState(String const& strategy_name) noexcept
{
	auto strategy = hydra->getStrategy(strategy_name);
	if (!strategy)
	{
		return std::nullopt;
	}
	auto grid = (*strategy)->getGrid();
	if (!grid)
	{
		return std::nullopt;
	}
	auto const& grid_ref = grid.value();
	auto const& dims = grid_ref->getDimensions();
	Vector<double> x = dims.first->getValues();
	Vector<double> y = dims.second->getValues();
	String x_label = dims.first->getName();
	String y_label = dims.second->getName();

	auto const& tracers = grid_ref->getTracers();
	Vector<Vector<double>> z;
	z.reserve(x.size());
	for (size_t i = 0; i < x.size(); ++i)
	{
		Vector<double> row;
		row.reserve(y.size());
		for (size_t j = 0; j < y.size(); ++j)
		{
			auto const& tracer = tracers(i, j);
			double returns = tracer->getNLV();
			row.push_back(returns);
		}
		z.push_back(row);
	}
	return std::make_shared<GridState>(
		"Returns",
		x_label,
		y_label,
		x,
		y,
		z
	);
}


//============================================================================
Option<size_t>
AtlasXAppImpl::getAssetIndex(String const& asset_name) noexcept
{
	Option<size_t> asset_id = std::nullopt;
	auto parent_name = getParentExchangeName(asset_name);
	if (!parent_name)
	{
		return std::nullopt;
	}
	auto exchange = hydra->getExchange(parent_name.value());
	if (!exchange)
	{
		return std::nullopt;
	}
	asset_id = (*exchange)->getAssetIndex(asset_name);
	if (!asset_id)
	{
		return std::nullopt;
	}
	return asset_id;
}


//============================================================================
Vector<Atlas::Order>
AtlasXAppImpl::getOrders(
	Option<String> asset_name,
	Option<String> strategy_name
) noexcept
{
	Vector<Atlas::Order> orders;
	auto strategy_map = hydra->getStrategyIdxMap();
	if (strategy_name)
	{
		for (auto it = strategy_map.begin(); it != strategy_map.end();) {
			if (it->first != *strategy_name) {
				it = strategy_map.erase(it);
			}
			else {
				++it;
			}
		}
	}

	Option<size_t> asset_id = std::nullopt;
	if (asset_name)
	{
		asset_id = getAssetIndex(*asset_name);
		if (!asset_id)
		{
			return orders;
		}
	}

	for (auto const& [name, idx] : strategy_map)
	{
		auto strategy = hydra->getStrategy(name);
		if (!strategy)
		{
			continue;
		}
		auto strategy_orders = (*strategy)->getTracer().getOrders();
		if (!strategy_orders.size())
		{
			continue;
		}
		for (auto const& order : strategy_orders)
		{
			if (asset_id && order.asset_id != *asset_id)
			{
				continue;
			}
			orders.push_back(order);
		}
	}
	return orders;
}

}