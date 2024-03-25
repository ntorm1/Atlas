#include <fstream>
#include <filesystem>
#include <rapidjson/allocators.h>
#include <rapidjson/document.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/writer.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <AtlasMacros.hpp>

#include "standard/AtlasSerialize.hpp"
#include "hydra/Hydra.hpp"
#include "exchange/Exchange.hpp"
#include "exchange/ExchangeMap.hpp"

namespace Atlas 
{


//============================================================================
Result<rapidjson::Value, AtlasException>
serialize_hydra(
	rapidjson::Document::AllocatorType& allocator,
	Hydra const& hydra,
	Option<std::string> out_path
) noexcept
{
	rapidjson::Value j(rapidjson::kObjectType);
	auto exchange_map_json = serialize_exchange_map(allocator, hydra.getExchangeMap());
	j.AddMember("exchanges", std::move(exchange_map_json), allocator);

	if (out_path)
	{
		rapidjson::StringBuffer buffer;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer); // Use PrettyWriter for indentation
		writer.SetIndent(' ', 4);
		j.Accept(writer);
		std::ofstream out(*out_path);
		if (!out.is_open())
		{
			return Err(AtlasException("Failed to open file: " + *out_path));
		}
		out << buffer.GetString();
		out.close();
	}
	return j;
}



//============================================================================
Result<UniquePtr<Hydra>, AtlasException>
deserialize_hydra(String const& path) noexcept
{
	// validate path exists
	if (!std::filesystem::exists(path))
	{
		return Err(AtlasException("File does not exist: " + path));
	}
	// validate it is json file
	if (std::filesystem::path(path).extension() != ".json")
	{
		return Err(AtlasException("File is not a json file: " + path));
	}
	// read it into rapidjson document
	std::ifstream in(path);
	String json_str((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
	rapidjson::Document doc;
	doc.Parse(json_str.c_str());
	if (doc.HasParseError())
	{
		return Err(AtlasException("Failed to parse json file: " + path));
	}
	if (!doc.HasMember("hydra_config"))
	{
		return Err(AtlasException("Json file does not have hydra_config key"));
	}

	UniquePtr<Hydra> hydra = std::make_unique<Hydra>();
	ATLAS_ASSIGN_OR_RETURN(res, deserialize_exchange_map(doc, hydra.get()));
	return std::move(hydra);
}


//============================================================================
rapidjson::Document
serialize_exchange(
	rapidjson::Document::AllocatorType& allocator,
	Exchange const& exchange) noexcept
{
	auto exchange_id = exchange.getName();
	auto source = exchange.getSource();
	auto datetime_format = exchange.getDatetimeFormat();
	rapidjson::Document j(rapidjson::kObjectType);

	rapidjson::Value v_id(exchange_id.c_str(), allocator);
	j.AddMember("exchange_id", v_id.Move(), allocator);

	rapidjson::Value v_source(source.c_str(), allocator);
	j.AddMember("source_dir", v_source.Move(), allocator);

	if (datetime_format)
	{
		rapidjson::Value v_datetime_format(datetime_format.value().c_str(), allocator);
		j.AddMember("datetime_format", v_datetime_format.Move(), allocator);
	}

	return j;
}


//============================================================================
Result<bool, AtlasException>
deserialize_exchange_map(
	rapidjson::Document const& json,
	Hydra* hydra
) noexcept
{
	// get hydra_config value from document
	auto& hydra_config = json["hydra_config"];
	if (!hydra_config.HasMember("exchanges"))
	{
		return Err(AtlasException("Json file does not have exchanges"));
	}
	auto& exchanges = hydra_config["exchanges"];
	if (!exchanges.IsObject())
	{
		return Err(AtlasException("Json file does not have exchanges: "));
	}
	for (auto& exchange : exchanges.GetObject())
	{
		auto exchange_id = exchange.name.GetString();
		auto source = exchange.value["source_dir"].GetString();
		Option<String> datetime_format;
		if (exchange.value.HasMember("datetime_format"))
		{
			datetime_format = exchange.value["datetime_format"].GetString();
		}

		// load in init symbol list if it exists
		std::optional<std::vector<std::string>> _symbols;
		if (exchange.value.HasMember("symbols"))
		{
			auto& symbols = exchange.value["symbols"];
			if (!symbols.IsArray())
			{
				return Err(AtlasException("expected symbols to be array"));
			}
			std::vector<std::string> symbols_vec;
			for (auto& symbol : symbols.GetArray())
			{
				symbols_vec.push_back(symbol.GetString());
			}
			_symbols = symbols_vec;
		}

		auto res = hydra->addExchange(exchange_id, source, datetime_format);
		if (!res)
		{
			return Err(res.error());
		}
	}
	return true;
}


//============================================================================
rapidjson::Document
serialize_exchange_map(
	rapidjson::Document::AllocatorType& allocator,
	ExchangeMap const& exchange_map) noexcept
{
	rapidjson::Document j(rapidjson::kObjectType);
	auto const& exchange_ids = exchange_map.getExchangeIds();
	for (const auto& [id, index] : exchange_ids) {
		auto exchange = exchange_map.getExchangeConst(id).value();
		auto json = serialize_exchange(allocator, *exchange);
		rapidjson::Value key(id.c_str(), allocator);
		j.AddMember(key.Move(), std::move(json), allocator);
	}
	return j;
}


}