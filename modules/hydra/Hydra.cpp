#include "AtlasMacros.hpp"
#include <cassert>
#include <stdexcept>
#include "exchange/ExchangeMap.hpp"
#include "strategy/Allocator.hpp"
#include "strategy/MetaStrategy.hpp"
#include "hydra/Hydra.hpp"

namespace Atlas {

//============================================================================
struct HydraImpl {
  ExchangeMap m_exchange_map;
  Vector<AtlasException> exceptions;
  Vector<SharedPtr<MetaStrategy>> m_strategies;
  HashMap<String, size_t> m_strategy_map;
  HashMap<String, size_t> m_portfolio_map;
};

//============================================================================
Hydra::Hydra() noexcept { m_impl = std::make_unique<HydraImpl>(); }

//============================================================================
Hydra::~Hydra() noexcept {}

//============================================================================
ExchangeMap const &Hydra::getExchangeMap() const noexcept {
  return m_impl->m_exchange_map;
}

//============================================================================
Option<String>
Hydra::getParentExchangeName(String const &asset_name) const noexcept {
  return m_impl->m_exchange_map.getParentExchangeName(asset_name);
}

//============================================================================
Result<SharedPtr<Exchange>, AtlasException>
Hydra::addExchange(String name, String source,
                   Option<String> datetime_format) noexcept {
  if (m_state != HydraState::INIT && m_state != HydraState::BUILT) {
    return Err("Hydra must be in init state to add exchange");
  }
  auto res = m_impl->m_exchange_map.addExchange(
      std::move(name), std::move(source), std::move(datetime_format));
  if (!res) {
    return res;
  }
  m_state = HydraState::INIT;
  return res;
}

//============================================================================
Result<SharedPtr<Exchange>, AtlasException>
Hydra::getExchange(String const &name) const noexcept {
  return m_impl->m_exchange_map.getExchange(name);
}

//============================================================================
Result<MetaStrategy const *, AtlasException>
Hydra::addStrategy(SharedPtr<MetaStrategy> allocator,
                   bool replace_if_exists) noexcept {
  if (m_state != HydraState::BUILT && m_state != HydraState::FINISHED) {
    return Err("Hydra must be in build or finished state to add Allocator");
  }
  if (m_impl->m_strategy_map.contains(allocator->getName())) {
    if (!replace_if_exists) {
      return Err("Allocator with name " + allocator->getName() +
                 " already exists");
    }
    // find the Allocator and replace it in the vector
    auto idx = m_impl->m_strategy_map[allocator->getName()];
    allocator->load();
    m_impl->m_strategies[idx] = std::move(allocator);
    return m_impl->m_strategies[idx].get();
  }
  allocator->setID(m_impl->m_strategies.size());
  allocator->load();
  m_impl->m_strategy_map[allocator->getName()] = m_impl->m_strategies.size();
  m_impl->m_strategies.push_back(std::move(allocator));
  return m_impl->m_strategies.back().get();
}

//============================================================================
Option<SharedPtr<Allocator>>
Hydra::getStrategy(String const &strategy_name) noexcept {
  if (!m_impl->m_strategy_map.contains(strategy_name)) {
    return std::nullopt;
  }
  return m_impl->m_strategies[m_impl->m_strategy_map[strategy_name]];
}

//============================================================================
Result<bool, AtlasException>
Hydra::removeExchange(String const &name) noexcept {
  auto exchange_opt = getExchange(name);
  if (!exchange_opt) {
    return Err("Exchange with name " + name + " does not exist");
  }
  auto &exchange = *exchange_opt;

  for (auto const &allocator : m_impl->m_strategies) {
    if (&(allocator->getExchange()) == exchange.get()) {
      return Err("Exchange " + name + " is used by Allocator " +
                 allocator->getName());
    }
  }

  if (m_state == HydraState::RUNING) {
    return Err("Hydra must be in build or finished state to remove exchange");
  }
  m_impl->m_exchange_map.removeExchange(name);
  reset();
  build();
  return true;
}

//============================================================================
Result<bool, AtlasException> Hydra::build() {
  m_impl->m_exchange_map.build();

  if (m_impl->m_exchange_map.getTimestamps().size() == 0) {
    return Err("No timestamps found");
  }

  m_state = HydraState::BUILT;
  return true;
}

//============================================================================
void Hydra::removeStrategy(String const &name) noexcept {
  if (!m_impl->m_strategy_map.contains(name)) {
    m_impl->m_exchange_map.cleanup();
    return;
  }
  auto idx = m_impl->m_strategy_map[name];
  // update all the Allocator ids greater than the one we are removing
  for (size_t i = idx + 1; idx < m_impl->m_strategies.size() - 1; ++i) {
    m_impl->m_strategies[i]->setID(i - 1);
    m_impl->m_strategy_map[m_impl->m_strategies[i]->getName()] = i - 1;
  }

  auto& allocator = m_impl->m_strategies[idx];
  // erase the Allocator from the vector
  m_impl->m_strategies.erase(m_impl->m_strategies.begin() + idx);
  // assert reference count is 1
  assert(allocator.use_count() == 1);
  // update the Allocator map
  m_impl->m_strategy_map.erase(name);
  // check for unused trigger nodes or covariance nodes.
  m_impl->m_exchange_map.cleanup();
}

//============================================================================
void Hydra::step() noexcept {
  assert(m_state == HydraState::BUILT || m_state == HydraState::RUNING);
  m_impl->m_exchange_map.step();

  for (int i = 0; i < m_impl->m_strategies.size(); i++) {
    // execute Allocator logic to populate new target weights
    auto &allocator = m_impl->m_strategies[i];
    allocator->step();
  }
  m_state = HydraState::RUNING;
}

//============================================================================
Result<bool, AtlasException> Hydra::run() noexcept {
  // validate that the hydra has been built
  if (m_state == HydraState::INIT) {
    return Err("Hydra must be in build or finished state to run");
  }
  // if called directly after a run, reset the hydra
  if (m_state == HydraState::FINISHED) {
    auto res = reset();
    assert(res);
  }
  size_t steps = m_impl->m_exchange_map.getTimestamps().size();

  // if there are no timestamps, return false
  if (steps == 0) {
    return Err("No timestamps found");
  }

  // adjust loop size if calling run from middle of simulation
  if (m_state == HydraState::RUNING) {
    steps -= m_impl->m_exchange_map.getCurrentIdx();
  }

  // enter simulation loop
  for (size_t i = 0; i < steps; ++i) {
    step();
  }

  // on finish realize Allocator valuation as needed
  for (auto &allocator : m_impl->m_strategies) {
    allocator->realize(m_impl->exceptions);
  }
  if (m_impl->exceptions.size() > 0) {
		return Err("Exceptions occurred during run, see Hydra::getExceptions()");
	}

  m_state = HydraState::FINISHED;
  return true;
}

//============================================================================
Int64 Hydra::currentGlobalTime() const noexcept {
  auto idx = m_impl->m_exchange_map.getCurrentIdx();
  if (idx == 0) {
    return 0;
  }
  auto const &timestamps = m_impl->m_exchange_map.getTimestamps();
  if (idx >= timestamps.size()) {
    return 0;
  }
  return timestamps[idx - 1];
}

//============================================================================
Int64 Hydra::nextGlobalTime() const noexcept {
  auto idx = m_impl->m_exchange_map.getCurrentIdx();
  if (idx == 0) {
    return 0;
  }
  auto const &timestamps = m_impl->m_exchange_map.getTimestamps();
  if (idx >= timestamps.size()) {
    return 0;
  }
  return timestamps[idx];
}

//============================================================================
size_t Hydra::getCurrentIdx() const noexcept {
  auto idx = m_impl->m_exchange_map.getCurrentIdx();
  if (idx == 0) {
    return 0;
  }
  return idx - 1;
}

//============================================================================
size_t *Hydra::getCurrentIdxPtr() const noexcept {
  return m_impl->m_exchange_map.getCurrentIdxPtr();
}

//============================================================================
Vector<Int64> const &Hydra::getTimestamps() const noexcept {
  return m_impl->m_exchange_map.getTimestamps();
}

//============================================================================
HashMap<String, size_t> Hydra::getStrategyIdxMap() const noexcept {
  return m_impl->m_strategy_map;
}

//============================================================================
Result<bool, AtlasException> Hydra::reset() noexcept {
  if (m_state == HydraState::INIT) {
    return Err("Hydra can not be in init state to reset");
  }
  m_impl->exceptions.clear();
  m_impl->m_exchange_map.reset();
  for (auto &allocator : m_impl->m_strategies) {
    allocator->reset();
  }
  m_state = HydraState::BUILT;
  return true;
}

//============================================================================
SharedPtr<Exchange> Hydra::pyAddExchange(String name, String source,
                                         Option<String> datetime_format) {
  auto res = addExchange(std::move(name), std::move(source),
                         std::move(datetime_format));
  if (!res) {
    throw std::exception(res.error().what());
  }
  return *res;
}

//============================================================================
SharedPtr<MetaStrategy> Hydra::pyAddStrategy(SharedPtr<MetaStrategy> Allocator,
                                         bool replace_if_exists) {
  auto res = addStrategy(std::move(Allocator), replace_if_exists);
  if (!res) {
    throw std::exception(res.error().what());
  }
  return m_impl->m_strategies.back();
}

//============================================================================
SharedPtr<Exchange> Hydra::pyGetExchange(String const &name) const {
  auto res = getExchange(name);
  if (!res) {
    throw std::exception(res.error().what());
  }
  return *res;
}

//============================================================================
Vector<AtlasException> const &Hydra::getExceptions() const noexcept {
  return m_impl->exceptions;
}

//============================================================================
void Hydra::pyRun() {
  auto res = run();
  if (!res) {
    throw std::exception(res.error().what());
  }
}

//============================================================================
void Hydra::pyBuild() {
  auto res = build();
  if (!res) {
    throw std::exception(res.error().what());
  }
}

//============================================================================
void Hydra::pyReset() {
  auto res = reset();
  if (!res) {
    throw std::exception(res.error().what());
  }
}

} // namespace Atlas