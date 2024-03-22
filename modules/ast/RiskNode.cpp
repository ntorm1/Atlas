module;
#include <Eigen/Dense>
#ifdef _DEBUG
#include <iostream>
#endif _DEBUG
module RiskNodeModule;

import HelperNodesModule;
import ExchangeModule;
import AllocationNodeModule;

namespace Atlas {

namespace AST {

//============================================================================
CovarianceNodeBase::CovarianceNodeBase(Exchange &exchange,
                                       SharedPtr<TriggerNode> trigger,
                                       size_t lookback_window) noexcept
    : StatementNode(NodeType::COVARIANCE), m_exchange(exchange),
      m_trigger(trigger), m_lookback_window(lookback_window) {
  // find the index location of the first non-zero value
  Eigen::VectorXi const &mask = m_trigger->getMask();
  size_t idx = 0;
  for (size_t i = 0; i < static_cast<size_t>(mask.size()); ++i) {
    if (mask(i) != 0) {
      idx = i;
      break;
    }
  }
  // set the warmup equal to the max of the lookback window and the index
  m_warmup = std::max(m_lookback_window, idx);

  size_t row_count = exchange.getAssetCount();
  m_covariance.resize(row_count, row_count);
  m_covariance.setZero();
}

//============================================================================
CovarianceNode::CovarianceNode(Exchange &exchange,
                               SharedPtr<TriggerNode> trigger,
                               size_t lookback_window) noexcept
    : CovarianceNodeBase(exchange, trigger, lookback_window) {}

//============================================================================
CovarianceNode::~CovarianceNode() noexcept {}

//============================================================================
void CovarianceNode::evaluateChild() noexcept {
  size_t start_idx = (m_exchange.currentIdx() - m_lookback_window) + 1;
  auto const &returns_block =
      m_exchange.getMarketReturnsBlock(start_idx, m_exchange.currentIdx());
  Eigen::MatrixXd returns_block_transpose = returns_block.transpose();
  m_centered_returns = returns_block_transpose.rowwise() -
                       returns_block_transpose.colwise().mean();
  m_covariance = (m_centered_returns.adjoint() * m_centered_returns) /
                 double(returns_block_transpose.rows() - 1);
}

//============================================================================
IncrementalCovarianceNode::IncrementalCovarianceNode(
    Exchange &exchange, SharedPtr<TriggerNode> trigger,
    size_t lookback_window) noexcept
    : CovarianceNodeBase(exchange, trigger, lookback_window) {
  size_t row_count = exchange.getAssetCount();
  m_sum.resize(row_count, row_count);
  m_sum.setZero();
  m_sum_product.resize(row_count, row_count);
  m_sum_product.setZero();
  m_product_buffer.resize(row_count, row_count);
  m_product_buffer.setZero();
  enableIncremental();
}

//============================================================================
IncrementalCovarianceNode::~IncrementalCovarianceNode() noexcept {}

//============================================================================
void IncrementalCovarianceNode::evaluateChild() noexcept {
  assert(false);
  /*
  auto returns = m_exchange.getMarketReturns().transpose();
  m_sum.rowwise() += returns;

  m_product_buffer = returns.replicate(returns.cols(),1);
  m_product_buffer = m_product_buffer.transpose().lazyProduct(m_product_buffer);
  m_sum_product += m_product_buffer;

  if (m_exchange.currentIdx() >= m_lookback_window)
  {
          int row_offset = -1 * static_cast<int>(m_lookback_window);
          returns = m_exchange.getMarketReturns(row_offset).transpose();
          m_sum.rowwise() -= returns;
          m_product_buffer = returns.replicate(returns.cols(), 1);
          m_product_buffer =
  m_product_buffer.transpose().lazyProduct(m_product_buffer); m_sum_product -=
  m_product_buffer;
  }
  m_covariance = (m_sum_product - (m_sum * m_sum.transpose()) /
  double(m_lookback_window)) / double(m_lookback_window - 1);
  */
}

//============================================================================
void CovarianceNode::resetChild() noexcept { m_centered_returns.setZero(); }

//============================================================================
void IncrementalCovarianceNode::resetChild() noexcept {
  m_counter = 0;
  m_sum.setZero();
  m_sum_product.setZero();
  m_product_buffer.setZero();
}

//============================================================================
void CovarianceNodeBase::reset() noexcept {
  m_covariance.setZero();
  m_cached = false;
  resetChild();
}

//============================================================================
void CovarianceNodeBase::evaluate() noexcept {
  if (!m_trigger->evaluate()) {
    return;
  }
  if (m_exchange.currentIdx() < m_lookback_window) {
    if (!m_incremental) {
      return;
    } else {
      evaluateChild();
    }
    return;
  }
  evaluateChild();
  m_cached = true;
}

//============================================================================
AllocationWeightNode::~AllocationWeightNode() noexcept {}

//============================================================================
AllocationWeightNode::AllocationWeightNode(
    SharedPtr<CovarianceNodeBase> covariance,
    Option<double> vol_target) noexcept
    : StrategyBufferOpNode(NodeType::ALLOC_WEIGHT, covariance->getExchange(),
                           std::nullopt),
      m_covariance(covariance), m_vol_target(vol_target) {}

//============================================================================
void AllocationWeightNode::targetVol(
    LinAlg::EigenRef<LinAlg::EigenVectorXd> target) const noexcept {
  auto const &covariance = m_covariance->getCovariance();
  assert(covariance.rows() == covariance.cols());
  assert(m_vol_target);
  auto v = (target.transpose() * covariance * target);
  double annualized_vol = std::sqrt(v(0)) * std::sqrt(252);
  double vol_scale = m_vol_target.value() / annualized_vol;
  target *= vol_scale;
}

//============================================================================
InvVolWeight::~InvVolWeight() noexcept {}

//============================================================================
InvVolWeight::InvVolWeight(SharedPtr<CovarianceNodeBase> covariance,
                           Option<double> vol_target) noexcept
    : AllocationWeightNode(std::move(covariance), vol_target) {}

//============================================================================
void InvVolWeight::evaluate(
    LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept {
  // prevent division by zero
  assert(target.array().abs().sum() > 1e-10);

  auto const &covariance = m_covariance->getCovariance();

  assert(covariance.rows() == covariance.cols());
  auto vol = covariance.diagonal().array().sqrt();
  assert(!vol.array().isNaN().any());

  target.array() /= vol.array();
  double abs_weight_sum = target.array().abs().sum();
  assert(abs_weight_sum > 1e-10);
  target = (target.array() / abs_weight_sum).matrix();
  if (m_vol_target) {
    targetVol(target);
  }
  assert(!target.array().isNaN().any());
}

//============================================================================
static LinAlg::EigenVectorXd
risk_parity_weights_ccd_spinu(LinAlg::EigenVectorXd const &weights,
                              LinAlg::EigenMatrixXd const &cov,
                              const double tol, const int max_iter) {
  assert(weights.size() == cov.rows());
  double aux, x_diff, xk_sum;
  auto n = weights.size();
  LinAlg::EigenVectorXd xk = LinAlg::EigenVectorXd::Constant(n, 1);
  xk = std::sqrt(1.0 / cov.sum()) * xk;
  LinAlg::EigenVectorXd x_star(n), Sigma_xk(n), rc(n);
  Sigma_xk = cov * xk;
  for (auto k = 0; k < max_iter; ++k) {
    for (auto i = 0; i < n; ++i) {
      // compute update for the portfolio weights x
      aux = xk(i) * cov(i, i) - Sigma_xk(i);
      x_star(i) = (.5 / cov(i, i)) *
                  (aux + std::sqrt(aux * aux + 4 * cov(i, i) * weights(i)));
      // update auxiliary terms
      x_diff = x_star(i) - xk(i);
      Sigma_xk += (cov.col(i).array() * x_diff).matrix();
      xk(i) = x_star(i);
    }
    xk_sum = xk.sum();
    rc = (xk.array() * (Sigma_xk).array() / (xk_sum * xk_sum)).matrix();
    if ((rc.array() / rc.sum() - weights.array()).abs().maxCoeff() < tol)
      break;
  }
  return x_star / xk_sum;
}

//============================================================================
static LinAlg::EigenVectorXd
risk_parity_portfolio_ccd_choi(LinAlg::EigenVectorXd const &weights,
                               LinAlg::EigenMatrixXd const &cov,
                               const double tol, const int max_iter) {
  assert(weights.size() == cov.rows());
  double ai;
  auto n = weights.size();
  auto vol = cov.diagonal().array().sqrt();
  auto invvol = (1 / vol.array()).matrix();
  LinAlg::EigenMatrixXd corr = cov.array().colwise() * invvol.array();
  corr = corr.array().rowwise() * invvol.transpose().array();
  LinAlg::EigenMatrixXd adj = corr;
  adj.diagonal().array() = 0;
  LinAlg::EigenVectorXd wk = LinAlg::EigenVectorXd::Ones(n);
  wk = (wk.array() / std::sqrt(corr.sum())).matrix();
  for (auto k = 0; k < max_iter; ++k) {
    for (auto i = 0; i < n; ++i) {
      // compute portfolio weights
      ai = 0.5 * ((adj.col(i).array() * wk.array()).sum());
      wk(i) = std::sqrt(ai * ai + weights(i)) - ai;
    }
    wk = wk.array() / std::sqrt(wk.transpose() * corr * wk);
    if ((wk.array() * (corr * wk).array() - weights.array()).abs().maxCoeff() <
        tol)
      break;
  }
  LinAlg::EigenVectorXd w = wk.array() / vol.array();
  return (w / w.sum()).matrix();
}

// ===========================================================================
RiskParityWeight::~RiskParityWeight() noexcept {}

//============================================================================
RiskParityWeight::RiskParityWeight(SharedPtr<CovarianceNodeBase> covariance,
                                   Option<double> vol_target, double tol,
                                   int max_iter, bool chinu) noexcept
    : AllocationWeightNode(std::move(covariance), vol_target), m_chinu(chinu),
      m_tol(tol), m_max_iter(max_iter) {}

//============================================================================
void RiskParityWeight::evaluate(
    LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept {
  // scale target to 1 for stability
  target /= target.sum();
  if (m_chinu) {
    target = risk_parity_portfolio_ccd_choi(
        target, m_covariance->getCovariance(), m_tol, m_max_iter);
  } else {
    target = risk_parity_weights_ccd_spinu(
        target, m_covariance->getCovariance(), m_tol, m_max_iter);
  }
  if (m_vol_target) {
    targetVol(target);
  }
}

//============================================================================
bool RiskParityWeight::isSame(
    StrategyBufferOpNode const* other) const noexcept {
  return false; //TODO: implement
}

} // namespace AST

} // namespace Atlas