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
size_t Allocator::getAssetCount() const noexcept {
  return m_exchange.getAssetCount();
}

//============================================================================
Allocator::~Allocator() noexcept {}

//============================================================================
Allocator::Allocator(String name, Exchange &exchange,
                     Option<SharedPtr<Allocator>> parent,
                     double cash_weight) noexcept
    : m_name(name), m_exchange(exchange) {
  if (parent) {
    double parent_cash = parent.value()->getTracer().getInitialCash();
    cash_weight = parent_cash * cash_weight;
  }
  m_tracer = std::make_shared<Tracer>(this, m_exchange, cash_weight);
  m_exchange.registerAllocator(this);
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
  auto buffer = getAllocationBuffer();
  assert(asset_index < static_cast<size_t>(buffer.cols()));
  return buffer[asset_index];
}

//============================================================================
void Allocator::resetBase() noexcept {
  m_step_call = false;
  m_tracer->reset();
  reset();
}

//============================================================================
void Allocator::realize() noexcept { m_tracer->realize(); }

//============================================================================
Option<SharedPtr<Allocator>> Allocator::getParent() const noexcept {
  return m_impl->parent;
}

//============================================================================
Eigen::VectorXd const &Allocator::getHistory(TracerType t) const noexcept {
  return m_tracer->getHistory(t);
}

//============================================================================
Eigen::MatrixXd const &Allocator::getWeightHistory() const noexcept {
  return m_tracer->m_weight_history;
}

//============================================================================
void Allocator::lateRebalance(
    Eigen::Ref<Eigen::VectorXd> target_weights_buffer) noexcept {
  // if the strategy does not override the target weights buffer at the end of
  // a time step, then we need to rebalance the portfolio to the target
  // weights buffer according to the market returns update the target weights
  // buffer according to the indivual asset returns
  target_weights_buffer =
      m_exchange.getReturnsScalar().cwiseProduct(target_weights_buffer);
  assert(!target_weights_buffer.array().isNaN().any());
}

} // namespace Atlas