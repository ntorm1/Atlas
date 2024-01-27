#include <fstream>

#include "../include/AtlasXImpl.h"


#include <rapidjson/document.h>
#include <rapidjson/writer.h>


import HydraModule;
import AtlasSerializeModule;
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
HashMap<String, size_t>
AtlasXAppImpl::getExchangeIds() noexcept
{
	auto const& exchange_map = hydra->getExchangeMap();
	return exchange_map.getExchangeIds();
}


//============================================================================
HashMap<String, size_t>
AtlasXAppImpl::getAssetMap(SharedPtr<Atlas::Exchange> e) noexcept
{
	return e->getAssetMap();
}

}