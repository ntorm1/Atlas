#pragma once
#ifdef ATLAS_EXPORTS
#define ATLAS_API __declspec(dllexport)
#else
#define ATLAS_API __declspec(dllimport)
#endif
#include "standard/AtlasCore.hpp"
#include "standard/AtlasLinAlg.hpp"

namespace Atlas {

namespace Stats {

class StatsBuilder {
private:
  size_t m_days_per_year = 252;
  double m_risk_free = 0.0;
  SharedPtr<Allocator> m_allocator;
  LinAlg::EigenVectorXd m_returns;
  double sharpe = 0.0;

public:
  StatsBuilder(SharedPtr<Allocator> allocator, double risk_free = 0,
               size_t days_per_year = 252) noexcept;
  ~StatsBuilder() noexcept;

  double getSharpe() const noexcept;
};

} // namespace Stats

} // namespace Atlas