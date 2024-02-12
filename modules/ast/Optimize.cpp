module;
#include <Eigen/dense>
module OptimizeNodeModule;

import ExchangeModule;
import TracerModule;
import StrategyModule;

namespace Atlas
{


namespace AST
{

//============================================================================
GridDimension::GridDimension(
	const String& name,
	const Vector<double>& dimension_values,
	const SharedPtr<TradeLimitNode>& node,
	uintptr_t getter,
	uintptr_t setter
) noexcept :
	dimension_name(name),
	dimension_size(dimension_values.size()),
	dimension_values(dimension_values),
	buffer_node(node),
	getter_addr(getter),
	setter_addr(setter)
{

	buffer_node_setter = reinterpret_cast<SetterFuncType>(setter);
	buffer_node_getter = reinterpret_cast<GetterFuncType>(getter);
}


//============================================================================
SharedPtr<GridDimension>
GridDimension::make(const String& name, const Vector<double>& dimension_values, const SharedPtr<TradeLimitNode>& node, uintptr_t getter, uintptr_t setter) noexcept
{
	return std::make_shared<GridDimension>(name,
		dimension_values, 
		node,
		getter,
		setter
	);
}


//============================================================================
StrategyGrid::StrategyGrid(
	Strategy* strategy,
	Exchange const& exchange,
	std::pair<SharedPtr<GridDimension>, SharedPtr<GridDimension>> dimensions
) noexcept
:
	m_strategy(strategy),
	m_exchange(exchange),
	m_dimensions(dimensions),
	m_asset_count(exchange.getAssetCount())
{
	double initial_cash = strategy->getTracer().getInitialCash();


	size_t row_count = m_dimensions.first->size();
	size_t col_count = m_dimensions.second->size();
	size_t depth = m_asset_count;

	m_tracers.resize(row_count, col_count);
	for (size_t i = 0; i < row_count; ++i)
	{
		for (size_t j = 0; j < col_count; ++j)
		{
			m_tracers(i, j) = std::make_shared<Tracer>(
				*m_strategy,
				exchange,
				initial_cash
			);
		}
	}

	m_weights_grid = new double[row_count * col_count * depth];
}


//============================================================================
size_t
StrategyGrid::gridStart(size_t row, size_t col) const noexcept
{
	size_t col_count = m_dimensions.second->size();
	size_t depth = m_asset_count;
	return (row * col_count * depth) + (col * depth);
}


//============================================================================
void
StrategyGrid::reset() noexcept
{
	size_t row_count = m_dimensions.first->size();
	size_t col_count = m_dimensions.second->size();
	size_t depth = m_asset_count;

	for (size_t i = 0; i < row_count; ++i)
	{
		for (size_t j = 0; j < col_count; ++j)
		{
			auto tracer = m_tracers(i, j);
			tracer->reset();
		}
	}

	// reset the weights grid
	size_t buffer_size = row_count * col_count * depth;
	for (size_t i = 0; i < buffer_size; ++i)
	{
		m_weights_grid[i] = 0.0;
	}
}


//============================================================================
LinAlg::EigenMap<LinAlg::EigenVectorXd>
StrategyGrid::getBuffer(size_t row, size_t col) noexcept
{
	return LinAlg::EigenMap<LinAlg::EigenVectorXd>(
		m_weights_grid + gridStart(row, col),
		m_asset_count
	);
}


//============================================================================
void
StrategyGrid::evaluateGrid() noexcept
{
	// copy shared pointers to tracers to swap back in after evaluation of grid
	auto tracer = m_strategy->getTracerPtr();

	// loop over grid and use node getters and setters to evaluate the strategy
	// over the parameter space
	size_t row_count = m_dimensions.first->size();
	size_t col_count = m_dimensions.second->size();

	// store the original value of the dimensions
	double original_row_value = m_dimensions.first->getNodeValue();
	double original_col_value = m_dimensions.second->getNodeValue();

	for (size_t i = 0; i < row_count; ++i)
	{
		double row_value = m_dimensions.first->getNodeValue();
		m_dimensions.first->set(i);
		for (size_t j = 0; j < col_count; ++j)
		{
			double col_value = m_dimensions.second->getNodeValue();
			m_dimensions.second->set(j);
			evaluateChild(i, j);
			m_dimensions.second->setNodeValue(col_value);
		}
		m_dimensions.first->setNodeValue(row_value);
	}

	// restore original value of the dimensions for the base strategy
	m_dimensions.first->setNodeValue(original_row_value);
	m_dimensions.second->setNodeValue(original_col_value);
	m_strategy->setTracer(tracer);
}


//============================================================================
void
StrategyGrid::evaluateChild(size_t row, size_t col) noexcept
{
	// AST has been swapped in place, now evaluate strategy using grid's buffers
	auto weights_buffer = getBuffer(
		row,
		col
	);
	// evaluate the strategy with the current market prices and weights using 
	// the grid buffers
	auto tracer = m_tracers(row, col);
	m_strategy->setTracer(tracer);
	m_strategy->step(weights_buffer);
}


//============================================================================
StrategyGrid::~StrategyGrid() noexcept
{
	if (m_weights_grid)
	{
		delete[] m_weights_grid;
	}
}


}


}