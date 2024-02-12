module;
#include <cstdint>
export module AtlasTypes;

import <vector>;
import <string>;
import <string_view>;
import <memory>;
import <optional>;
import <unordered_map>;
import <expected>;
import <set>;


export namespace Atlas
{

	// template aliases
	template <typename T>
	using UniquePtr = std::unique_ptr<T>;

	template <typename T>
	using SharedPtr = std::shared_ptr<T>;

	template <typename T, typename E>
	using Result = std::expected<T, E>;

	template <typename T>
	using Option = std::optional<T>;

	template <typename K, typename V>
	using HashMap = std::unordered_map<K, V>;

	template <typename T>
	using Vector = std::vector<T>;

	template <typename T>
	using Err = std::unexpected<T>;

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


	// forward declarations
	class Exchange;
	class ExchangeMap;
	struct Asset;
	class Portfolio;
	class Strategy;
	class Tracer;
	class CommisionManager;
	class Hydra;

	namespace AST
	{
		class AssetReadNode;
		class AssetProductNode;
		class AssetSumNode;
		class AssetDifferenceNode;
		class AssetQuotientNode;
		class CovarianceNodeBase;
		class IncrementalCovarianceNode;
		class CovarianceNode;
		class ExchangeViewNode;
		class StrategyNode;
		class StrategyGrid;
		struct GridDimension;
		class TriggerNode;
		class TradeLimitNode;
		class AllocationWeightNode;
		class AllocationBaseNode;
	}
}