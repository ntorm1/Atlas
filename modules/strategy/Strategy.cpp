module;
#define NOMINMAX
#include "AtlasMacros.hpp"
#include <Eigen/Dense>
module StrategyModule;

import ExchangeModule;
import StrategyNodeModule;
import CommissionsModule;
import OptimizeNodeModule;
import TracerModule;

import AtlasLinAlg;

namespace Atlas {

//============================================================================
class StrategyImpl {
public:
  SharedPtr<AST::StrategyNode> m_ast;
  Option<SharedPtr<CommisionManager>> m_commision_manager;
  Option<SharedPtr<AST::StrategyGrid>> m_grid;

  StrategyImpl(SharedPtr<AST::StrategyNode> ast) noexcept
      : m_ast(std::move(ast)) {}
};

//============================================================================
Strategy::Strategy(String name, SharedPtr<Exchange> exchange,
                   SharedPtr<Allocator> parent,
                   double portfolio_weight) noexcept
    : Allocator(name, *exchange, parent, portfolio_weight) {
  m_impl = nullptr;
  m_portfolio_weight = portfolio_weight;
}

//============================================================================
void Strategy::load() noexcept {
  auto ast = loadAST();
  m_impl = std::make_unique<StrategyImpl>(std::move(ast));
  m_impl->m_ast->setTracer(m_tracer);
}

//============================================================================
Strategy::~Strategy() noexcept {}

//============================================================================
[[nodiscard]] Result<bool, AtlasException>
Strategy::enableTracerHistory(TracerType t) noexcept {
  if (!m_impl->m_ast) {
    return Err("ast not build yet");
  }
  switch (t) {
  case TracerType::ORDERS_EAGER:
    m_impl->m_ast->enableCopyWeightsBuffer();
  default:
    break;
  }

  return m_tracer->enableTracerHistory(t);
}

//============================================================================
Option<SharedPtr<AST::StrategyGrid>> Strategy::getGrid() const noexcept {
  return m_impl->m_grid;
}

//============================================================================
Result<SharedPtr<AST::StrategyGrid const>, AtlasException>
Strategy::setGridDimmensions(
    std::pair<SharedPtr<AST::GridDimension>, SharedPtr<AST::GridDimension>>
        dimensions,
    Option<GridType> grid_type) noexcept {
  if (m_impl->m_grid) {
    return Err("Grid already set");
  }
  m_impl->m_grid = std::make_shared<AST::StrategyGrid>(
      this, m_exchange, std::move(dimensions), grid_type);
  return m_impl->m_grid.value();
}

//============================================================================
SharedPtr<AST::StrategyGrid const> Strategy::pySetGridDimmensions(
    std::pair<SharedPtr<AST::GridDimension>, SharedPtr<AST::GridDimension>>
        dimensions,
    Option<GridType> grid_type) {
  auto res = setGridDimmensions(std::move(dimensions), grid_type);
  if (!res) {
    throw std::runtime_error(res.error().what());
  }
  return res.value();
}

//============================================================================
void Strategy::pyEnableTracerHistory(TracerType t) {
  auto res = enableTracerHistory(t);
  if (!res) {
    throw std::runtime_error(res.error().what());
  }
}

//============================================================================
void Strategy::setVolTracer(SharedPtr<AST::CovarianceNodeBase> node) noexcept {
  assert(node);
  m_tracer->setCovarianceNode(node);
  auto res = m_tracer->enableTracerHistory(TracerType::VOLATILITY);
  assert(res);
}

//============================================================================
SharedPtr<CommisionManager> Strategy::initCommissionManager() noexcept {
  m_impl->m_commision_manager = CommissionManagerFactory::create(*this);
  m_impl->m_ast->setCommissionManager(m_impl->m_commision_manager.value());
  return m_impl->m_commision_manager.value();
}

//============================================================================
void Strategy::evaluate(
    Eigen::Ref<Eigen::VectorXd> const &target_weights_buffer) noexcept {
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
void Strategy::lateRebalance(
    Eigen::Ref<Eigen::VectorXd> target_weights_buffer) noexcept {
  // if the strategy does not override the target weights buffer at the end of a
  // time step, then we need to rebalance the portfolio to the target weights
  // buffer according to the market returns update the target weights buffer
  // according to the indivual asset returns
  target_weights_buffer =
      m_exchange.getReturnsScalar().cwiseProduct(target_weights_buffer);
  assert(!target_weights_buffer.array().isNaN().any());
}

//============================================================================
void Strategy::step(
    Eigen::Ref<Eigen::VectorXd> target_weights_buffer) noexcept {

  // execute the strategy AST node. Update rebalance call if AST
  // did not update the target weights buffer
  if (!m_impl->m_ast->evaluate(target_weights_buffer)) {
    // if no action was taken, propogate asset returns to adjust weights
    lateRebalance(target_weights_buffer);
  }
}

//============================================================================
void Strategy::step() noexcept {
  // evaluate the strategy with the current market prices and weights
  evaluate(m_target_weights_buffer);

  // check if exchange took step or if the warmup is not over. In which case,
  // we do not need to execute the strategy AST node but we do check to see
  // if the grid needs to be evaluated so that the grid history lines up
  // with the base strategy
  if (!m_step_call) {
    if (m_impl->m_grid)
      (*m_impl->m_grid)->evaluate();
    return;
  }

  // check if warmup over
  if (m_exchange.currentIdx() < m_impl->m_ast->getWarmup()) {
    if (m_impl->m_grid)
      (*m_impl->m_grid)->evaluateGrid();
    return;
  }
  // execute the strategy AST node. Update rebalance call if AST
  // did not update the target weights buffer
  if (!m_impl->m_ast->evaluate(m_target_weights_buffer)) {
    // if no action was taken, propogate asset returns to adjust weights
    lateRebalance(m_target_weights_buffer);
  }
  assert(!m_target_weights_buffer.array().isNaN().any());
  m_step_call = false;

  if (m_impl->m_grid)
    (*m_impl->m_grid)->evaluateGrid();
}

//============================================================================
void Strategy::setNlv(double nlv_new) noexcept { m_tracer->setNLV(nlv_new); }

//============================================================================
SharedPtr<Tracer> Strategy::getTracerPtr() const noexcept { return m_tracer; }

//============================================================================
Option<SharedPtr<AST::TradeLimitNode>>
Strategy::getTradeLimitNode() const noexcept {
  return m_impl->m_ast->getTradeLimitNode();
}

//============================================================================
LinAlg::EigenRef<LinAlg::EigenVectorXd> Strategy::getPnL() noexcept {
  return m_impl->m_ast->getPnL();
}

//============================================================================
void Strategy::setTracer(SharedPtr<Tracer> tracer) noexcept {
  m_impl->m_ast->setTracer(tracer);
  std::swap(m_tracer, tracer);
}

//============================================================================
size_t Strategy::refreshWarmup() noexcept {
  return m_impl->m_ast->refreshWarmup();
}

//============================================================================
size_t Strategy::getWarmup() const noexcept {
  return m_impl->m_ast->getWarmup();
}

//============================================================================
void Strategy::reset() noexcept {
  m_impl->m_ast->reset();
  if (m_impl->m_grid)
    (*m_impl->m_grid)->reset();
}

//============================================================================
void Strategy::realize() noexcept { m_tracer->realize(); }

} // namespace Atlas