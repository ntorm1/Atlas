module;
#pragma once
#define NOMINMAX
#ifdef ATLAS_EXPORTS
#define ATLAS_API __declspec(dllexport)
#else
#define ATLAS_API  __declspec(dllimport)
#endif
export module ExchangeNodeModule;

import <variant>;

import AtlasCore;
import AtlasLinAlg;
import BaseNodeModule;
import AssetNodeModule;
import StrategyBufferModule;

namespace Atlas
{

namespace AST
{


//============================================================================
export enum class ExchangeViewFilterType
{
	GREATER_THAN = 0,
	LESS_THAN = 1,
	EQUAL_TO = 2
};


//============================================================================
export struct ExchangeViewFilter
{
	ExchangeViewFilterType type;
	double value;
	Option<double> value_inplace = std::nullopt;

	ExchangeViewFilter() = delete;
	ATLAS_API ~ExchangeViewFilter() noexcept = default;
	ATLAS_API ExchangeViewFilter(
		ExchangeViewFilterType type,
		double value,
		Option<double> value_inplace = std::nullopt
	)  noexcept :
		type(type), value(value), value_inplace(value_inplace) {}

	void setValue(double v) noexcept { value = v; }
};


//============================================================================
export class ExchangeViewNode
	final
	: public StrategyBufferOpNode
{
private:
	Exchange& m_exchange;
	SharedPtr<StrategyBufferOpNode> m_asset_op_node;
	size_t m_view_size;
	size_t m_warmup;
	Vector<SharedPtr<ExchangeViewFilter>> m_filters;
public:
	ATLAS_API ~ExchangeViewNode() noexcept;

	//============================================================================
	ATLAS_API ExchangeViewNode(
		Exchange& exchange,
		SharedPtr<StrategyBufferOpNode> asset_op_node
	) noexcept;


	//============================================================================
	ATLAS_API ExchangeViewNode(
		SharedPtr<Exchange> exchange,
		SharedPtr<StrategyBufferOpNode> asset_op_node
	) noexcept;


	//============================================================================
	ATLAS_API [[nodiscard]] static SharedPtr<ExchangeViewNode> make(
		SharedPtr<Exchange> exchange,
		SharedPtr<StrategyBufferOpNode> asset_op_node,
		Option<SharedPtr<ExchangeViewFilter>> filter = std::nullopt
	) noexcept
	{
		auto node = std::make_shared<ExchangeViewNode>(
			exchange, std::move(asset_op_node)
		);
		if (filter.has_value())
		{
			node->setFilter(filter.value());
		}
		return node;
	}


	//============================================================================
	ATLAS_API SharedPtr<ExchangeViewFilter> setFilter(
		ExchangeViewFilterType type, 
		double value,
		Option<double> value_inplace = std::nullopt
	) noexcept
	{
		auto f = std::make_shared<ExchangeViewFilter>(type, value, value_inplace);
		return setFilter(f);
	}

	//============================================================================
	ATLAS_API SharedPtr<ExchangeViewFilter> setFilter(SharedPtr<ExchangeViewFilter> filter) noexcept
	{
		m_filters.push_back(filter);
		return filter;
	}

	[[nodiscard]] size_t getWarmup() const noexcept override { return m_warmup; }
	[[nodiscard]] size_t getViewSize() const noexcept { return m_view_size; }
	[[nodiscard]] auto& getExchange() { return m_exchange; }
	void filter(LinAlg::EigenVectorXd& v) const noexcept;
	ATLAS_API  void evaluate(LinAlg::EigenVectorXd&) noexcept override;
};


};

}