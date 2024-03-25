#pragma once

namespace Atlas {
//============================================================================
enum class TracerType { NLV, VOLATILITY, WEIGHTS, ORDERS_EAGER };

//============================================================================
enum TradeLimitType : unsigned int {
  NONE = 0,
  STOP_LOSS = 1 << 0,
  TAKE_PROFIT = 1 << 1
};

// Define bitwise OR assignment operator for TradeLimitType
inline TradeLimitType &operator|=(TradeLimitType &lhs, TradeLimitType rhs) {
  return lhs = static_cast<TradeLimitType>(static_cast<unsigned int>(lhs) |
                                           static_cast<unsigned int>(rhs));
}

inline TradeLimitType &operator&=(TradeLimitType &lhs, unsigned int rhs) {
  lhs = static_cast<TradeLimitType>(static_cast<unsigned int>(lhs) &
                                    static_cast<unsigned int>(rhs));
  return lhs;
}

//============================================================================
enum class CovarianceType { FULL, INCREMENTAL };

//============================================================================
enum class WeightScaleType {
  NO_SCALE = 0,
  VOLATILITY,
  EQUAL_RISK_CONTRIBUTION
};

//============================================================================
enum class GridType { FULL = 0, UPPER_TRIANGULAR = 1, LOWER_TRIANGULAR = 2 };

//============================================================================
enum class LogicalType : unsigned int { AND = 0, OR = 1 };

} // namespace Atlas