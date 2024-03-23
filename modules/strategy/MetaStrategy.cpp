module;
#include "AtlasMacros.hpp"
#pragma warning(push, 0)
#include <Eigen/Dense>
#pragma warning(pop)
module MetaStrategyModule;

namespace Atlas {

//============================================================================
struct MetaStrategyImpl {
  Vector<SharedPtr<Allocator>> child_strategies;
  HashMap<String, size_t> strategy_map;
  LinAlg::EigenMatrixXd weights;
  LinAlg::EigenVectorXd child_strategy_weights;
  LinAlg::EigenVectorXd meta_weights;
};

//============================================================================
void MetaStrategy::enableCopyWeightsBuffer() noexcept {}

//============================================================================
size_t MetaStrategy::getWarmup() const noexcept { return 0; }

//============================================================================
MetaStrategy::MetaStrategy(String name, SharedPtr<Exchange> exchange,
                           Option<SharedPtr<Allocator>> parent,
                           double cash) noexcept
    : Allocator(name, *exchange, parent, cash) {
  m_impl = std::make_unique<MetaStrategyImpl>();
  m_impl->meta_weights.resize(getAssetCount());
  m_impl->meta_weights.setZero();
  setIsMeta(true);
}

//============================================================================
Vector<SharedPtr<Allocator>> MetaStrategy::getStrategies() const noexcept {
  return m_impl->child_strategies;
}

//============================================================================
const Eigen::Ref<const Eigen::VectorXd>
MetaStrategy::getAllocationBuffer() const noexcept {
  return m_impl->meta_weights;
}

//============================================================================
const Eigen::Ref<const Eigen::VectorXd>
MetaStrategy::getAllocationBuffer(Allocator const *strategy) const noexcept {
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

  // reshape matrix/vector containers holding sub strategy weightings
  m_impl->weights.resize(getAssetCount(), m_impl->child_strategies.size());
  m_impl->weights.setZero();
  // reshape vector holding child strategy allocations
  m_impl->child_strategy_weights.resize(m_impl->child_strategies.size());
  int i = 0;
  for (auto const &strategy : m_impl->child_strategies) {
    m_impl->child_strategy_weights[i] = strategy->getAllocation();
    i++;
  }
  return m_impl->child_strategies.back();
}

//============================================================================
void MetaStrategy::step() noexcept {
  if (!m_impl->child_strategies.size()) {
    return;
  }
  evaluate(m_impl->meta_weights);

#pragma omp parallel for
  for (int i = 0; i < m_impl->child_strategies.size(); i++) {
    auto target_weights = m_impl->weights.col(static_cast<int>(i));
    auto &strategy = m_impl->child_strategies[i];
    strategy->stepBase(target_weights);
    if (strategy->getIsMeta()) {
      auto meta_strategy = static_cast<MetaStrategy *>(strategy.get());
      meta_strategy->step();
    }
  }
  stepBase(m_impl->meta_weights);
}

//============================================================================
void MetaStrategy::step(
    LinAlg::EigenRef<LinAlg::EigenVectorXd> target_weights_buffer) noexcept {
  // weights holds an M by N matrix of portfolio weights for the child
  // strategies where M is the number of assets and N is the number of child
  // strategies, to get the parent allocation, get the row-wise weighted mean
  assert(m_impl->weights.cols() == m_impl->child_strategy_weights.rows());
  assert(target_weights_buffer.rows() == m_impl->weights.rows());
  target_weights_buffer = (m_impl->weights.array().rowwise() *
                           m_impl->child_strategy_weights.array().transpose())
                              .rowwise()
                              .sum();
  m_step_call = false;
}

//============================================================================
void MetaStrategy::reset() noexcept {
  m_impl->weights.setZero();
  m_impl->meta_weights.setZero();
}

} // namespace Atlas