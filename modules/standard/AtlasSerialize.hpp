#pragma once
#ifdef ATLAS_EXPORTS
#define ATLAS_API __declspec(dllexport)
#else
#define ATLAS_API __declspec(dllimport)
#endif
#include <rapidjson/allocators.h>
#include <rapidjson/document.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include "standard/AtlasCore.hpp"

namespace Atlas
{


//============================================================================
 ATLAS_API Result<rapidjson::Value, AtlasException> serialize_hydra(
	rapidjson::Document::AllocatorType& allocator,
	Hydra const& hydra,
	Option<String> out_path = std::nullopt
) noexcept;


//============================================================================
 ATLAS_API Result<UniquePtr<Hydra>, AtlasException> deserialize_hydra(
	String const& path
) noexcept;


}

namespace Atlas
{


//============================================================================
Result<bool, AtlasException> deserialize_exchange_map(
	rapidjson::Document const& json,
	Hydra* hydra
) noexcept;


//============================================================================
rapidjson::Document serialize_exchange_map(
	rapidjson::Document::AllocatorType& allocator,
	ExchangeMap const& exchange_map
) noexcept;


//============================================================================
rapidjson::Document serialize_exchange(
	rapidjson::Document::AllocatorType& allocator,
	Exchange const& exchange
) noexcept;


//============================================================================
Result<bool, AtlasException> deserialize_exchange_map(
	rapidjson::Document const& json,
	Hydra* hydra
) noexcept;


}