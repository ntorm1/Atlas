module;
module OptimizeNodeModule;

import ExchangeModule;
import TracerModule;
import StrategyModule;

namespace Atlas
{


namespace AST
{

//============================================================================
StrategyGrid::StrategyGrid(
	Strategy* strategy,
	Exchange const& exchange,
	std::pair<GridDimension, GridDimension> dimensions
) noexcept
:
	m_strategy(strategy),
	m_exchange(exchange),
	m_dimensions(dimensions),
	m_asset_count(exchange.getAssetCount())
{
	double initial_cash = strategy->getTracer().getInitialCash();


	size_t row_count = m_dimensions.first.size();
	size_t col_count = m_dimensions.second.size();
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
	m_pnl_grid = new double[row_count * col_count * depth];
}


//============================================================================
size_t
StrategyGrid::gridStart(size_t row, size_t col) const noexcept
{
	size_t col_count = m_dimensions.second.size();
	size_t depth = m_asset_count;
	return (row * col_count * depth) + (col * depth);
}


//============================================================================
void
StrategyGrid::evaluateGrid() noexcept
{
	auto tracer = m_strategy->getTracer();

	size_t row_count = m_dimensions.first.size();
	size_t col_count = m_dimensions.second.size();

	for (size_t i = 0; i < row_count; ++i)
	{
		m_dimensions.first.set(i);
		for (size_t j = 0; j < col_count; ++j)
		{
			m_dimensions.second.set(j);
		}
	}
}


//============================================================================
LinAlg::EigenMap<LinAlg::EigenVectorXd>
StrategyGrid::getBuffer(size_t row, size_t col, GridDimensionType t) noexcept
{
	double* buffer = nullptr;
	switch (t)
	{
	case GridDimensionType::WEIGHT:
		buffer = m_weights_grid;
		break;
	case GridDimensionType::PNL:
		buffer = m_pnl_grid;
		break;
	default:
		std::unreachable();
	}

	return LinAlg::EigenMap<LinAlg::EigenVectorXd>(
		buffer,
		m_asset_count
	);
}


//============================================================================
void
StrategyGrid::evaluateChild() noexcept
{
	// AST has been swapped in place, now evaluate strategy using new buffers
	size_t row = m_dimensions.first.current_index;
	size_t col = m_dimensions.second.current_index;
	auto weights_buffer = getBuffer(
		row,
		col,
		GridDimensionType::WEIGHT
	);
	auto pnl_buffer = getBuffer(
		row,
		col,
		GridDimensionType::PNL
	);
	auto tracer = m_tracers(row, col);
	m_strategy->setTracer(tracer);

	// evaluate the strategy with the current market prices and weights using 
	// the grid buffers
	m_strategy->step(weights_buffer);
}


//============================================================================
StrategyGrid::~StrategyGrid() noexcept
{
	if (m_weights_grid)
	{
		delete[] m_weights_grid;
	}
	if (m_pnl_grid)
	{
		delete[] m_pnl_grid;
	}
}


}


}