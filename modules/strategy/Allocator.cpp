module;
#include <Eigen/Dense>
#include <cassert>
module AtlasAllocatorModule;

import TracerModule;
import ExchangeModule;

namespace Atlas {

//============================================================================
struct AllocatorImpl {
  Option<SharedPtr<Allocator>> parent;

  AllocatorImpl(Option<SharedPtr<Allocator>> p) noexcept : parent(p) {}
};

//============================================================================
Allocator::~Allocator() noexcept {}

//============================================================================
Allocator::Allocator(String name, Exchange &exchange,
                     Option<SharedPtr<Allocator>> parent, double cash_weight) noexcept
    : m_name(name), m_exchange(exchange) {
  if (parent) {
    double parent_cash = parent.value()->getTracer().getInitialCash();
    cash_weight = parent_cash * cash_weight;
  }
  m_tracer = std::make_shared<Tracer>(this, m_exchange, cash_weight);
  m_exchange.registerAllocator(this);
  m_target_weights_buffer.resize(m_exchange.getAssetCount());
  m_target_weights_buffer.setZero();
  m_impl = std::make_unique<AllocatorImpl>(parent);
}

//============================================================================
Tracer const &Allocator::getTracer() const noexcept { return *m_tracer; }

//============================================================================
double Allocator::getNLV() const noexcept { return m_tracer->getNLV(); }

//============================================================================
Exchange const &Allocator::getExchange() const noexcept { return m_exchange; }

//============================================================================
double Allocator::getAllocation(size_t asset_index) const noexcept {
  assert(asset_index < static_cast<size_t>(m_target_weights_buffer.rows()));
  return m_target_weights_buffer[asset_index];
}

//============================================================================
void Allocator::resetBase() noexcept {
  m_step_call = false;
  m_target_weights_buffer.setZero();
  m_tracer->reset();
  reset();
}

//============================================================================
Eigen::VectorXd const &Allocator::getHistory(TracerType t) const noexcept {
  return m_tracer->getHistory(t);
}

//============================================================================
Eigen::MatrixXd const &Allocator::getWeightHistory() const noexcept {
  return m_tracer->m_weight_history;
}

} // namespace Atlas