module;
#include "AtlasMacros.hpp"
#include <Eigen/Dense>
module TracerModule;

import ExchangeModule;
import AtlasAllocatorModule;
import MeasureModule;

namespace Atlas {

//============================================================================
Tracer::Tracer(Allocator const *allocator, Exchange const &exchange,
               double cash) noexcept
    : m_exchange(exchange), m_allocator(allocator) {
  m_cash = cash;
  m_nlv = cash;
  m_initial_cash = cash;
  m_weights_buffer.resize(exchange.getAssetCount());
  m_weights_buffer.setZero();
}

//============================================================================
void Tracer::evaluate() noexcept {
  for (auto &m : m_measures) {
    m->measure(m_idx);
  }
  if (m_struct_tracer && m_struct_tracer.value()->eager()) {
    m_struct_tracer.value()->evaluate(m_allocator->getAllocationBuffer(),
                                      m_weights_buffer);
  }
  m_idx++;
}

//============================================================================
void Tracer::initPnL() noexcept {
  m_pnl.resize(m_exchange.getAssetCount());
  m_pnl.setZero();
}

//============================================================================
LinAlg::EigenVectorXd &Tracer::getPnL() noexcept {
  if (!m_pnl.rows()) {
    initPnL();
  }
  return m_pnl;
}

//============================================================================
void Tracer::reset() noexcept {
  m_cash = m_initial_cash;
  m_nlv = m_initial_cash;
  for (auto &m : m_measures) {
    m->reset();
  }
  if (m_struct_tracer) {
    m_struct_tracer.value()->reset();
  }
  m_weights_buffer.setZero();
  m_pnl.setZero();
  m_idx = 0;
}

//============================================================================
Result<bool, AtlasException>
Tracer::enableTracerHistory(TracerType t) noexcept {
  size_t n = m_exchange.getTimestamps().size();
  switch (t) {
  case TracerType::NLV:
    m_measures.push_back(std::make_shared<NLVMeasure>(m_exchange, this));
    break;
  case TracerType::WEIGHTS:
    m_measures.push_back(std::make_shared<WeightMeasure>(m_exchange, this));
    break;
  case TracerType::VOLATILITY:
    if (!m_covariance) {
      return Err("Covariance matrix not set");
    }
    m_measures.push_back(
				std::make_shared<VolatilityMeasure>(m_exchange, this, 252));
    break;
  case TracerType::ORDERS_EAGER:
    if (!m_struct_tracer) {
      m_struct_tracer = std::make_unique<StructTracer>(m_exchange, *this);
    }
    m_struct_tracer.value()->enabelTracerHistory(t);
    break;
  }
  return true;
}

//============================================================================
void Tracer::setPnL(LinAlg::EigenRef<LinAlg::EigenVectorXd> pnl) noexcept {
  m_pnl = pnl;
}

//============================================================================
void Tracer::realize() noexcept {}

//============================================================================
Vector<Order> const &Tracer::getOrders() const noexcept {
  return m_struct_tracer.value()->m_orders;
}

//============================================================================
Option<SharedPtr<Measure>>
Tracer::getMeasure(TracerType t) const noexcept {
  for (auto& m : m_measures) {
    if (m->getType() == t) {
			return m;
		}
	}
  return std::nullopt;
}

//============================================================================
StructTracer::StructTracer(Exchange const &exchange,
                           Tracer const &tracer) noexcept
    : m_exchange(exchange), m_tracer(tracer) {
  auto close_idx = exchange.getCloseIndex();
  assert(close_idx);
  close_index = *close_idx;
}

//============================================================================
StructTracer::~StructTracer() noexcept {}

//============================================================================
void StructTracer::enabelTracerHistory(TracerType t) noexcept {
  switch (t) {
  case TracerType::ORDERS_EAGER:
    orders_eager = true;
    break;
  default:
    return;
  }
}

//============================================================================
void StructTracer::evaluate(
    LinAlg::EigenVectorXd const &weights,
    LinAlg::EigenVectorXd const &previous_weights) noexcept {
  if (orders_eager) {
    // get the deviation from the previous weights
    LinAlg::EigenVectorXd deviation = weights - previous_weights;
    // scale weights by the nlv
    // deviation *= m_tracer.m_nlv;
    // fetch the current market prices
    auto close_prices = m_exchange.getSlice(close_index, 0);
    // scale the deviation by the market prices to get the units
    // deviation = deviation.cwiseQuotient(close_prices);
    // populate order struct where deviation is greater than 0
    Int64 current_time = m_exchange.getCurrentTimestamp();
    size_t exchange_offset = m_exchange.getExchangeOffset();
    size_t strategy_id = m_tracer.m_allocator->getId();
    for (size_t i = 0; i < static_cast<size_t>(deviation.size()); i++) {
      if (abs(deviation(i)) < ORDER_EPSILON) {
        continue;
      }
      auto order = Order(i + exchange_offset, strategy_id, current_time,
                         deviation(i), close_prices(i));
      m_orders.push_back(std::move(order));
    }
  }
}

//============================================================================
void StructTracer::reset() noexcept {
  m_orders.clear();
  m_trades.clear();
}

} // namespace Atlas