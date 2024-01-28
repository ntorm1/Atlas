module;
#pragma once
#ifdef ATLAS_EXPORTS
#define ATLAS_API __declspec(dllexport)
#else
#define ATLAS_API __declspec(dllimport)
#endif
#include <fstream>
#include <filesystem>
#include <rapidjson/allocators.h>
#include <rapidjson/document.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/writer.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <AtlasMacros.hpp>

module AtlasSerializeModule;

import HydraModule;
import ExchangeModule;
import ExchangeMapModule;
import PortfolioModule;

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
	auto master_portfolio_json = serialize_portfolios(allocator, hydra);
	j.AddMember("exchanges", std::move(exchange_map_json), allocator);
	j.AddMember("portfolios", std::move(master_portfolio_json), allocator);


	if (out_path)
	{
		rapidjson::StringBuffer buffer;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer); // Use PrettyWriter for indentation
		writer.SetIndent(' ', 4);
		j.Accept(writer);
		std::ofstream out(*out_path);
		if (!out.is_open())
		{
			return std::unexpected(AtlasException("Failed to open file: " + *out_path));
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
		return std::unexpected(AtlasException("File does not exist: " + path));
	}
	// validate it is json file
	if (std::filesystem::path(path).extension() != ".json")
	{
		return std::unexpected(AtlasException("File is not a json file: " + path));
	}
	// read it into rapidjson document
	std::ifstream in(path);
	String json_str((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
	rapidjson::Document doc;
	doc.Parse(json_str.c_str());
	if (doc.HasParseError())
	{
		return std::unexpected(AtlasException("Failed to parse json file: " + path));
	}
	if (!doc.HasMember("hydra_config"))
	{
		return std::unexpected(AtlasException("Json file does not have hydra_config key"));
	}

	UniquePtr<Hydra> hydra = std::make_unique<Hydra>();
	ATLAS_ASSIGN_OR_RETURN(res, deserialize_exchange_map(doc, hydra.get()));
	ATLAS_ASSIGN_OR_RETURN(res_p, deserialize_portfolios(doc, hydra.get()));
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
	rapidjson::Document j(rapidjson::kObjectType);

	rapidjson::Value v_id(exchange_id.c_str(), allocator);
	j.AddMember("exchange_id", v_id.Move(), allocator);

	rapidjson::Value v_source(source.c_str(), allocator);
	j.AddMember("source_dir", v_source.Move(), allocator);

	return j;
}


//============================================================================
rapidjson::Document serialize_portfolio(
	rapidjson::Document::AllocatorType& allocator,
	SharedPtr<Portfolio> const& portfolio) noexcept
{
	auto name = portfolio->getName();
	auto intial_cash = portfolio->getInitialCash();
	auto exchange_name = portfolio->getExchangeName();

	rapidjson::Document j(rapidjson::kObjectType);
	rapidjson::Value v_name(name.c_str(), allocator);
	j.AddMember("name", v_name.Move(), allocator);
	rapidjson::Value v_cash(std::to_string(intial_cash).c_str(), allocator);
	j.AddMember("initial_cash", v_cash.Move(), allocator);
	rapidjson::Value v_exchange(exchange_name.c_str(), allocator);
	j.AddMember("exchange", v_exchange.Move(), allocator);

	return j;	
}


//============================================================================
Result<bool, AtlasException>
deserialize_portfolios(rapidjson::Document const& json, Hydra* hydra) noexcept
{
	try {
		auto& hydra_config = json["hydra_config"];
		if (!hydra_config.HasMember("portfolios"))
		{
			return std::unexpected(AtlasException("Json file does not have portfolios"));
		}
		auto& portfolios = hydra_config["portfolios"];
		for (auto& portfolio : portfolios.GetObject())
		{
			auto name = portfolio.name.GetString();
			auto initial_cash = std::stod(portfolio.value["initial_cash"].GetString());
			auto exchange_name = portfolio.value["exchange"].GetString();
			auto exchange_opt = hydra->getExchange(exchange_name);
			if (!exchange_opt)
			{
				String msg = "Exchange does not exist: ";
				msg += exchange_name;
				return Err(msg.c_str());
			}
			auto res = hydra->addPortfolio(name, *(exchange_opt.value()), initial_cash);
			if (!res)
			{
				return Err(res.error().what());
			}
		}
		return true;
	}
	catch (std::exception& e)
	{
		return std::unexpected(AtlasException(e.what()));
	}
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
		return std::unexpected(AtlasException("Json file does not have exchanges"));
	}
	auto& exchanges = hydra_config["exchanges"];
	if (!exchanges.IsObject())
	{
		return std::unexpected(AtlasException("Json file does not have exchanges: "));
	}
	for (auto& exchange : exchanges.GetObject())
	{
		auto exchange_id = exchange.name.GetString();
		auto source = exchange.value["source_dir"].GetString();

		// load in init symbol list if it exists
		std::optional<std::vector<std::string>> _symbols;
		if (exchange.value.HasMember("symbols"))
		{
			auto& symbols = exchange.value["symbols"];
			if (!symbols.IsArray())
			{
				return std::unexpected(AtlasException("expected symbols to be array"));
			}
			std::vector<std::string> symbols_vec;
			for (auto& symbol : symbols.GetArray())
			{
				symbols_vec.push_back(symbol.GetString());
			}
			_symbols = symbols_vec;
		}

		auto res = hydra->addExchange(exchange_id, source);
		if (!res)
		{
			return std::unexpected(res.error());
		}
	}
	return true;
}


//============================================================================
rapidjson::Document serialize_portfolios(rapidjson::Document::AllocatorType& allocator, Hydra const& hydra) noexcept
{
	rapidjson::Document j(rapidjson::kObjectType);
	HashMap<String, size_t> portfolio_ids = hydra.getPortfolioIdxMap();
	for (const auto& [name, index] : portfolio_ids) {
		auto portfolio = hydra.pyGetPortfolio(name);
		auto json = serialize_portfolio(allocator,portfolio);
		rapidjson::Value key(name.c_str(), allocator);
		j.AddMember(key.Move(), std::move(json), allocator);
	}
	return j;
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