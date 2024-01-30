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

export enum class AssetNodeType {
	AssetReadNode,
	AssetProductNode,
	AssetQuotientNode,
	AssetSumNode,
	AssetDifferenceNode
};


//============================================================================
// Define AssetOpNodeVariant using std::variant
export class AssetOpNodeVariant {
public:
	mutable std::variant<
		SharedPtr<AssetReadNode>,
		SharedPtr<AssetProductNode>,
		SharedPtr<AssetQuotientNode>,
		SharedPtr<AssetSumNode>,
		SharedPtr<AssetDifferenceNode>
	> value;
	size_t warmup;
	AssetNodeType  t;

public:
	using node_variant = std::variant <
		SharedPtr<AssetReadNode>,
		SharedPtr<AssetProductNode>,
		SharedPtr<AssetQuotientNode>,
		SharedPtr<AssetSumNode>,
		SharedPtr<AssetDifferenceNode>>;

	AssetOpNodeVariant(
		node_variant node,
		size_t warmup,
		AssetNodeType t
	) noexcept:
		value(std::move(node)), warmup(warmup), t(t) {}

	ATLAS_API AssetOpNodeVariant(const AssetOpNodeVariant& other);

	AssetOpNodeVariant() = delete;
	AssetOpNodeVariant(AssetOpNodeVariant&&) = default;
	AssetOpNodeVariant& operator=(const AssetOpNodeVariant&) = default;
	AssetOpNodeVariant& operator=(AssetOpNodeVariant&&) = default;
	ATLAS_API AssetOpNodeVariant(node_variant node) noexcept;
	ATLAS_API ~AssetOpNodeVariant() noexcept;
};


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

	ExchangeViewFilter() = delete;
	ATLAS_API ~ExchangeViewFilter() noexcept = default;
	ATLAS_API ExchangeViewFilter(
		ExchangeViewFilterType type,
		double value
	)  noexcept :
		type(type), value(value) {}
};


//============================================================================
export class ExchangeViewNode
	final
	: public StrategyBufferOpNode
{
private:
	Exchange& m_exchange;
	AssetOpNodeVariant m_asset_op_node;
	size_t m_view_size;
	size_t m_warmup;
	Option<ExchangeViewFilter> m_filter = std::nullopt;
public:
	ATLAS_API ~ExchangeViewNode() noexcept;

	//============================================================================
	ATLAS_API ExchangeViewNode(
		Exchange& exchange,
		AssetOpNodeVariant asset_op_node
	) noexcept;


	//============================================================================
	ATLAS_API ExchangeViewNode(
		SharedPtr<Exchange> exchange,
		AssetOpNodeVariant asset_op_node
	) noexcept;


	//============================================================================
	ATLAS_API [[nodiscard]] static SharedPtr<ExchangeViewNode> make(
		SharedPtr<Exchange> exchange,
		AssetOpNodeVariant asset_op_node,
		Option<ExchangeViewFilter> filter = std::nullopt
	) noexcept
	{
		auto node = std::make_shared<ExchangeViewNode>(
			exchange, std::move(asset_op_node)
		);
		node->setFilter(filter);
		return node;
	}


	//============================================================================
	ATLAS_API void setFilter(ExchangeViewFilterType type, double value) noexcept
	{
		m_filter = ExchangeViewFilter(type, value);
	}

	//============================================================================
	ATLAS_API void setFilter(Option<ExchangeViewFilter> filter) noexcept
	{
		m_filter = filter;
	}



	[[nodiscard]] AssetNodeType getType() const noexcept { return m_asset_op_node.t; }
	[[nodiscard]] size_t getWarmup() const noexcept override { return m_warmup; }
	[[nodiscard]] size_t getViewSize() const noexcept { return m_view_size; }
	[[nodiscard]] auto& getExchange() { return m_exchange; }
	
	void filter(LinAlg::EigenVectorXd& v) const noexcept;
	ATLAS_API  void evaluate(LinAlg::EigenVectorXd&) noexcept override;
};


};

}