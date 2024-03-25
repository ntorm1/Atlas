#include "exchange/Exchange.hpp"
#include "ast/AssetNode.hpp"
#include "ast/ObserverNode.hpp"

namespace Atlas {

namespace AST {

//============================================================================
AssetObserverNode::AssetObserverNode(Option<String> name,
                                     SharedPtr<StrategyBufferOpNode> parent,
                                     AssetObserverType observer_type,
                                     size_t window) noexcept
    : StrategyBufferOpNode(NodeType::ASSET_OBSERVER, parent->getExchange(),
                           parent.get()),
      m_parent(parent), m_window(window), m_warmup(window), m_id(name),
      m_observer_warmup(window), m_observer_type(observer_type) {
  m_buffer_matrix.resize(m_exchange.getAssetCount(), window);
  m_buffer_matrix.setZero();
  m_signal.resize(m_exchange.getAssetCount());
  m_signal.setZero();
  m_signal_copy.resize(m_exchange.getAssetCount());
  m_signal_copy.setConstant(-std::numeric_limits<double>::max());
}

//============================================================================
AssetObserverNode::~AssetObserverNode() noexcept {}

//============================================================================
void AssetObserverNode::setObserverBuffer(double c) noexcept {
  m_buffer_matrix.setConstant(c);
}

//============================================================================
void AssetObserverNode::resetBase() noexcept {
  m_buffer_matrix.setZero();
  m_buffer_idx = 0;
  reset();
}

//============================================================================
void AssetObserverNode::cacheBase() noexcept {
  if (m_exchange.currentIdx() < m_observer_warmup) {
    return;
  }
  auto buffer_ref = buffer();
  m_parent->evaluate(buffer_ref);

  cacheObserver();

  if (hasCache() && m_exchange.currentIdx() >= (m_window - 1))
    cacheColumn() = m_signal;

  m_buffer_idx = (m_buffer_idx + 1) % m_window;

  m_signal_copy = m_signal;

  if (m_exchange.currentIdx() >= (m_window - 1))
    onOutOfRange(m_buffer_matrix.col(m_buffer_idx));
}

//============================================================================
bool AssetObserverNode::isSame(
    StrategyBufferOpNode const *other) const noexcept {
  if (other->getType() != NodeType::ASSET_OBSERVER)
		return false;
  auto ptr = static_cast<AssetObserverNode const *>(other);
  if (ptr->m_observer_type != m_observer_type)
		return false;
  if (ptr->window() != window())
    return false;
  return sameParents(other->getParents());
}

//============================================================================
LinAlg::EigenRef<LinAlg::EigenVectorXd> AssetObserverNode::buffer() noexcept {
  size_t col = 0;
  assert(m_buffer_idx < static_cast<size_t>(m_buffer_matrix.cols()));
  return m_buffer_matrix.col(m_buffer_idx);
}

//============================================================================
void AssetObserverNode::evaluate(
    LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept {
  assert(target.size() == m_signal_copy.size());
  target = m_signal_copy;
}

} // namespace AST

} // namespace Atlas