module;
#include "AtlasMacros.hpp"
#include <Eigen/Dense>
module MetaStrategyModule;

namespace Atlas {

//============================================================================
struct MetaStrategyImpl {
  size_t warmup = 0;
  Vector<SharedPtr<Allocator>> child_strategies;
  HashMap<String, size_t> strategy_map;
  LinAlg::EigenMatrixXd weights;
  LinAlg::EigenVectorXd meta_weights;
};

//============================================================================
size_t MetaStrategy::getWarmup() const noexcept { return m_impl->warmup; }

//============================================================================
MetaStrategy::MetaStrategy(String name, SharedPtr<Exchange> exchange,
                           Option<SharedPtr<Allocator>> parent,
                           double cash) noexcept
    : Allocator(name, *exchange, parent, cash) {
  m_impl = std::make_unique<MetaStrategyImpl>();
  if (parent) {
    m_impl->warmup = parent.value()->getWarmup();
    m_impl->weights.resize(getAssetCount(), 1);
    m_impl->weights.setZero();
    m_impl->meta_weights.resize(getAssetCount());
    m_impl->meta_weights.setZero();
  }
  setIsMeta(true);
}

//============================================================================
const Eigen::Ref<const Eigen::VectorXd>
MetaStrategy::getAllocationBuffer() const noexcept {
  return m_impl->meta_weights;
}

//============================================================================
const Eigen::Ref<const Eigen::VectorXd>
MetaStrategy::getAllocationBuffer(Allocator const*strategy) const noexcept {
  auto it = m_impl->strategy_map.find(strategy->getName());
  assert(it != m_impl->strategy_map.end());
  auto idx = it->second;
  return m_impl->weights.col(static_cast<int>(idx));
}

//============================================================================
MetaStrategy::~MetaStrategy() noexcept {}

//============================================================================
SharedPtr<Allocator> MetaStrategy::pyAddStrategy(SharedPtr<Allocator> allocator,
                                                 bool replace_if_exists) {
  if (m_impl->strategy_map.contains(allocator->getName())) {
    if (!replace_if_exists) {
      throw std::runtime_error("Allocator already exists");
    }
    // find the Allocator and replace it in the vector
    auto idx = m_impl->strategy_map[allocator->getName()];
    allocator->load();
    m_impl->child_strategies[idx] = std::move(allocator);
    return m_impl->child_strategies[idx];
  }
  allocator->setID(m_impl->child_strategies.size());
  allocator->load();
  m_impl->strategy_map[allocator->getName()] = m_impl->child_strategies.size();
  m_impl->child_strategies.push_back(std::move(allocator));

  m_impl->weights.conservativeResize(getAssetCount(),
																		 m_impl->child_strategies.size());
  m_impl->weights.setZero();
  return m_impl->child_strategies.back();
}

//============================================================================
void MetaStrategy::step() noexcept {
  if (!m_step_call) {
    return;
  }
  for (size_t i = 0; i < m_impl->child_strategies.size(); i++) {
    auto target_weights = m_impl->weights.col(static_cast<int>(i));
    auto &strategy = m_impl->child_strategies[i];
    strategy->step(target_weights);
    if (strategy->getIsMeta()) {
      auto meta_strategy = static_cast<MetaStrategy *>(strategy.get());
      meta_strategy->step();
    }
  }
  step(m_impl->meta_weights);
}

//============================================================================
void MetaStrategy::step(
    LinAlg::EigenRef<LinAlg::EigenVectorXd> target_weights_buffer) noexcept {}

//============================================================================
void MetaStrategy::reset() noexcept {
  m_impl->weights.setZero();
  m_impl->meta_weights.setZero();
}

} // namespace Atlas