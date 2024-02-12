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
	using GetterFuncType = double(*)(SharedPtr<TradeLimitNode>) noexcept;
	using SetterFuncType = void(*)(SharedPtr<TradeLimitNode>, double) noexcept;

	String dimension_name;
	size_t dimension_size;
	Vector<double> dimension_values;
	SharedPtr<TradeLimitNode> buffer_node;
	double(*buffer_node_getter)(SharedPtr<TradeLimitNode>);
	void(*buffer_node_setter)(SharedPtr<TradeLimitNode>, double);
	uintptr_t getter_addr;
	uintptr_t setter_addr;
	size_t current_index = 0;

	GridDimension(
		const String& name,
		const Vector<double>& dimension_values,
		const SharedPtr<TradeLimitNode>& node,
		uintptr_t getter,
		uintptr_t setter
	) noexcept;

	ATLAS_API static SharedPtr<GridDimension> make(
		const String& name,
		const Vector<double>& dimension_values,
		const SharedPtr<TradeLimitNode>& node,
		uintptr_t getter,
		uintptr_t setter
	) noexcept;

	void set(size_t index) noexcept
	{
		current_index = index;
		double value = dimension_values[index];
		setNodeValue(value);
	}

	double getNodeValue() const noexcept
	{
		return buffer_node_getter(buffer_node);
	}

	void setNodeValue(double value) noexcept
	{
		buffer_node_setter(buffer_node, value);
	}

	size_t size() const noexcept
	{
		return dimension_size;
	}
};


//============================================================================
export class StrategyGrid
{
	friend class Strategy;
private:
	Strategy* m_strategy;
	Exchange const& m_exchange;
	std::pair<SharedPtr<GridDimension>, SharedPtr<GridDimension>> m_dimensions;
	LinAlg::EigenMatrix<SharedPtr<Tracer>> m_tracers;
	double* m_weights_grid = nullptr;
	size_t m_asset_count = 0;

	LinAlg::EigenMap<LinAlg::EigenVectorXd> getBuffer(size_t row, size_t col) noexcept;
	size_t gridStart(size_t row, size_t col) const noexcept;
	void reset() noexcept;
	void evaluateGrid() noexcept;
	void evaluateChild(size_t row, size_t col) noexcept;

public:
	StrategyGrid(
		Strategy* strategy,
		Exchange const& exchange,
		std::pair<SharedPtr<GridDimension>, SharedPtr<GridDimension>> m_dimensions
	) noexcept;

	auto const& getTracers() const noexcept
	{
		return m_tracers;
	}

	auto const& getDimensions() const noexcept
	{
		return m_dimensions;
	}

	ATLAS_API ~StrategyGrid() noexcept;
};


}


}
