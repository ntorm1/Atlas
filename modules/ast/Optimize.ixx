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
export struct GridDimension
{
	String dimension_name;
	size_t dimension_size;
	Vector<double> dimension_values;
	SharedPtr<StrategyBufferOpNode> buffer_node;
	std::function<void(StrategyBufferOpNode*, double)> buffer_node_getter;
	std::function<void(StrategyBufferOpNode*, double)> buffer_node_setter;
	size_t current_index = 0;

	void set(size_t index) noexcept
	{
		current_index = index;
		double value = dimension_values[index];
		dimension_values[index] = value;
		buffer_node_setter(buffer_node.get(), value);
	}

	size_t size() const noexcept
	{
		return dimension_size;
	}
};


//============================================================================
enum class GridDimensionType
{
	WEIGHT,
	PNL
};


//============================================================================
class StrategyGrid
{
	friend class Strategy;
private:
	Strategy* m_strategy;
	Exchange const& m_exchange;
	std::pair<GridDimension, GridDimension> m_dimensions;
	LinAlg::EigenMatrix<SharedPtr<Tracer>> m_tracers;
	double* m_weights_grid = nullptr;
	double* m_pnl_grid = nullptr;
	size_t m_asset_count = 0;

	LinAlg::EigenMap<LinAlg::EigenVectorXd> getBuffer(size_t row, size_t col, GridDimensionType t) noexcept;
	size_t gridStart(size_t row, size_t col) const noexcept;
	void evaluateGrid() noexcept;
	void evaluateChild() noexcept;

public:
	StrategyGrid(
		Strategy* strategy,
		Exchange const& exchange,
		std::pair<GridDimension, GridDimension> m_dimensions
	) noexcept;

	~StrategyGrid() noexcept;
};


}


}
