#include <cstdint>
#include <vector>
#include <string>
#include <string_view>
#include <memory>
#include <optional>
#include <unordered_map>
#include <set>
#include "../external/expected.hpp"

namespace Atlas {
template <typename T> using UniquePtr = std::unique_ptr<T>;

template <typename T> using SharedPtr = std::shared_ptr<T>;

template <typename T, typename E> using Result = tl::expected<T, E>;

template <typename T> using Option = std::optional<T>;

template <typename K, typename V> using HashMap = std::unordered_map<K, V>;

template <typename T> using Vector = std::vector<T>;

template <typename T> using Err = tl::unexpected<T>;

template <typename T> using Set = std::set<T>;

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
class Strategy;
class MetaStrategy;
class Allocator;
class Tracer;
class CommisionManager;
class Hydra;
class Measure;

namespace Model {
class ModelBase;
class LinearRegressionModel;
} // namespace Model

namespace AST {
class AssetReadNode;
class AssetProductNode;
class AssetSumNode;
class AssetObserverNode;
class AssetScalerNode;
class AssetDifferenceNode;
class AssetQuotientNode;
class CovarianceNodeBase;
class IncrementalCovarianceNode;
class CovarianceNode;
class ExchangeViewNode;
class StrategyNode;
class StrategyGrid;
class StrategyBufferOpNode;
class GridDimension;
class GridDimensionObserver;
class GridDimensionLimit;
class TriggerNode;
class TradeLimitNode;
class AllocationWeightNode;
class AllocationBaseNode;
} // namespace AST
} // namespace Atlas