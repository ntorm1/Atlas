#include "exchange/Exchange.hpp"
#include "ast/RiskNode.hpp"
#include "strategy/Tracer.hpp"
#include "strategy/Measure.hpp"
#include "strategy/Allocator.hpp"

namespace Atlas {

void Measure::reset() noexcept { m_values.setZero(); }

Measure::Measure(TracerType type, Exchange const &exchange,
                 Tracer const *tracer) noexcept
    : m_exchange(&exchange), m_type(type), m_tracer(tracer)
       {
  size_t rows = m_tracer->getExchange().getTimestamps().size();
  m_values.resize(rows,1);
}

Measure::~Measure() noexcept {}

void NLVMeasure::measure(size_t m_idx) noexcept {
  m_values(m_idx, 0) = m_tracer->getNLV();
}

NLVMeasure::NLVMeasure(Exchange const &exchange, Tracer const *tracer) noexcept
    : Measure(TracerType::NLV, exchange, tracer) {}

NLVMeasure::~NLVMeasure() noexcept {}

void WeightMeasure::measure(size_t m_idx) noexcept {
  m_values.col(m_idx) = m_tracer->getAllocator()->getAllocationBuffer();
}

WeightMeasure::WeightMeasure(Exchange const &exchange,
                             Tracer const *tracer) noexcept
    : Measure(TracerType::WEIGHTS, exchange, tracer) {
  size_t rows = tracer->getExchange().getTimestamps().size();
  m_values.resize(tracer->getExchange().getAssetCount(), rows);
}

WeightMeasure::~WeightMeasure() noexcept {}

void VolatilityMeasure::measure(size_t m_idx) noexcept {
  auto const &cov = m_tracer->getCovariance();
  if (!cov) {
    return;
  }
  if (m_idx > (*cov)->getWarmup()) {
    LinAlg::EigenMatrixXd const &covariance = (*cov)->getCovariance();
    auto weights = m_tracer->getAllocator()->getAllocationBuffer();
    auto current_vol = (weights.transpose() * covariance * weights);
    double current_volatility = std::sqrt(current_vol(0)) * std::sqrt(252);
    m_values(m_idx, 0) = current_volatility;
  }
}

VolatilityMeasure::VolatilityMeasure(Exchange const &exchange,
  																		 Tracer const *tracer, size_t days_per_year) noexcept
		: Measure(TracerType::VOLATILITY, exchange, tracer),
      m_days_per_year(days_per_year) {}

VolatilityMeasure::~VolatilityMeasure() noexcept {}

} // namespace Atlas