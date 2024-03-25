#pragma once
#ifdef ATLAS_EXPORTS
#define ATLAS_API __declspec(dllexport)
#else
#define ATLAS_API  __declspec(dllimport)
#endif

#include "standard/AtlasCore.hpp"
#include "standard/AtlasEnums.hpp"
#include "standard/AtlasLinAlg.hpp"
#include "ast/BaseNode.hpp"
#include "ast/StrategyBufferNode.hpp"

namespace Atlas
{

namespace AST
{

//============================================================================
enum class DimensionType : Uint8
{
	OBSERVER = 0,
	LIMIT = 1
};


//============================================================================
class GridDimension
{
private:

protected:
	String m_name;
	size_t m_dimension_size;
	Vector<double> m_dimension_values;
	DimensionType m_type;

public:
	GridDimension(
		const String& name,
		const Vector<double>& dimension_values,
		DimensionType type
	) noexcept;

	ATLAS_API virtual ~GridDimension() noexcept;

	virtual void reset() noexcept = 0;
	virtual void set(size_t index) noexcept = 0;

	String const& getName() const noexcept
	{
		return m_name;
	}

	Vector<double> const& getValues() const noexcept
	{
		return m_dimension_values;
	}
	
	DimensionType getType() const noexcept
	{
		return m_type;
	}

	double get(size_t index) const noexcept
	{
		return m_dimension_values[index];
	}

	size_t size() const noexcept
	{
		return m_dimension_size;
	}
};


//============================================================================
class GridDimensionObserver : public GridDimension
{
	friend class StrategyGrid;
private:
	using swapFuncType = void(*)(SharedPtr<ASTNode>, SharedPtr<StrategyBufferOpNode>&) noexcept;

	uintptr_t swap_addr;
	void(*swap_func)(SharedPtr<ASTNode>, SharedPtr<StrategyBufferOpNode>&) noexcept;
	SharedPtr<AssetObserverNode> m_observer_base;
	SharedPtr<StrategyBufferOpNode> m_observer_child;
	LinAlg::EigenVector<SharedPtr<StrategyBufferOpNode>> m_observers;
	LinAlg::EigenVector<size_t> m_warmup;

public:
	GridDimensionObserver(
		const String& name,
		const Vector<double>& dimension_values,
		SharedPtr<AssetObserverNode> observer_base,
		SharedPtr<StrategyBufferOpNode> observer_child,
		uintptr_t swap_addr
	) noexcept;
	ATLAS_API ~GridDimensionObserver() noexcept;


	ATLAS_API static SharedPtr<GridDimensionObserver> make(
		const String& name,
		const Vector<double>& dimension_values,
		SharedPtr<AssetObserverNode> observer_base,
		SharedPtr<StrategyBufferOpNode> observer_child,
		uintptr_t swap_addr
	) noexcept;

	void buildWarmup(Strategy* m_strategy) noexcept;

	void addObserver(SharedPtr<StrategyBufferOpNode> observer, size_t i) noexcept
	{
		m_observers(i) = observer;
	}

	SharedPtr<AssetObserverNode> const& getObserverBase() const noexcept
	{
		return m_observer_base;
	}

	void set(size_t index) noexcept override
	{
		swap_func(m_observer_child, m_observers(index));
	}

	void reset() noexcept override;
};


//============================================================================
class GridDimensionLimit : public GridDimension
{
private:
	using GetterFuncType = double(*)(SharedPtr<TradeLimitNode>) noexcept;
	using SetterFuncType = void(*)(SharedPtr<TradeLimitNode>, double) noexcept;

	SharedPtr<TradeLimitNode> buffer_node;
	double(*buffer_node_getter)(SharedPtr<TradeLimitNode>);
	void(*buffer_node_setter)(SharedPtr<TradeLimitNode>, double);
	uintptr_t getter_addr;
	uintptr_t setter_addr;
	size_t current_index = 0;
	double cached_value = 0;
	double original_value = 0;

public:
	GridDimensionLimit(
		const String& name,
		const Vector<double>& dimension_values,
		const SharedPtr<TradeLimitNode>& node,
		uintptr_t getter,
		uintptr_t setter
	) noexcept;

	ATLAS_API static SharedPtr<GridDimensionLimit> make(
		const String& name,
		const Vector<double>& dimension_values,
		const SharedPtr<TradeLimitNode>& node,
		uintptr_t getter,
		uintptr_t setter
	) noexcept;

	void reset() noexcept override
	{
		setNodeValue(original_value);
	}

	void set(size_t index) noexcept override
	{
		current_index = index;
		double value = m_dimension_values[index];
		setNodeValue(value);
	}

	void setNodeValue(double value) noexcept
	{
		buffer_node_setter(buffer_node, value);
	}
};


//============================================================================
class StrategyGrid
{
	friend class Strategy;
private:
	Strategy* m_strategy;
	Exchange const& m_exchange;
	std::pair<SharedPtr<GridDimension>, SharedPtr<GridDimension>> m_dimensions;
	LinAlg::EigenMatrix<SharedPtr<Tracer>> m_tracers;
	double* m_weights_grid = nullptr;
	size_t m_asset_count = 0;
	GridType m_grid_type = GridType::FULL;
	GridDimensionObserver* __observer_dim1 = nullptr;
	GridDimensionObserver* __observer_dim2 = nullptr;


	LinAlg::EigenMap<LinAlg::EigenVectorXd> getBuffer(size_t row, size_t col) noexcept;
	size_t gridStart(size_t row, size_t col) const noexcept;
	void reset() noexcept;
	void buildNodeGrid() noexcept;
	void builNodeDim(GridDimensionObserver* dim) noexcept;
	void evaluate() noexcept;
	void evaluateGrid() noexcept;
	void evaluateChild(size_t row, size_t col) noexcept;

public:
	StrategyGrid(
		Strategy* strategy,
		Exchange const& exchange,
		std::pair<SharedPtr<GridDimension>, SharedPtr<GridDimension>> m_dimensions,
		Option<GridType> grid_type = std::nullopt
	) noexcept;

	auto const& getTracers() const noexcept
	{
		return m_tracers;
	}

	auto const& getDimensions() const noexcept
	{
		return m_dimensions;
	}

	ATLAS_API [[nodiscard ]] bool enableTracerHistory(TracerType t) noexcept;
	ATLAS_API double meanReturn() noexcept;
	ATLAS_API Option<SharedPtr<Tracer>> getTracer(size_t row, size_t col) const noexcept;
	ATLAS_API ~StrategyGrid() noexcept;
	ATLAS_API size_t rows() const noexcept {return m_dimensions.first->size();}
	ATLAS_API size_t cols() const noexcept {return m_dimensions.second->size();}
};



}


}
