module;
#include <Eigen/dense>
#include <omp.h>
module OptimizeNodeModule;

import ExchangeModule;
import TracerModule;
import StrategyModule;
import ObserverNodeBaseModule;

namespace Atlas {

namespace AST {

//============================================================================
GridDimension::GridDimension(const String &name,
                             const Vector<double> &dimension_values,
                             DimensionType type) noexcept
    : m_name(name), m_dimension_values(std::move(dimension_values)),
      m_type(type) {
  m_dimension_size = m_dimension_values.size();
}

//============================================================================
GridDimension::~GridDimension() noexcept {}

//============================================================================
GridDimensionLimit::GridDimensionLimit(const String &name,
                                       const Vector<double> &dimension_values,
                                       const SharedPtr<TradeLimitNode> &node,
                                       uintptr_t getter,
                                       uintptr_t setter) noexcept
    : GridDimension(name, dimension_values, DimensionType::LIMIT),
      buffer_node(node), getter_addr(getter), setter_addr(setter) {

  buffer_node_setter = reinterpret_cast<SetterFuncType>(setter);
  buffer_node_getter = reinterpret_cast<GetterFuncType>(getter);
  original_value = buffer_node_getter(buffer_node);
}

//============================================================================
SharedPtr<GridDimensionLimit>
GridDimensionLimit::make(const String &name,
                         const Vector<double> &dimension_values,
                         const SharedPtr<TradeLimitNode> &node,
                         uintptr_t getter, uintptr_t setter) noexcept {
  return std::make_shared<GridDimensionLimit>(name, dimension_values, node,
                                              getter, setter);
}

//============================================================================
StrategyGrid::StrategyGrid(
    Strategy *strategy, Exchange const &exchange,
    std::pair<SharedPtr<GridDimension>, SharedPtr<GridDimension>> dimensions,
    Option<GridType> grid_type) noexcept
    : m_strategy(strategy), m_exchange(exchange), m_dimensions(dimensions),
      m_asset_count(exchange.getAssetCount()) {
  double initial_cash = strategy->getTracer().getInitialCash();

  size_t row_count = m_dimensions.first->size();
  size_t col_count = m_dimensions.second->size();
  size_t depth = m_asset_count;

  m_tracers.resize(row_count, col_count);
  for (size_t i = 0; i < row_count; ++i) {
    for (size_t j = 0; j < col_count; ++j) {
      m_tracers(i, j) =
          std::make_shared<Tracer>(m_strategy, exchange, initial_cash);
    }
  }

  if (grid_type) {
    m_grid_type = *grid_type;
  }

  __observer_dim1 =
      m_dimensions.first->getType() == DimensionType::OBSERVER
          ? static_cast<GridDimensionObserver *>(m_dimensions.first.get())
          : nullptr;
  __observer_dim2 =
      m_dimensions.second->getType() == DimensionType::OBSERVER
          ? static_cast<GridDimensionObserver *>(m_dimensions.second.get())
          : nullptr;

  m_weights_grid = new double[row_count * col_count * depth];
  memset(m_weights_grid, 0, row_count * col_count * depth * sizeof(double));
  buildNodeGrid();
}

//============================================================================
void StrategyGrid::reset() noexcept {
  size_t row_count = m_dimensions.first->size();
  size_t col_count = m_dimensions.second->size();
  size_t depth = m_asset_count;

  for (size_t i = 0; i < row_count; ++i) {
    for (size_t j = 0; j < col_count; ++j) {
      m_tracers(i, j)->reset();
    }
  }

  // reset the weights grid
  size_t buffer_size = row_count * col_count * depth;
  for (size_t i = 0; i < buffer_size; ++i) {
    m_weights_grid[i] = 0.0;
  }
}

//============================================================================
void StrategyGrid::builNodeDim(GridDimensionObserver *observer_dim) noexcept {
  size_t size = observer_dim->size();
  SharedPtr<AssetObserverNode> const &observer_node =
      observer_dim->getObserverBase();
  auto id = observer_node->getId();
  for (size_t i = 0; i < size; ++i) {
    // auto node_id = std::format("{}_{}", id, i);
    // switch (observer_node->observerType())
    //{
    // case AssetObserverType::SUM:
    //{
    //	SharedPtr<SumObserverNode> new_observer =
    //std::make_shared<SumObserverNode>( 		node_id, 		observer_node->parent(),
    //		static_cast<size_t>(observer_dim->get(i))
    //	);
    //	observer_dim->addObserver(std::move(new_observer), i);
    //	break;
    // }
    // case AssetObserverType::MEAN:
    //{
    //	SharedPtr<MeanObserverNode> new_observer =
    //std::make_shared<MeanObserverNode>( 		node_id, 		observer_node->parent(),
    //		static_cast<size_t>(observer_dim->get(i))
    //	);
    //	observer_dim->addObserver(std::move(new_observer), i);
    //	break;
    // }
    // default:
    //	assert(false);
    // }
  }
}

//============================================================================
void StrategyGrid::buildNodeGrid() noexcept {
  int dim_count = 0;
  if (m_dimensions.first->getType() == DimensionType::OBSERVER)
    ++dim_count;
  if (m_dimensions.second->getType() == DimensionType::OBSERVER)
    ++dim_count;
  if (!dim_count) {
    return;
  }
  if (dim_count == 1) {
    auto observer_dim =
        m_dimensions.first->getType() == DimensionType::OBSERVER
            ? static_cast<GridDimensionObserver *>(m_dimensions.first.get())
            : static_cast<GridDimensionObserver *>(m_dimensions.second.get());
    observer_dim->buildWarmup(m_strategy);
    builNodeDim(observer_dim);
  } else {
    auto dim1 = static_cast<GridDimensionObserver *>(m_dimensions.first.get());
    auto dim2 = static_cast<GridDimensionObserver *>(m_dimensions.second.get());
    builNodeDim(static_cast<GridDimensionObserver *>(dim1));
    builNodeDim(static_cast<GridDimensionObserver *>(dim2));
    dim1->buildWarmup(m_strategy);
    dim2->buildWarmup(m_strategy);
  }
}

//============================================================================
void StrategyGrid::evaluate() noexcept {
  LinAlg::EigenConstColView market_returns = m_exchange.getMarketReturns();
  LinAlg::EigenMap<LinAlg::EigenMatrixXd> weights_grid(
      m_weights_grid, m_asset_count,
      m_dimensions.first->size() * m_dimensions.second->size());
  assert(market_returns.rows() == weights_grid.rows());
  LinAlg::EigenVectorXd portfolio_returns =
      market_returns.transpose() * weights_grid;
  for (size_t i = 0; i < m_dimensions.first->size(); ++i) {
    for (size_t j = 0; j < m_dimensions.second->size(); ++j) {
      auto tracer = m_tracers(i, j);
      double nlv = tracer->getNLV();
      tracer->setNLV(
          nlv * (1.0 + portfolio_returns(i * m_dimensions.second->size() + j)));
      tracer->evaluate();
    }
  }
}

//============================================================================
size_t StrategyGrid::gridStart(size_t row, size_t col) const noexcept {
  size_t col_count = m_dimensions.second->size();
  size_t depth = m_asset_count;
  return (row * col_count * depth) + (col * depth);
}

//============================================================================
LinAlg::EigenMap<LinAlg::EigenVectorXd>
StrategyGrid::getBuffer(size_t row, size_t col) noexcept {
  return LinAlg::EigenMap<LinAlg::EigenVectorXd>(
      m_weights_grid + gridStart(row, col), m_asset_count);
}

//============================================================================
void StrategyGrid::evaluateGrid() noexcept {
  // copy shared pointers to tracers to swap back in after evaluation of grid
  auto tracer = m_strategy->getTracerPtr();

  // loop over grid and use node getters and setters to evaluate the strategy
  // over the parameter space
  size_t row_count = m_dimensions.first->size();
  size_t col_count = m_dimensions.second->size();

  // evaluate the grid strategy with the current market prices and weights
  evaluate();

  for (size_t i = 0; i < row_count; ++i) {
    if (__observer_dim1 &&
        m_exchange.currentIdx() < __observer_dim1->m_warmup(i)) {
      continue;
    }

    m_dimensions.first->set(i);
    for (size_t j = 0; j < col_count; ++j) {
      if (__observer_dim2 &&
          m_exchange.currentIdx() < __observer_dim2->m_warmup(j)) {
        continue;
      }

      switch (m_grid_type) {
      case GridType::UPPER_TRIANGULAR: {
        if (j < i) {
          continue;
        }
        break;
      }
      case GridType::LOWER_TRIANGULAR: {
        if (j > i) {
          continue;
        }
        break;
      }
      default:
        break;
      }

      m_dimensions.second->set(j);
      evaluateChild(i, j);

      // swap the observer node back into the grid
      if (__observer_dim2) {
        m_dimensions.second->set(j);
      }
    }
    if (__observer_dim1) {
      m_dimensions.first->set(i);
    }
  }
  // restore original value of the dimensions for the base strategy
  m_dimensions.first->reset();
  m_dimensions.second->reset();
  m_strategy->setTracer(tracer);
}

//============================================================================
void StrategyGrid::evaluateChild(size_t row, size_t col) noexcept {
  // AST has been swapped in place, now evaluate strategy using grid's buffers
  auto weights_buffer = getBuffer(row, col);
  // evaluate the strategy with the current market prices and weights using
  // the grid buffers
  auto tracer = m_tracers(row, col);
  m_strategy->setTracer(tracer);
  m_strategy->step(weights_buffer);
}

//============================================================================
bool StrategyGrid::enableTracerHistory(TracerType t) noexcept {
  for (size_t i = 0; i < m_dimensions.first->size(); ++i) {
    for (size_t j = 0; j < m_dimensions.second->size(); ++j) {
      if (!m_tracers(i, j)->enableTracerHistory(t)) {
        return false;
      }
    }
  }
  return true;
}

//============================================================================
double StrategyGrid::meanReturn() noexcept {
  double returns = 0.0;
  size_t row_count = m_dimensions.first->size();
  size_t col_count = m_dimensions.second->size();
  for (size_t i = 0; i < row_count; ++i) {
    for (size_t j = 0; j < col_count; ++j) {
      returns +=
          (m_tracers(i, j)->getNLV() - m_tracers(i, j)->getInitialCash()) /
          m_tracers(i, j)->getInitialCash();
    }
  }
  return returns / (row_count * col_count);
}

//============================================================================
Option<SharedPtr<Tracer>> StrategyGrid::getTracer(size_t row,
                                                  size_t col) const noexcept {
  if (row < m_dimensions.first->size() && col < m_dimensions.second->size()) {
    return m_tracers(row, col);
  }
  return std::nullopt;
}

//============================================================================
StrategyGrid::~StrategyGrid() noexcept {
  if (m_weights_grid) {
    delete[] m_weights_grid;
  }
}

//============================================================================
GridDimensionObserver::GridDimensionObserver(
    const String &name, const Vector<double> &dimension_values,
    SharedPtr<AssetObserverNode> observer_base,
    SharedPtr<StrategyBufferOpNode> observer_child,
    uintptr_t swap_addr) noexcept
    : GridDimension(name, dimension_values, DimensionType::OBSERVER),
      m_observer_base(observer_base), m_observer_child(observer_child) {
  m_observers.resize(dimension_values.size(), 1);
  swap_func = reinterpret_cast<swapFuncType>(swap_addr);
}

//============================================================================
GridDimensionObserver::~GridDimensionObserver() noexcept {}

//============================================================================
SharedPtr<GridDimensionObserver>
GridDimensionObserver::make(const String &name,
                            const Vector<double> &dimension_values,
                            SharedPtr<AssetObserverNode> observer_base,
                            SharedPtr<StrategyBufferOpNode> observer_child,
                            uintptr_t swap_addr) noexcept {
  return std::make_shared<GridDimensionObserver>(
      name, dimension_values, observer_base, observer_child, swap_addr);
}

//============================================================================
void GridDimensionObserver::buildWarmup(Strategy *m_strategy) noexcept {
  m_warmup.resize(m_dimension_size);
  m_warmup.setZero();
  SharedPtr<StrategyBufferOpNode> temp = nullptr;
  swap_func(m_observer_child, temp);
  for (size_t i = 0; i < m_dimension_size; ++i) {
    swap_func(m_observer_child, m_observers(i));
    auto warmup = m_strategy->refreshWarmup();
    m_warmup(i) = warmup;
    swap_func(m_observer_child, m_observers(i));
  }
  swap_func(m_observer_child, temp);
  m_strategy->refreshWarmup();
}

//============================================================================
void GridDimensionObserver::reset() noexcept {
  auto node = std::dynamic_pointer_cast<StrategyBufferOpNode>(m_observer_base);
  swap_func(m_observer_child, node);
}

} // namespace AST

} // namespace Atlas