module;
#pragma once
#ifdef ATLAS_EXPORTS
#define ATLAS_API __declspec(dllexport)
#else
#define ATLAS_API  __declspec(dllimport)
#endif
#include <functional>
export module OptimizeNodeModule;

import AtlasCore;
import AtlasLinAlg;
import BaseNodeModule;
import StrategyBufferModule;

namespace Atlas
{

namespace AST
{

//============================================================================
struct GridDimension
{
	String dimension_name;
	size_t dimension_size;
	Vector<double> dimension_values;
	StrategyBufferOpNode& buffer_node;
	std::function<void(StrategyBufferOpNode*, double)> buffer_node_callback;

	size_t size() const noexcept
	{
		return dimension_size;
	}
};


//============================================================================
class StrategyGrid
{
private:
	using NodeMatrix = LinAlg::EigenMatrix<SharedPtr<StrategyBufferOpNode>>;
	NodeMatrix m_matrix;
	std::pair<GridDimension, GridDimension> m_dimensions;
	double* m_target_buffer_grid = nullptr;
	Exchange const& m_exchange;
	size_t m_asset_count;

	Eigen::Map<LinAlg::EigenVectorXd> getDimensionBuffer(Vector<size_t> index) noexcept;

public:
	StrategyGrid(
		Exchange const& exchange,
		std::pair<GridDimension, GridDimension> m_dimensions
	) noexcept;

	~StrategyGrid() noexcept;
};


}


}
