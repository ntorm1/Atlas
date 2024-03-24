module;
#define NOMINMAX
#include "AtlasMacros.hpp"
#include <Eigen/Dense>
module StrategyModule;

import ExchangeModule;
import StrategyNodeModule;
import CommissionsModule;
import OptimizeNodeModule;
import MetaStrategyModule;
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
void Strategy::load() {
  SharedPtr<AST::StrategyNode> ast;
  ast = loadAST();
  m_impl = std::make_unique<StrategyImpl>(std::move(ast));
  m_impl->m_ast->setTracer(m_tracer);
}

//============================================================================
Strategy::~Strategy() noexcept {}

//============================================================================
const Eigen::Ref<const Eigen::VectorXd>
Strategy::getAllocationBuffer() const noexcept {
  auto parent = getParent();
  assert(parent);
  auto meta_strategy = static_cast<MetaStrategy *>(parent.value().get());
  return meta_strategy->getAllocationBuffer(this);
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
SharedPtr<CommisionManager> Strategy::initCommissionManager() noexcept {
  m_impl->m_commision_manager = CommissionManagerFactory::create(*this);
  m_impl->m_ast->setCommissionManager(m_impl->m_commision_manager.value());
  return m_impl->m_commision_manager.value();
}

//============================================================================
void Strategy::step(
    Eigen::Ref<Eigen::VectorXd> target_weights_buffer) noexcept {
  // evaluate the strategy with the current market prices and weights
  evaluate(target_weights_buffer);

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
  if (!m_impl->m_ast->evaluate(target_weights_buffer)) {
    // if no action was taken, propogate asset returns to adjust weights
    lateRebalance(target_weights_buffer);
  }
  assert(!target_weights_buffer.array().isNaN().any());
  m_step_call = false;

  if (m_impl->m_grid)
    (*m_impl->m_grid)->evaluateGrid();
  validate(target_weights_buffer);
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
void Strategy::enableCopyWeightsBuffer() noexcept {
  if (!m_impl->m_ast) {
    return;
  }
  m_impl->m_ast->enableCopyWeightsBuffer();
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

} // namespace Atlas