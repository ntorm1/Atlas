#include <cmath>
#include "strategy/Allocator.hpp"
#include "strategy/Measure.hpp"
#include "standard/AtlasStats.hpp"


namespace Atlas {

namespace Stats {

static double stdv(LinAlg::EigenVectorXd const &data) {
  auto mean = data.mean();
  double sum_squared_diff = 0.0;
  for (const auto &value : data) {
    sum_squared_diff += std::pow(value - mean, 2);
  }
  return std::sqrt(static_cast<double>(sum_squared_diff) / data.size());
}

StatsBuilder::StatsBuilder(SharedPtr<Allocator> allocator, double risk_free,
                           size_t days_per_year) noexcept
    : m_risk_free(risk_free), m_days_per_year(days_per_year) {
  auto const nlv_opt = allocator->getMeasure(TracerType::NLV);
  if (!nlv_opt) {
    return;
  }
  auto nlv = (*nlv_opt)->getValues().col(0);
  if (nlv.size() < 2) {
    m_returns.resize(0);
    m_returns.setZero();
    return;
  }
  m_returns.resize(nlv.size() - 1);
  for (int i = 1; i < nlv.size(); i++) {
    m_returns[i - 1] = (nlv[i] - nlv[i - 1]) / nlv[i - 1];
  }
}

StatsBuilder::~StatsBuilder() noexcept {}

double StatsBuilder::getSharpe() const noexcept {
  return (m_returns.mean() - m_risk_free) / stdv(m_returns);
}

} // namespace Stats

} // namespace Atlas