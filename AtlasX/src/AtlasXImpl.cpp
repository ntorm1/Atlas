#include <fstream>

#include "../include/AtlasXImpl.h"


#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include "../include/AtlasXImpl.h"


import HydraModule;
import AtlasSerializeModule;
import AtlasTimeModule;
import ExchangeMapModule;
import ExchangeModule;

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
void
AtlasXAppImpl::run() noexcept
{
	hydra->run();
}


//============================================================================
Result<SharedPtr<Atlas::Exchange>, Atlas::AtlasException>
AtlasXAppImpl::addExchange(String name, String source) noexcept
{
	return hydra->addExchange(name, source);
}


//============================================================================
Result<SharedPtr<Atlas::Exchange>, Atlas::AtlasException>
AtlasXAppImpl::getExchange(String name) noexcept
{
	return hydra->getExchange(name);
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
HashMap<String, size_t>
AtlasXAppImpl::getExchangeIds() noexcept
{
	auto const& exchange_map = hydra->getExchangeMap();
	return exchange_map.getExchangeIds();
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
AtlasXAppImpl::getParentExchangeName(String const& asset_name) const noexcept
{
	return hydra->getParentExchangeName(asset_name);
}


//============================================================================
HashMap<String, size_t>
AtlasXAppImpl::getAssetMap(SharedPtr<Atlas::Exchange> e) noexcept
{
	return e->getAssetMap();
}

}