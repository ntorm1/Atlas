module;
module OptimizeNodeModule;

import ExchangeModule;

namespace Atlas
{


namespace AST
{

//============================================================================
StrategyGrid::StrategyGrid(
	Exchange const& exchange,
	std::pair<GridDimension, GridDimension> dimensions
) noexcept
:
m_exchange(exchange),
	m_dimensions(dimensions),
	m_asset_count(exchange.getAssetCount())
{
	// get the total required buffer size for the grid as the product of the 
	// dimension sizes multiplied by the asset count of the exchange
	size_t total_buffer_size = 1;
	total_buffer_size *= m_dimensions.first.size();
	total_buffer_size *= m_dimensions.second.size();
	total_buffer_size *= m_asset_count;
	m_target_buffer_grid = new double[total_buffer_size];
}


//============================================================================
StrategyGrid::~StrategyGrid() noexcept
{
	if (m_target_buffer_grid)
	{
		delete[] m_target_buffer_grid;
	}
}


//============================================================================
Eigen::Map<LinAlg::EigenVectorXd>
StrategyGrid::getDimensionBuffer(Vector<size_t> index) noexcept
{
	size_t offset = index[0] * m_dimensions.second.size() * m_asset_count;
	offset += index[1] * m_asset_count;
	return Eigen::Map<LinAlg::EigenVectorXd>(
		m_target_buffer_grid + offset,
		m_asset_count
	);
}


}


}