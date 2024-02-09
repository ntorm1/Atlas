#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <set>
#include <expected>

namespace AtlasX
{

	// template aliases
	template <typename T>
	using UniquePtr = std::unique_ptr<T>;

	template <typename T>
	using SharedPtr = std::shared_ptr<T>;

	template <typename T>
	using Option = std::optional<T>;

	template <typename K, typename V>
	using HashMap = std::unordered_map<K, V>;

	template <typename T>
	using Vector = std::vector<T>;

	template <typename T, typename E>
	using Result = std::expected<T, E>;

	template <typename T>
	using Set = std::set<T>;

	using String = std::string;
	using StringRef = std::string_view;

	using Int = int;
	using Int8 = int8_t;
	using Int16 = int16_t;
	using Int32 = int32_t;
	using Int64 = int64_t;

	using Uint = unsigned int;
	using Uint8 = uint8_t;
	using Uint16 = uint16_t;
	using Uint32 = uint32_t;
	using Uint64 = uint64_t;

	class AtlasXAppImpl;
	class AtlasXExchangeManager;
	class AtlasXStrategyManager;
	class AtlasXExchange;
	class AtlasXAsset;
}

namespace Atlas
{

	// forward declarations
	class Exchange;
	class ExchangeMap;
	struct Asset;
	class Portfolio;
	class Strategy;
	class Tracer;
	class Hydra;
	class CommisionManager;

	namespace AST
	{
		class AssetReadNode;
		class AssetProductNode;
		class AssetSumNode;
		class AssetDifferenceNode;
		class AssetQuotientNode;
		class ExchangeViewNode;
		class StrategyNode;
		class TriggerNode;
		class TradeLimitNode;
		class AllocationWeightNode;
		class AllocationBaseNode;
	}
}