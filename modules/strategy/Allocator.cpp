module;
#include <Eigen/Dense>
#include <cassert>
module AtlasAllocatorModule;

import TracerModule;
import ExchangeModule;
import MetaStrategyModule;

namespace Atlas {

//============================================================================
struct AllocatorImpl {
  Option<SharedPtr<Allocator>> parent;
  Option<AtlasException> exception = std::nullopt;
  Option<AllocatorConfig> config = std::nullopt;
  bool is_disabled = false;

  AllocatorImpl(Option<SharedPtr<Allocator>> p) noexcept : parent(p) {}
};

//============================================================================
void Allocator::takeException(Vector<AtlasException> &exceptions) noexcept {
  auto res = m_impl->exception;
  if (res) {
    m_impl->exception = std::nullopt;
    exceptions.push_back(std::move(res.value()));
  }
  if (m_is_meta) {
    auto p = static_cast<MetaStrategy *>(this);
    for (auto &a : p->getStrategies()) {
      a->takeException(exceptions);
    }
  }
}

//============================================================================
void Allocator::setException(AtlasException const &exception) noexcept {
	m_impl->exception = exception;
}

//============================================================================
Option<AtlasException> Allocator::getException() noexcept {
	return m_impl->exception;
}

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
    m_portfolio_weight = cash_weight;
    cash_weight = parent_cash * cash_weight;
  } else {
    m_portfolio_weight = 1;
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
void Allocator::evaluate(
    Eigen::Ref<Eigen::VectorXd> const &target_weights_buffer) noexcept {
  if (m_impl->is_disabled) {
    return;
  }
  // get the current market returns
  LinAlg::EigenConstColView market_returns = m_exchange.getMarketReturns();

  // get the portfolio return by calculating the sum product of the market
  // returns and the portfolio weights
  assert(market_returns.rows() == target_weights_buffer.rows());
  assert(!market_returns.array().isNaN().any());

  // print out target weights buffer and market returns
  double portfolio_return = market_returns.dot(target_weights_buffer);

  // update the tracer nlv
  double nlv = m_tracer->getNLV();
  m_tracer->setNLV(nlv * (1.0 + portfolio_return));
  m_tracer->evaluate();
}

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
void Allocator::validate(
    LinAlg::EigenRef<LinAlg::EigenVectorXd> target_weights_buffer) noexcept {
  if (!m_impl->config) {
    return;
  }
  auto const &config = m_impl->config.value();
  if (!config.can_short && target_weights_buffer.minCoeff() < 0) {
    return disable("Shorting is not allowed");
  }
  if (config.weight_clip) {
    if (config.disable_on_breach &&
        target_weights_buffer.maxCoeff() > *config.weight_clip) {
      return disable("Weight clip breach");
    }
    target_weights_buffer = target_weights_buffer.cwiseMax(*config.weight_clip);
  }
}

//============================================================================
void Allocator::realize(Vector<AtlasException> &exceptions) noexcept {
  takeException(exceptions);
  m_tracer->realize();
}

//============================================================================
void Allocator::disable(String const &exception) noexcept {
  m_impl->exception = AtlasException(exception);
  m_impl->is_disabled = true;
}

//============================================================================
void Allocator::stepBase(
    LinAlg::EigenRef<LinAlg::EigenVectorXd> target_weights_buffer) noexcept {
  if (m_impl->is_disabled) {
    return;
  }
  if (!m_step_call) {
    validate(target_weights_buffer);
    return;
  }
  step(target_weights_buffer);
  validate(target_weights_buffer);
}

//============================================================================
Option<SharedPtr<Allocator>> Allocator::getParent() const noexcept {
  return m_impl->parent;
}

//============================================================================
Option<SharedPtr<Measure>> Allocator::getMeasure(TracerType t) const noexcept {
  return m_tracer->getMeasure(t);
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

//============================================================================
void Allocator::pyEnableMeasure(TracerType t) {
  auto res = enableTracerHistory(t);
  if (!res) {
    throw std::runtime_error(res.error().what());
  }
}

//============================================================================
void Allocator::setVolMeasure(
    SharedPtr<AST::CovarianceNodeBase> node) noexcept {
  assert(node);
  m_tracer->setCovarianceNode(node);
  auto res = m_tracer->enableTracerHistory(TracerType::VOLATILITY);
  assert(res);
}

//============================================================================
Result<bool, AtlasException>
Allocator::enableTracerHistory(TracerType t) noexcept {
  switch (t) {
  case TracerType::ORDERS_EAGER:
    enableCopyWeightsBuffer();
  default:
    break;
  }
  return m_tracer->enableTracerHistory(t);
}

} // namespace Atlas